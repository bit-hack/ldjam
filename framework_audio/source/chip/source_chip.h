#pragma once
#include <stdint.h>
#include <array>
#include <queue>
#include <mutex>

#include "decimate.h"
#include "../../audio.h"

/* todo:
 *  add lerp glide to sources
 */

namespace tengu {
namespace source_chip {

/* simple fixed size stack */
template <typename type_t, size_t SIZE>
struct small_stack_t {

    small_stack_t()
        : head_(0)
    {}

    bool push(const type_t & in) {
        if (!full()) {
            bin_[head_++] = in;
            return true;
        }
        return false;
    }

    bool pop(type_t & out) {
        if (!empty()) {
            out = bin_[--head_];
            return true;
        }
        return false;
    }

    const type_t & top() const {
        assert(size());
        return bin_[head_-1];
    }

    void clear() {
        head_ = 0;
    }

    void discard() {
        if (!empty()) {
            --head_;
        }
    }

    bool empty() const {
        return head_<=0;
    }

    bool full() const {
        return head_>=SIZE-1;
    }

    size_t size() const {
        return head_;
    }

    void remove(const type_t & v) {
        size_t j = 0;
        for (size_t i = 0; i<head_; ++i) {
            if (bin_[i]==v)
                continue;
            bin_[j++] = bin_[i];
        }
        head_ = j;
    }

protected:
    std::array<type_t, SIZE> bin_;
    size_t head_;
};


/* attack decay envelope */
struct env_ad_t {
    env_ad_t(float sample_rate_ = 44100.f)
        : ms_conv_(sample_rate_/1000.f)
        , attack_(1.f)
        , decay_(1.f)
        , env_(0.f)
        , one_shot_(false)
    {}

    void set_attack(float ms) {
        attack_ = 1.f/maxv(1.f, ms * ms_conv_);
    }

    void set_decay(float ms) {
        decay_ = 1.f/maxv(1.f, ms * ms_conv_);
    }

    void kill() {
        stage_ = e_end;
        env_ = 0.f;
    }

    void note_on(bool retrigger) {
        stage_ = e_attack;
        if (retrigger) {
            env_ = 0.f;
        }
    }

    void note_off() {
        if (stage_<e_decay) {
            stage_ = e_decay;
        }
    }

    float next() {
        switch (stage_) {
        case (e_attack):
            if ((env_ += attack_)>1.f) {
                env_ = 1.f;
                stage_ = one_shot_ ? e_decay : e_hold;
            }
            return env_;
        case (e_hold):
            return 1.f;
        case (e_decay):
            if ((env_ -= decay_)<0.f) {
                env_ = 0.f;
                stage_ = e_end;
            }
            return env_;
        case (e_end):
        default:
            return 0.f;
        }
    }

    float get_lin() const {
        return env_;
    }

    float get_exp() const {
        switch (stage_) {
        case (e_attack):
            return 1-(env_-1)*(env_-1);
        default:
            return 0+(env_-1)*(env_-1);
        }
    }

protected:
    enum {
        e_attack = 0,
        e_hold,
        e_decay,
        e_end,
    } stage_;
    float attack_, decay_;
    float env_;
    bool one_shot_;
    const float ms_conv_;
};


/* low frequency sinwave oscillator
 */
struct lfo_sin_t {
    float a_;
    float s_[2];
    const float sample_rate_;

    lfo_sin_t(const float sample_rate)
        : sample_rate_(sample_rate)
    {
        init(5.f);
    }

    void init(float freq) {
        const bool stable = true;
        a_ = 2.f*C_PI*freq/sample_rate_;    // stable at very low freq.
//      a_ = sinf(C_PI*freq/sample_rate_);  // better for higher freq.
        s_[0] = 1.f;
        s_[1] = 0.f;
    }

    float tick() {
        s_[0] -= a_*s_[1];
        s_[1] += a_*s_[0];
        return s_[0];
    }

    float sin_part() const {
        return s_[0];
    }

    float cos_part() const {
        return s_[1];
    }

    void normalize() {
        const float mag = (s_[1]*s_[1]+s_[0]*s_[0]);
        const float nrm = (3.f-mag)*.5f; // approximation of 1/sqrt(x)
        s_[0] *= nrm;
        s_[1] *= nrm;
    }
};


/* mono floating point sound buffer
 */
struct sound_t {
    static const size_t _SIZE = 1024;

    sound_t()
        : decimate_()
        , dither_(1)
        , dc_(0.f) {
        data_.fill(0.f);
    }

