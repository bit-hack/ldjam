#include <cassert>
#include "source_chip.h"

namespace {
template <typename type_t>
constexpr type_t _min(type_t a, type_t b) {
    return (a<b) ? a : b;
}

template <typename type_t>
constexpr type_t _max(type_t a, type_t b) {
    return (a>b) ? a : b;
}

template <typename type_t>
constexpr type_t _clamp(type_t lo, type_t in, type_t hi) {
    return (in<lo) ? lo :
                     ((in>hi) ? hi :
                                in);
}

/* Return clipped input between [-1,+1] */
constexpr float _clip(float in) {
    return _clamp(-1.f, in, 1.f);
}

/* Linear interpolation */
constexpr float _lerp(float a, float b, float i) {
    return a+(b-a) * i;
}

/* Fractional part */
constexpr float _fpart(float in) {
    return in-float(int32_t(in));
}

/* 64 bit xor shift pseudo random number generator */
uint64_t _rand64(uint64_t & x) {
    x ^= x>>12;
    x ^= x<<25;
    x ^= x>>27;
    return x;
}

/* Triangular noise distribution [-1,+1] tending to 0 */
float _dither(
    uint64_t & x)
{
    static const uint32_t fmask = (1<<23)-1;
    union { float f; uint32_t i; } u, v;
    u.i = (uint32_t(_rand64(x)) & fmask)|0x3f800000;
    v.i = (uint32_t(_rand64(x)) & fmask)|0x3f800000;
    float out = (u.f+v.f-3.f);
    return out;
}
} // namespace {}

void sound_t::_render_stereo(
    int16_t * out,
    size_t length)
{
    // makeup gain from float to int16_t
    static const float c_gain = float(0x7000);
    // local copy of dither seed
    uint64_t dither = dither_;
    decimate_9_t & dec = decimate_;
    float dc = dc_;
    // mix down buffer to output region
    float * src = &data_[0];
    while (length--) {
        //
        float a = dec(src[0], src[1]);
        src += 2;
        // clip
        float b = _clip(a);
        // apply makeup gain
        float c = b * c_gain;
        // dither
        float d = c + _dither(dither);
        // dc removal
        dc = _lerp(dc_, d, 0.0001f);
        // write to output buffer
        *(out++) = int16_t(d);
    }
    // save dither seed back out
    dither_ = dither;
    dc_ = dc;
}

void sound_t::render(
    int16_t * out,
    size_t length,
    sound_t::format_t fmt)
{
    switch (fmt) {
    case (e_stereo):
        _render_stereo(out, length);
    default:
        assert(!"unknown mixdown type");
    }
}

#if 0
/* Render sound sources through mixdown and into the output stream */
void audio_source_chip_t::sound_render(
    sound_t * buffer,
    int16_t * out,
    size_t length,
    source_t * source)
{
    // buffer size halved since we are 2x oversampling
    const size_t c_buffer_size = buffer->data_.size()/2;
    // while there are samples to render
    while (length) {
        // max samples we can render
        const size_t count = _min(length, c_buffer_size);
        // clear the sound buffer
        sound_clear(buffer);
        // for all sound sources
        for (source_t * s = source; s && s->render_; ++s) {
            // if this source is enabled
            if (s->enable_) {
                // render into the intermediate buffer
                s->render_(&buffer->data_[0],
                           count*2,
                           s->user_,
                           s->volume_);
            }
        }
        // mixdown into the output buffer
        sound_mixdown(buffer, out, count);
        // advance in samples
        length -= count;
        out    += count;
    }
}

void sound_source_pulse(
    float * out,
    size_t length,
    void * user,
    float attenuation)
{
    assert(user && out && length);
    pulse_t & pulse = *(pulse_t*)user;

    float accum = pulse.accum_;
    const float period = pulse.period_;
    const float offset = pulse.offset_;
    const float volume = pulse.volume_ * attenuation;
    // dont render if over nyquist
    if (period<=2.f) return;
    // while there are samples to render
    while (length--) {
        // tick the sqaure wave counter
        if ((accum -= 1.f)<0.f) {
            accum += period;
        }
        // apply duty offset
        float a = accum-offset;
        // turn into pulse wave
        float b = a > 0.f ? 1.f : -1.f;
        // apply volume attenuation
        float c = b * volume;
        // move into output buffer
        *(out++) += c;
    }
    // copy period back to structure
    pulse.accum_ = accum;
}

/* SOUND SOURCE: NES Linear Feedback Shift Register */
void sound_source_lfsr(
    float * out,
    size_t length,
    void * user,
    float attenuation)
{
    assert(user && out && length);
    lfsr_t & lfsr = *(lfsr_t*)user;

    uint32_t reg            = lfsr.lfsr_;
    uint32_t counter        = lfsr.counter_;
    const uint32_t period   = lfsr.period_;
    const float volume      = lfsr.volume_ * attenuation;
    // zero volume skip
    if (volume<=0.f)
        return;
    // while there are samples to render
    while (length--) {
        // get current noise state
        float value = (reg&1) ? 1.f : -1.f;
        // apply attenuation
        *(out++) += value * volume;
        // if the shift register should be clocked
        if (counter==0) {
            // calculate new bit (taps{6,0})
            uint32_t bit = ((reg>>1)^reg);
            // shift out
            reg >>= 1;
            // shift in new bit
            reg |= (bit<<14)&0x4000;
            // reset counter
            counter += period;
        }
        else {
            // decrement the lfsr clock counter
           --counter;
        }
    }
    // copy shift register back into structure
    lfsr.lfsr_    = reg;
    lfsr.counter_ = counter;
}

