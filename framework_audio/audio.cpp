#include <set>
#include <map>
#include <mutex>
#include <cassert>
#include <vector>
#include <array>
#include "audio.h"
#include "../framework_core/common.h"

namespace {
    struct info_wave_t {
        const wave_t * wave_;
        uint64_t frame_;    // 0xffff fixed point
        uint64_t delta_;    // 0xffff fixed point
        uint16_t volume_;   // 0x00ff fixed point
        bool looping_;
    };

    struct info_vorbis_t {
        int dummy_;
    };
}

struct audio_t::detail_t {

    static const uint32_t C_BUFFER_SIZE = 1024;

    std::map<const wave_t*, info_wave_t> waves_;
    std::vector<play_wave_t> msg_wave_;
    std::vector<play_vorbis_t> msg_vorbis_;
    std::mutex mutex_;

    std::array<int32_t, C_BUFFER_SIZE> mix_l_;
    std::array<int32_t, C_BUFFER_SIZE> mix_r_;

    bool post(const play_wave_t & wave) {
        std::lock_guard<std::mutex> guard(mutex_);
        msg_wave_.push_back(wave);
        return true;
    }

    bool post(const play_vorbis_t & vorbis) {
        std::lock_guard<std::mutex> guard(mutex_);
        msg_vorbis_.push_back(vorbis);
        return true;
    }

    void handle(const play_wave_t & msg) {
        auto itt = waves_.find(msg.wave_);
        if (itt == waves_.end()) {
            if (msg.volume_ <= 0.f) {
                return;
            }
            else {
                info_wave_t info;
                info.wave_ = msg.wave_;
                info.volume_ = uint32_t(msg.volume_ * 0x100);
                info.delta_ = uint64_t(msg.rate_ * 0x10000);
                info.frame_ = 0;
                info.looping_ = msg.looping_;
                waves_[info.wave_] = info;
            }
        }
        else {
            info_wave_t & info = itt->second;
            if (msg.volume_ <= 0.f) {
                waves_.erase(itt);
            }
            else {
                info.volume_ = uint32_t(msg.volume_ * 0x100);
                info.delta_ = uint64_t(msg.rate_ * 0x10000);
                info.looping_ = msg.looping_;
                if (msg.retrigger_) {
                    info.frame_ = 0;
                }
            }
        }
    }

    void handle(const play_vorbis_t & vorbis) {
        // todo
    }

    void check_messages() {
        std::lock_guard<std::mutex> guard(mutex_);
        while (msg_wave_.size()) {
            handle(msg_wave_.back());
            msg_wave_.pop_back();
        }
        while (msg_vorbis_.size()) {
            handle(msg_vorbis_.back());
            msg_vorbis_.pop_back();
        }
    }

    // 8 bit mono
    bool _render_8_1(info_wave_t & info, uint32_t count) {
        const wave_t & wave = *(info.wave_);
        const uint64_t end = uint64_t(wave.num_frames()) * 0x10000llu;
        const int8_t * src = wave.get<int8_t>();
        // get mix down buffer pointers
        int32_t * ml = mix_l_.data();
        int32_t * mr = mix_r_.data();
        // repeat until all samples are done
        for (;;) {
            // render loop
            for (;count && info.frame_ < end; --count) {
                // buffer index point
                const uint64_t index = info.frame_ / 0x10000;
                // index wave file
                const int8_t sample = src[index];
                // write to sample buffer
                *(ml++) += sample * info.volume_;
                *(mr++) += sample * info.volume_;
                // advance sample pointer
                info.frame_ += info.delta_;
            }
            // obey looping
            if (count > 0 && info.looping_)
                info.frame_ = 0;
            else
                break;
        }
        // success
        return true;
    }

    // 8 bit stereo
    bool _render_8_2(info_wave_t & info, uint32_t count) {
        const wave_t & wave = *(info.wave_);
        return false;
    }