    void clear() {
        data_.fill(0.f);
    }

    void reset() {
        data_.fill(0.f);
        dc_ = 0.f;
        dither_ = 1;
        decimate_.reset();
    }

    size_t size() const {
        return data_.size();
    }

    std::array<float, _SIZE> & data() {
        return data_;
    }

    const std::array<float, _SIZE> & data() const {
        return data_;
    }

    size_t render(int32_t * l,
                  const size_t num_samples);

    size_t render(float * l,
                  const size_t num_samples);

protected:
    decimate_9_t decimate_;
    uint64_t dither_;
    float dc_;
    std::array<float, _SIZE> data_;
};


/* midi event
 *
 * this is essentially a wrapper for a midi event.
**/
struct event_t {
    enum: uint8_t {
        e_note_on,  // chan note vel
        e_note_off, // chan note
        e_cc,       // chan cc val
    };

    bool operator == (const event_t & e) const {
        return data_[1]==e.data_[1];
    }

    uint32_t delta_;
    uint8_t type_;
    uint8_t data_[3];
};
typedef std::queue<event_t> event_queue_t;


/* sound source base class helper
 *
 * use the CRTP here ot abstract some of the complexityy of creating a sound
 * source and handling the main processing loop.
**/
template <typename derived_t>
struct source_t {

    /* contract:
     *
     *   derived_t {
     *       size_t _render(size_t samples, sound_t &out);
     *       void on_note_on(const event_t &event, bool retrigger);
     *       void on_note_off(const event_t &event);
     *       void on_cc(const event_t &event);
     *   };
     */

    source_t(event_queue_t &stream, float sample_rate)
        : queue_(stream)
        , sample_rate_(sample_rate)
    {
    }

    size_t render(size_t samples, std::array<sound_t, 4> & out) {
        derived_t & derived = *static_cast<derived_t*>(this);
        // can only render up to buffer size
        assert(samples<=out[0].size());
        // dispatch any pending events
        while (!queue_.empty()) {
            event_t & e = queue_.front();
            switch (e.type_) {
            case (event_t::e_note_on):
                derived.on_note_on(e, note_.empty());
                note_.push(e);
                break;

            case (event_t::e_note_off):
                note_.remove(e);
                if (note_.empty()) {
                    derived.on_note_off(event_t{});
                }
                else {
                    derived.on_note_on(note_.top(), false);
                }
                break;

            case (event_t::e_cc):
                derived.on_cc(e);
                break;

            default:
                assert(!"unknown message type");
            }
            queue_.pop();
        }
        // render out the requested number of samples
        size_t done = derived._render(samples, out);
        assert(done==samples);
        return done;
    }

protected:
    small_stack_t<event_t, 32> note_;
    event_queue_t & queue_;
    const float sample_rate_;
};

/* reverb module (freeverb)
**/
struct reverb_t {
    reverb_t(uint32_t sample_rate);
    ~reverb_t();

    void process(
        std::array<sound_t, 4> & buffers,
        const uint32_t samples);

    void set_width(float);
    void set_wetdry(float mix);
    void set_damp(float);
    void set_room_size(float);

    // call after adjusting parameters
    void recalc();

protected:
    struct detail_t;
    std::unique_ptr<detail_t> d_;
};

/* pulse wave sound source
 */
struct pulse_t: public source_t<pulse_t> {
    friend struct source_t<pulse_t>;

    pulse_t(event_queue_t & stream, float sample_rate = 44100.f * 2.f)
        : source_t<pulse_t>(stream, sample_rate)
        , duty_(.5f)
        , lvol_(.0f)
        , rvol_(.0f)
        , accum_(.0f)
        , period_(100.f)
        , vibrato_(0.f)
        , env_(sample_rate_)
        , lfo_(sample_rate_)
        , send_(.5f)
    {
        set_freq(100.f);
        lfo_.init(5.f);
        env_.set_attack(1.f);
        env_.set_decay(1.f);
    }

    void set_duty(float duty) {
        duty_ = duty;
        offset_ = period_ * duty_;
    }

    void set_freq(float freq) {
        period_ = sample_rate_/freq;
        offset_ = period_ * duty_;
    }

protected:
    // internal render function
    size_t _render(size_t samples, std::array<sound_t, 4> & out);

    // cc values
    enum: uint8_t {
        e_attack = 0,
        e_decay,
        e_duty,
        e_vibrato,
        e_send
    };

    // event message handlers
    void on_note_on(const event_t & event, bool retrigger);
    void on_note_off(const event_t & event);
    void on_cc(const event_t & event);