/* SOUND SOURCE: Band Limited Impulse Train Pulse Wave Generator */
void sound_source_blit(
    float * dst,
    size_t length,
    void * user,
    float attenuation)
{
    assert(user && dst && length);
    blit_t & blit = *(blit_t*)user;

    // defined in blip_table.cpp
    extern const float g_blip_table[];

    const uint32_t c_ring_size  = blit_t::c_ring_size;
    const float    c_leak       = 0.999f;
    const uint32_t c_blip_count = 16;
    const uint32_t c_blip_size  = 32;

    assert(c_blip_size==c_ring_size);

    auto   & ring   = blit.ring_;
    float    accum  = blit.accum_;
    float    out    = blit.out_;
    uint32_t index  = blit.index_;
    uint32_t edge   = blit.edge_;

    const float volume = blit.volume_ * attenuation;

    if ((blit.hcycle_[0]<=0.f)||(blit.hcycle_[1]<=0.f)) {
        return;
    }

    assert(blit.hcycle_[0] > 0.f);
    assert(blit.hcycle_[1] > 0.f);
    // while there are samples left to render
    while (length) {
        // do we need to output a blip
        while (accum < 0.f) {
            // scale factor for this edge and volume
            const float scale = (edge&1) ? volume : -volume;
            // flip pulse edge
            edge ^= 0x1;
            // find lerp data
            float    r = (1.f+accum) * float(c_blip_count);
            uint32_t a = int32_t(r+0) & (c_blip_count-1);
            uint32_t b = int32_t(r+1) & (c_blip_count-1);
            float    l = _fpart(r);
            // locate the blip tables that bracket this index
            const float * blip_a = &g_blip_table[a * c_blip_size];
            const float * blip_b = &g_blip_table[b * c_blip_size];
            // for all samples in the blip
            for (uint32_t i = 0; i<c_ring_size; ++i) {
                // lerp blip value
                float v = _lerp(blip_a[i], blip_b[i], l);
                // sum into blip ring buffer
                ring[(index+i)%c_ring_size] += v * scale;
            }
            // reset the period with this duty cycle period
            accum += blit.hcycle_[edge&1];
        }
        // number of samples before next blip or end
        uint32_t interval = uint32_t(accum+1.f);
        uint32_t count = _min<uint32_t>(length, interval);
        assert(count > 0);
        length -= count;
        // advance the period by the amount we will render
        accum -= float(count);
        // render using the blip ring buffer
        for (uint32_t i = 0; i<count; ++i, ++dst) {
            // get this ring buffer slot
            float & slot = ring[(index++)%c_ring_size];
            // integrate using blip ring buffer
            out  += slot;
            out  *= c_leak;
            *dst += out;
            // clear ring buffer slot
            slot  = 0.f;
        }
    }
    // pack blip state back into struct
    blit.out_   = out;
    blit.accum_ = accum;
    blit.index_ = index;
    blit.edge_  = edge;
}

/* SOUND SOURCE: Nintendo Entertainment System APU Triangle */
void sound_source_nestri(
    float * out,
    size_t length,
    void * user,
    float attenuation)
{
#define A(X) ((X)/7.5f-1.f)
    static const std::array<float, 32> tri_table = {
        A(15),  A(14),  A(13),  A(12),  A(11), A(10), A(9), A(8),
        A(7),  A(6),  A(5),  A(4),  A(3), A(2), A(1), A(0),
        A(0),  A(1),  A(2),  A(3),  A(4), A(5), A(6), A(7),
        A(8),  A(9),  A(10),  A(11),  A(12), A(13), A(14), A(15),
    };
#undef A

    assert(user && out && length);
    nestri_t & nestri = *(nestri_t*)user;

    float accum = nestri.accum_;
    const float delta = nestri.delta_;
    const float volume = nestri.volume_ * attenuation;

    // while there are samples to render
    while (length--) {
        // tick the sqaure wave counter
        if ((accum -= delta)<0.f) {
            accum += 32.f;
        }
        // index into lookup table
#if 0
        size_t a = (size_t(accum)+0)&0x1f;
        size_t b = (size_t(accum)+1)&0x1f;
        float  i = accum-float(int(accum));
        float  v = _lerp(tri_table[a], tri_table[b], i);
#else
        float  v = tri_table[size_t(accum)&0x1f];
#endif
        // apply volume attenuation
        float c = v * volume;
        // move into output buffer
        *(out++) += c;
    }
    // copy period back to structure
    nestri.accum_ = accum;
}
#endif