#include <memory>
#include <array>

#include "source_chip.h"

namespace tengu {
namespace {
const int32_t c_numcombs = 8;
const int32_t c_numallpasses = 4;
const int32_t c_spread = 23;

const float c_muted = 0.f;
const float c_fixedgain = 0.015f;
const float c_scalewet = 3.f;
const float c_scaledry = 2.f;
const float c_scaledamp = 0.4f;
const float c_scaleroom = 0.28f;
const float c_offsetroom = 0.7f;

const float c_initialroom = 0.5f;
const float c_initialdamp = 0.5f;
const float c_initialwet = 1.f/c_scalewet;
const float c_initialdry = 0.f;
const float c_initialwidth = 1.f;

const int ctl1 = 1116;
const int ctl2 = 1188;
const int ctl3 = 1277;
const int ctl4 = 1356;
const int ctl5 = 1422;
const int ctl6 = 1491;
const int ctl7 = 1557;
const int ctl8 = 1617;

const int ctr1 = ctl1+c_spread;
const int ctr2 = ctl2+c_spread;
const int ctr3 = ctl3+c_spread;
const int ctr4 = ctl4+c_spread;
const int ctr5 = ctl5+c_spread;
const int ctr6 = ctl6+c_spread;
const int ctr7 = ctl7+c_spread;
const int ctr8 = ctl8+c_spread;

const int atl1 = 556;
const int atl2 = 441;
const int atl3 = 341;
const int atl4 = 225;

const int atr1 = atl1+c_spread;
const int atr2 = atl2+c_spread;
const int atr3 = atl3+c_spread;
const int atr4 = atl4+c_spread;

const float c_renorm = 1.0E-26f;

struct comb_t
{
    float damp1_, damp2_;
    float store_;
    float feedback_;

    std::unique_ptr<float[]> buffer_;
    const size_t size_;
    size_t index_;

    comb_t(size_t size)
        : buffer_(new float[size])
        , size_(size)
        , index_(0)
        , store_(0.f)
        , damp1_(0.f)
        , damp2_(0.f)
        , feedback_(0.f)
    {
        assert(buffer_.get());
        for (size_t i = 0; i<size; ++i)
            buffer_[i] = 0.f;
    }

    float process(float in) {
        float output = buffer_[index_];
        output += c_renorm;
        store_ = (output*damp2_)+(store_*damp1_);
        store_ += c_renorm;
        buffer_[index_] = in+(store_*feedback_);
        if (++index_>=size_)
            index_ = 0;
        return output;
    }
};

struct allpass_t
{
    std::unique_ptr<float[]> buffer_;
    const size_t size_;
    size_t index_;
    float feedback_;

    allpass_t(size_t size)
        : buffer_(new float[size])
        , size_(size)
        , index_(0)
        , feedback_(.5f)
    {
        assert(buffer_.get());
        for (size_t i = 0; i<size; ++i)
            buffer_[i] = 0.f;
    }

    float process(float in) {
        float bufout = buffer_[index_]+c_renorm;
        float output = bufout-in;
        buffer_[index_] = in+(bufout * feedback_);
        if (++index_>=size_)
            index_ = 0;
        return output;
    }
};
} // namespace {}

namespace source_chip {

struct reverb_t::detail_t {
    detail_t();

    // Comb filters
    std::array<comb_t, c_numcombs> cl_;
    std::array<comb_t, c_numcombs> cr_;

    // Allpass filters
    std::array<allpass_t, c_numallpasses> al_;
    std::array<allpass_t, c_numallpasses> ar_;

    // 
    float gain;
    float roomsize, roomsize1;
    float damp, damp1;
    float wet0, wet1, wet2;
    float dry;
    float width;
    float mode;

    // 
    void process(
        std::array<sound_t, 4> & buffers,
        const uint32_t samples);

    //
    void recalc();
};

reverb_t::detail_t::detail_t()
    : cl_{ctl1, ctl2, ctl3, ctl4, ctl5, ctl6, ctl7, ctl8}
    , cr_{ctr1, ctr2, ctr3, ctr4, ctr5, ctr6, ctr7, ctr8}
    , al_{atl1, atl2, atl3, atl4}
    , ar_{atr1, atr2, atr3, atr4}
{
}

reverb_t::~reverb_t() {
}

void reverb_t::detail_t::recalc()
{
    wet1 = wet0 * (width/2.f+0.5f);
    wet2 = wet0 * ((1.f-width)/2.f);

    roomsize1 = roomsize;
    damp1 = damp;
    gain = c_fixedgain;

    for (uint32_t i = 0; i<c_numcombs; i++) {
        cl_[i].feedback_ = roomsize1;
        cr_[i].feedback_ = roomsize1;
    }

    for (uint32_t i = 0; i<c_numcombs; i++) {
        cl_[i].damp1_ = damp1;
        cr_[i].damp1_ = damp1;
        cl_[i].damp2_ = 1-damp1;
        cr_[i].damp2_ = 1-damp1;
    }
}

void reverb_t::detail_t::process(
    std::array<sound_t, 4> & buffers,
    const uint32_t samples)
{
    // 
    assert(samples<buffers[0].size());

    float * out_l = buffers[0].data().data();
    float * out_r = buffers[1].data().data();
    float * in_l = buffers[2].data().data();
    float * in_r = buffers[3].data().data();

    // for each sample
    for (uint32_t i = 0; i<samples; ++i) {

        // input samples
        const float inl = in_l[i];
        const float inr = in_r[i];

        // 
        float outL = 0.f, outR = 0.f;
        float input = (inl+inr) * gain;

        // Accumulate comb filters in parallel
        for (uint32_t i = 0; i<c_numcombs; i++) {
            outL += cl_[i].process(input);
            outR += cr_[i].process(input);
        }

        // Feed through allpasses in series
        for (uint32_t i = 0; i<c_numallpasses; i++) {
            outL = al_[i].process(outL);
            outR = ar_[i].process(outR);
        }

        // Calculate output REPLACING anything already there
        out_l[i] += (outL*wet1)+(outR*wet2);
        out_r[i] += (outR*wet1)+(outL*wet2);
    }
}

reverb_t::reverb_t(uint32_t sample_rate)
    : d_(new detail_t)
{
    assert(d_);

    set_wetdry(0.5f);
    set_room_size(c_initialroom);
    set_damp(c_initialdamp);
    set_width(c_initialwidth);

    recalc();
}

void reverb_t::process(
    std::array<sound_t, 4> & buffers,
    const uint32_t samples)
{
    assert(d_);
    d_->process(buffers, samples);
}

void reverb_t::set_width(float value)
{
    assert(d_);
    d_->width = value;
}

void reverb_t::set_wetdry(float value)
{
    assert(d_);
    d_->wet0 = value * c_scalewet;
    d_->dry = (1.f-value) * c_scaledry;
}

void reverb_t::set_damp(float value)
{
    assert(d_);
    d_->damp = value * c_scaledamp;
}

void reverb_t::set_room_size(float value)
{
    assert(d_);
    d_->roomsize = (value * c_scaleroom)+c_offsetroom;
}

void reverb_t::recalc()
{
    assert(d_);
    d_->recalc();
}

} // namespace source_chip
} // namespace tengu