    env_ad_t env_;
    lfo_sin_t lfo_;
    float duty_, period_, offset_, accum_, vibrato_;
    // stereo
    float lvol_, rvol_;
    // send ammount
    float send_;
};


/* nintendo entertainment system triangle wave
 */
struct nestr_t: public source_t<nestr_t> {
    friend struct source_t<nestr_t>;

    nestr_t(event_queue_t & stream, float sample_rate)
        : source_t<nestr_t>(stream, sample_rate)
        , volume_(0.f)
        , accum_(0.f)
        , env_(sample_rate)
    {
        volume_ = 1.f;
        set_freq(100.f);
        env_.set_attack(1.f);
        env_.set_decay(1.f);
    }

    void set_freq(float freq) {
        delta_ = (32.f/sample_rate_) * freq;
    }

protected:
    // internal render function
    size_t _render(size_t samples, std::array<sound_t, 4> & out);

    // cc values
    enum: uint8_t {
        e_attack = 0,
        e_decay,
    };

    // event message handlers
    void on_note_on(const event_t & event, bool retrigger);
    void on_note_off(const event_t & event);
    void on_cc(const event_t & event);

    env_ad_t env_;
    float accum_, delta_, volume_;
};


/* retro LFSR noise source
 */
struct noise_t: public source_t<noise_t> {
    friend struct source_t<noise_t>;

    noise_t(event_queue_t & stream, float sample_rate = 44100.f * 2.f)
        : source_t<noise_t>(stream, sample_rate)
        , lfsr_(1)
        , period_(100)
        , counter_(100)
        , volume_(0.f)
        , env_(sample_rate)
    {
        volume_ = 1.f;
        env_.set_attack(1.f);
        env_.set_decay(1.f);
    }

    void set_volume(float volume) {
        volume_ = volume;
    }

    void set_period(uint32_t period) {
        period_ = period;
    }

protected:
    // internal render function
    size_t _render(size_t samples, std::array<sound_t, 4> & out);

    // cc values
    enum: uint8_t {
        e_attack = 0,
        e_decay,
        e_period
    };

    // event message handlers
    void on_note_on(const event_t & event, bool retrigger);
    void on_note_off(const event_t & event);
    void on_cc(const event_t & event);

    env_ad_t env_;
    uint32_t lfsr_, period_, counter_;
    float volume_;
};


/* simple thread safe queue
 */
template <typename type_t>
struct queue_t {

    void push(const type_t & in) {
        std::lock_guard<std::mutex> guard(mutex_);
        q_.push(in);
    }

    bool pop(type_t & out) {
        std::lock_guard<std::mutex> guard(mutex_);
        if (q_.empty()) {
            return false;
        }
        else {
            out = q_.front();
            q_.pop();
            return true;
        }
    }

protected:
    std::queue<event_t> q_;
    std::mutex mutex_;
};


/* sound chip channel config
 */
struct config_t {

    enum type_t {
        e_pulse,
        e_noise,
        e_nestr,
    };

    struct entry_t {
        type_t type_;
        uint32_t channel_;
    };

    void clear() {
        channels_.clear();
    }

    void operator += (const entry_t & e) {
        channels_.push_back(e);
    }

    const std::vector<entry_t> & channels() const {
        return channels_;
    }

protected:
    std::vector<entry_t> channels_;
};


struct audio_source_chip_t:
    public audio_source_t {

    audio_source_chip_t(uint32_t sample_rate);
    ~audio_source_chip_t();

    void init(const config_t & config);

    virtual void render(const mix_out_t &) override;

    // render as mono float
    void render(float * out, size_t samples);

    // render as sterio float
    void render(
        float * lout,
        float * rout,
        const size_t samples);

    // push an audio event
    void push(const event_t & event) {
        input_.push(event);
    }

protected:
    void _scatter_events();
    size_t _fill_buffer(size_t samples);

    reverb_t verb_;

    // thread safe input queue
    queue_t<event_t> input_;
    // mix buffers
    std::array<sound_t, 4> buffers_;
    // event buffer per channel
    std::array<std::queue<event_t>, 16> event_;
    // individual sound source
    std::vector<std::unique_ptr<pulse_t>> source_pulse_;
    std::vector<std::unique_ptr<noise_t>> source_noise_;
    std::vector<std::unique_ptr<nestr_t>> source_nestr_;

    const uint32_t sample_rate_;
};
} // namespace source_chip
} // namespace tengu
