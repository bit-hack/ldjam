#include <set>
#include <map>
#include <mutex>
#include <cassert>
#include <vector>
#include <array>
#include <deque>

#include "../framework_core/common.h"

#include "audio.h"
#include "stb_vorbis.h"

namespace {
    struct info_wave_t {
        const wave_t * wave_;
        uint64_t frame_;    // 0xffff fixed point
        uint64_t delta_;    // 0xffff fixed point
        uint16_t volume_;   // 0x00ff fixed point
        bool looping_;
    };

    struct info_vorbis_t {

        info_vorbis_t()
            : buffer_()
            , head_(0)
            , tail_(0)
            , finished_(true)
            , loop_(false)
            , stb_(nullptr)
            , vorbis_()
        {
        }

        ~info_vorbis_t() {
            _close();
        }

        void push(const audio_t::play_vorbis_t & v) {
            vorbis_.push_back(v);
        }

        void render(int32_t * left, int32_t * right, int32_t count) {
            assert(left && right && count);
            if (!stb_ || finished_) {
                _enqueue();
            }
            if (stb_) {
                _render(left, right, size_t(count));
            }
        }

    protected:
        static const size_t C_CHANNELS = 2;
        std::array<int16_t, 4096*C_CHANNELS> buffer_;
        size_t head_;
        size_t tail_;
        bool finished_;
        bool loop_;
        stb_vorbis * stb_;
        std::deque<audio_t::play_vorbis_t> vorbis_;

        void _render(int32_t * left, int32_t * right, int32_t count) {
            count *= 2;
            // while we have more samples to render
            while (!finished_ && count) {
                // if the buffer is empty
                if (head_ == tail_) {
                    // decode more data into the buffer
                    head_ = tail_ = 0;
                    head_ = stb_vorbis_get_frame_short_interleaved(
                        stb_, 
                        C_CHANNELS,
                        buffer_.data(),
                        int32_t(buffer_.size()))*C_CHANNELS;
                }
                // no more data in the stream
                if (head_<=0) {
                    if (loop_) {
                        stb_vorbis_seek_start(stb_);
                        continue;
                    }
                    else {
                        finished_ = true;
                        break;
                    }
                }
                // find max number of samples we can write out
                size_t notch = min2(tail_+count, head_);
                const size_t c_num = notch-tail_;
                count -= int32_t(c_num);
                // render out these samples
                assert((c_num&1)==0);
                for (size_t i = tail_; i<notch; i+=2) {
                    *(left++) = buffer_[i+0];
                    *(right++) = buffer_[i+1];
                }
                tail_ = notch;
            }
        }

        void _close() {
            // close decode stream
            if (stb_) {
                stb_vorbis_close(stb_);
                stb_ = nullptr;
            }
            // clear decode buffer
            _clear_buffer();
        }

        void _enqueue() {
            if (stb_) {
                stb_vorbis_close(stb_);
                stb_ = nullptr;
            }
            if (!vorbis_.empty()) {
                const audio_t::play_vorbis_t v = vorbis_.front();
                vorbis_.pop_front();
                assert(v.vorbis_);
                int error = 0;
                stb_ = stb_vorbis_open_memory(
                    v.vorbis_->data(), 
                    int32_t(v.vorbis_->size()),
                    &error,
                    nullptr);
                if (stb_) {
                    loop_ = v.loop_;
                    finished_ = false;
                    _clear_buffer();
                }
            }
        }

        void _clear_buffer() {
            head_ = tail_ = 0;
        }
    };
}

struct audio_t::detail_t {

    static const uint32_t C_BUFFER_SIZE = 1024;

    info_vorbis_t vorbis_;

    std::map<const wave_t*, info_wave_t> waves_;
    std::deque<play_wave_t> msg_wave_;
    std::deque<play_vorbis_t> msg_vorbis_;
    std::mutex mutex_;

    struct mix_t {
        std::array<int32_t, C_BUFFER_SIZE> left_;
        std::array<int32_t, C_BUFFER_SIZE> right_;
    } mix_;

    bool post(const play_wave_t & wave) {
        assert(wave.wave_);
        std::lock_guard<std::mutex> guard(mutex_);
        msg_wave_.push_back(wave);
        return true;
    }

    bool post(const play_vorbis_t & vorbis) {
        assert(vorbis.vorbis_);
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
        vorbis_.push(vorbis);
    }

    void check_messages() {
        std::lock_guard<std::mutex> guard(mutex_);
        while (msg_wave_.size()) {
            handle(msg_wave_.front());
            msg_wave_.pop_front();
        }
        while (msg_vorbis_.size()) {
            handle(msg_vorbis_.front());
            msg_vorbis_.pop_front();
        }
    }

    // 8 bit mono
    bool _render_8_1(info_wave_t & info, uint32_t count) {
        const wave_t & wave = *(info.wave_);
        const uint64_t end = uint64_t(wave.num_frames()) * 0x10000llu;
        const int8_t * src = wave.get<int8_t>();
        // get mix down buffer pointers
        int32_t * ml = mix_.left_.data();
        int32_t * mr = mix_.right_.data();
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
        int32_t * ml = mix_.left_.data();
        int32_t * mr = mix_.right_.data();
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
        int32_t * ml = mix_.left_.data();
        int32_t * mr = mix_.right_.data();
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
            out[i*2+0] = clip(mix_.left_.data()[i]);
            out[i*2+1] = clip(mix_.right_.data()[i]);
        }
    }

    bool render(int16_t * out, uint32_t count) {
        check_messages();
        while (count) {
            // figure our how many samples to render
            const uint32_t samples = min2(count, C_BUFFER_SIZE);
            count -= samples;
            // clear our mix buffers
            mix_.left_.fill(0);
            mix_.right_.fill(0);
            // render out vorbis stream
            vorbis_.render(mix_.left_.data(), 
                           mix_.right_.data(), 
                           samples);
            // render out any pending wave samples
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
