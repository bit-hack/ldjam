#include <cmath>
#include <cassert>
#include "source_chip.h"
#include "../../../framework_core/common.h"


namespace {
inline float note_to_freq(int32_t note) {
    float x = float(note-69)/12.f;
    float y = powf(2.f, x);
    return 440.f * y;
}

/* Return clipped input between [-1,+1] */
constexpr float _clip(float in) {
    return clampv(-1.f, in, 1.f);
}

/* 64 bit xor shift pseudo random number generator */
uint64_t _rand64(uint64_t & x) {
    x ^= x>>12;
    x ^= x<<25;
    x ^= x>>27;
    return x;
}

/* Triangular noise distribution [-1,+1] tending to 0 */
float _dither(uint64_t & x) {
    static const uint32_t fmask = (1<<23)-1;
    union { float f; uint32_t i; } u, v;
    u.i = (uint32_t(_rand64(x)) & fmask)|0x3f800000;
    v.i = (uint32_t(_rand64(x)) & fmask)|0x3f800000;
    float out = (u.f+v.f-3.f);
    return out;
}
} // namespace {}

namespace source_chip {

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- SOUND

size_t sound_t::render(
    int32_t * l,
    int32_t * r,
    size_t length)
{
    // number of samples to output
    const size_t count = min2(data_.size()/2, length);
    // makeup gain from float to int16_t
    static const float c_gain = float(0x7000);
    // local copy of dither seed
    uint64_t dither = dither_;
    decimate_9_t & dec = decimate_;
    float dc = dc_;
    // mix down buffer to output region
    float * src = &data_[0];
    for (size_t i = 0; i<count; ++i) {
        // decimate
        const float s0 = dec(src[i*2+0], src[i*2+1]);
        src += 2;
        // remove dc
        const float s1 = s0 - dc;
        dc = lerp<float>(dc, s1, 0.0001f);
        // clip
        const float s2 = _clip(s1);
        // apply makeup gain
        const float s3 = s2 * c_gain;
        // dither
        const float s4 = s3+_dither(dither);
        // write to output buffer
        l[i] = int16_t(s4);
        r[i] = int16_t(s4);
    }
    // save dither seed back out
    dither_ = dither;
    dc_ = dc;
    // return number of samples written
    return count;
}

size_t sound_t::render(float *out, size_t num_samples) {
    const size_t count = min2(num_samples, data_.size() / 2);
    const float * src = data_.data();
    decimate_9_t & dec = decimate_;
    float dc = dc_;
    for (int i=0; i<count; ++i) {
        // decimate sample in buffer
        const float s0 = dec(src[i*2+0], src[i*2+1]);
        // remove dc
        const float s1 = s0 - dc;
        dc = lerp(dc, s1, 0.0001f);
        // write output sample
        out[i] = s1;
    }
    dc_ = dc;
    return count;
}

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- AUDIO SOURCE CHIP

void audio_source_chip_t::init(const config_t & config) {

    const float sample_rate = 44100.f * 2;

    for (const config_t::entry_t & e : config.channels()) {

        const uint32_t channel = e.channel_;
        assert(channel < event_.size());

        event_queue_t & stream = event_[channel];

        switch (e.type_) {
        case (config_t::e_pulse):
            source_pulse_.push_back(new pulse_t(stream, sample_rate));
            break;
        case (config_t::e_noise):
            source_noise_.push_back(new noise_t(stream, sample_rate));
            break;
        case (config_t::e_nestr):
            source_nestr_.push_back(new nestr_t(stream, sample_rate));
            break;
        }
    }
}

void audio_source_chip_t::_scatter_events() {
    // scatter pending events to channel pigeon holes
    event_t event;
    while (input_.pop(event)) {
        const uint8_t slot = event.data_[0];
        event_[slot].push(event);
    }
}

size_t audio_source_chip_t::_fill_buffer(size_t samples) {
    // wipe the buffer before we accumulate into it
    buffer_.clear();
    // find samples remaining
    size_t count = min2(samples, buffer_.size());
    // render from all sources
    for (auto *src:source_pulse_) {
        src->render(count, buffer_);
    }
    // return number of samples written
    return count;
}

void audio_source_chip_t::render(const mix_out_t & mix) {
    _scatter_events();
    // loop until all samples are emitted
    size_t remaining = mix.count_;
    while (remaining) {
        // find samples remaining
        size_t count = min2(remaining * 2, buffer_.size());

        // todo: <-------- pop events from event file

        // render from all audio sources
        size_t rendered = _fill_buffer(count);
        assert(rendered > 0);
        // render out the samples in our buffer
        remaining -= buffer_.render(mix.left_, mix.right_, rendered / 2);
    }
}

void audio_source_chip_t::render(float * out,
                                 size_t samples) {
    // dispatch pending events
    _scatter_events();
    // loop until all samples are emitted
    size_t remaining = samples;
    while (remaining) {
        // find samples remaining
        const size_t count = min2(remaining * 2, buffer_.size());

        // todo: <-------- pop events from event file

        // render from all audio sources
        const size_t filled = _fill_buffer(count);
        assert(filled > 0);
        // render out the samples in our buffer
        size_t written = buffer_.render(out, filled / 2);
        // advance the stream
        remaining -= written;
        out += written;
    }
}

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- PULSE

size_t pulse_t::_render(
    size_t length,
    sound_t &out)
{
    lfo_.normalize();
    //
    const float period = period_;
    const float offset = offset_;
    const float volume = volume_;
    const size_t count = min2(length, out.data().size());
    float accum = accum_;
    float * dst = out.data().data();
    // do not render if over nyquist
    if (period<=2.f) {
        assert(!"HELP!");
        return 0;
    }
    // while there are samples to render
    for (size_t i = 0; i<count; ++i) {
        // vib contribution
        const float v = lfo_.tick() * vibrato_;
        // tick the square wave counter
        if ((accum -= (1.f + v)) < 0.f) {
            accum += period;
        }
        // apply duty offset
        float a = accum - offset;
        // turn into pulse wave
        float b = (a > 0.f) ? 1.f : -1.f;
        // apply volume attenuation
        float c = b * volume * env_.next();
        // move into output buffer
        dst[i] += c;
    }
    // copy period back to structure
    accum_ = accum;
    return count;
}

void pulse_t::on_note_on(const event_t & event) {
    env_.note_on(true);
    const uint8_t note = event.data_[1];
    const uint8_t velo = event.data_[2];
    set_freq(note_to_freq(note));
    volume_ = (1.f/256.f) * (float)velo;
}

void pulse_t::on_note_off(const event_t & event) {
    env_.note_off();
}

void pulse_t::on_cc(const event_t & event) {
    const uint8_t offset = 14;
    const uint8_t value = event.data_[2];
    switch (event.data_[1] - offset) {
    case (e_attack):
        env_.set_attack(value*4.f);
        break;
    case (e_decay):
        env_.set_decay(value*4.f);
        break;
    case (e_duty):
        set_duty((1.f/256.f) * value);
        break;
    case (e_vibrato):
        vibrato_ = (1.f/256.f) * value;
        break;
    default:
        break;
    }
}

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- NESTRI

void nestr_t::on_note_on(const event_t &event) {
    env_.note_on(true);
}

void nestr_t::on_note_off(const event_t &event) {
    env_.note_off();
}

void nestr_t::on_cc(const event_t &event) {
}

size_t nestr_t::_render(size_t length, sound_t &out) {
#define A(X) ((X)/7.5f-1.f)
    static const std::array<float, 32> tri_table = {
        A(15),  A(14),  A(13),  A(12),  A(11), A(10), A(9),  A(8),
        A(7),   A(6),   A(5),   A(4),   A(3),  A(2),  A(1),  A(0),
        A(0),   A(1),   A(2),   A(3),   A(4),  A(5),  A(6),  A(7),
        A(8),   A(9),   A(10),  A(11),  A(12), A(13), A(14), A(15),
    };
#undef A
    float accum = accum_;
    const float delta = delta_;
    const float volume = volume_;
    const size_t count = min2(length, out.data().size());
    float * dst = out.data().data();
    // while there are samples to render
    for (size_t i = 0; i<count; ++i) {
        // tick the counter
        if ((accum -= delta)<0.f) {
            accum += float(tri_table.size());
        }
#if 0
        size_t a = (size_t(accum)+0)&0x1f;
        size_t b = (size_t(accum)+1)&0x1f;
        float  i = accum-float(int(accum));
        float  v = _lerp(tri_table[a], tri_table[b], i);
#else
        // index into lookup table
        float  v = tri_table[size_t(accum)&0x1f];
#endif
        // apply volume attenuation
        float c = v * volume * env_.next();
        // move into output buffer
        dst[i] += c;
    }
    // copy period back to structure
    accum_ = accum;
    return count;
}

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- NOISE

void noise_t::on_note_on(const event_t & event) {
    const uint8_t note = event.data_[1];
    set_period(note_to_freq(note));
    env_.note_on(true);
}

void noise_t::on_note_off(const event_t &event) {
    env_.note_off();
}

void noise_t::on_cc(const event_t &event) {
}

size_t noise_t::_render(size_t length, sound_t &out) {
    const float volume = volume_;
    // zero volume skip
    if (volume<=0.f)
        return 0;
    const uint32_t period = period_;
    const size_t count = min2(length, out.data().size());
    uint32_t reg = lfsr_;
    uint32_t counter = counter_;
    float * dst = out.data().data();
    // while there are samples to render
    for (size_t i = 0; i<count; ++i) {
        // get current noise state
        float value = (reg&1) ? 1.f : -1.f;
        // apply attenuation
        dst[i] += value * volume * env_.next();
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
    lfsr_ = reg;
    counter_ = counter;
    return count;
}
} // namespace source_chip