    // 16 bit mono
    bool _render_16_1(info_wave_t & info, uint32_t count) {
        const wave_t & wave = *(info.wave_);
        const uint64_t end = uint64_t(wave.num_frames()) * 0x10000llu;
        const int16_t * src = wave.get<int16_t>();
        // get mix down buffer pointers
        int32_t * ml = mix_l_.data();
        int32_t * mr = mix_r_.data();
        // repeat until all samples are done
        for (;;) {
            // render loop
            for (;count && info.frame_ < end; --count) {
                // buffer index point
                const uint64_t index = info.frame_ / 0x10000;
                // index wave file
                const int16_t sample = (src[index] * info.volume_) / 0x100;
                // write to sample buffer
                *(ml++) += sample;
                *(mr++) += sample;
                // advance sample pointer
                info.frame_ += info.delta_;
            }
            // obey looping
            if (count > 0 && info.looping_)
                info.frame_ = 0;
            else
                break;
        }
        // success
        return true;
    }

    // 16 bit stereo
    bool _render_16_2(info_wave_t & info, uint32_t count) {
        const wave_t & wave = *(info.wave_);
        const uint64_t end = uint64_t(wave.num_frames()) * 0x10000llu;
        const int16_t * src = wave.get<int16_t>();
        // get mix down buffer pointers
        int32_t * ml = mix_l_.data();
        int32_t * mr = mix_r_.data();
        // repeat until all samples are done
        for (;;) {
            // render loop
            for (;count && info.frame_ < end; --count) {
                // buffer index point
                const uint64_t index = info.frame_ / 0x10000;
                // index wave file
                const int16_t sample_l = (src[index*2+0] * info.volume_) / 0x100;
                const int16_t sample_r = (src[index*2+1] * info.volume_) / 0x100;
                // write to sample buffer
                *(ml++) += sample_l;
                *(mr++) += sample_r;
                // advance sample pointer
                info.frame_ += info.delta_;
            }
            // obey looping
            if (count > 0 && info.looping_)
                info.frame_ = 0;
            else
                break;
        }
        // success
        return true;
    }

    bool _render(info_wave_t & wave, uint32_t count) {
        const wave_t * sample = wave.wave_;
        switch (sample->bit_depth()) {
        case (8 /* bit */):
            switch (sample->num_channels()) {
            case (1): return _render_8_1(wave, count);
            case (2): return _render_8_2(wave, count);
            default:  assert(!"unsupported channel count");
            }
        case (16 /* bit */):
            switch (sample->num_channels()) {
            case (1): return _render_16_1(wave, count);
            case (2): return _render_16_2(wave, count);
            default:  assert(!"unsupported channel count");
            }
        default:
            assert(!"unsupported bit depth");
        }
        return true;
    }

    // clip sample to signed 16bit integer range
    static constexpr int16_t clip(int32_t x) {
        return int16_t(x >  0x7fff ?  0x7fff :
                      (x < -0x7fff ? -0x7fff : x));
    }

    void _mixdown(int16_t * out, const uint32_t count) {
        for (uint32_t i=0; i<count; ++i) {
            out[i*2+0] = clip(mix_l_[i]);
            out[i*2+1] = clip(mix_r_[i]);
        }
    }

    bool render(int16_t * out, uint32_t count) {
        check_messages();
        while (count) {
            // figure our how many samples to render
            const uint32_t samples = min2(count, C_BUFFER_SIZE);
            count -= samples;
            // clear our mix buffers
            mix_l_.fill(0);
            mix_r_.fill(0);
            // render out any pending samples
            for (auto itt = waves_.begin(); itt != waves_.end();) {
                if (!_render(itt->second, samples)) {
                    itt = waves_.erase(itt);
                }
                else {
                    ++itt;
                }
            }
            // mix down into output stream
            _mixdown(out, samples);
            out += samples * 2;
        }
        return true;
    }
};

audio_t::audio_t()
    : detail_(new audio_t::detail_t)
{
}

bool audio_t::play(const play_wave_t & wave) {
    return detail_->post(wave);
}

bool audio_t::play(const play_vorbis_t & vorbis) {
    return detail_->post(vorbis);
}

bool audio_t::render(int16_t * out, uint32_t count) {
    return detail_->render(out, count);
}

audio_t::~audio_t() {
}
