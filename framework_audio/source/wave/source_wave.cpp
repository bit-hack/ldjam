#include "source_wave.h"

namespace tengu {
// 8 bit mono
bool audio_source_wave_t::_render_8_1(
    const mix_out_t& mix,
    info_wave_t& info)
{

    const wave_t& wave = *(info.wave_);
    const uint64_t end = uint64_t(wave.num_frames()) * 0x10000llu;
    const int8_t* src = wave.get<int8_t>();
    // get mix down buffer pointers
    int32_t* ml = mix.left_;
    int32_t* mr = mix.right_;
    size_t count = mix.count_;
    // repeat until all samples are done
    for (;;) {
        // render loop
        for (; count && info.frame_ < end; --count) {
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
bool audio_source_wave_t::_render_8_2(
    const mix_out_t& mix,
    info_wave_t& info)
{

    const wave_t& wave = *(info.wave_);
    return false;
}

// 16 bit mono
bool audio_source_wave_t::_render_16_1(
    const mix_out_t& mix,
    info_wave_t& info)
{

    const wave_t& wave = *(info.wave_);
    const uint64_t end = uint64_t(wave.num_frames()) * 0x10000llu;
    const int16_t* src = wave.get<int16_t>();
    // get mix down buffer pointers
    int32_t* ml = mix.left_;
    int32_t* mr = mix.right_;
    size_t count = mix.count_;
    // repeat until all samples are done
    for (;;) {
        // render loop
        for (; count && info.frame_ < end; --count) {
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
bool audio_source_wave_t::_render_16_2(
    const mix_out_t& mix,
    info_wave_t& info)
{

    const wave_t& wave = *(info.wave_);
    const uint64_t end = uint64_t(wave.num_frames()) * 0x10000llu;
    const int16_t* src = wave.get<int16_t>();
    // get mix down buffer pointers
    int32_t* ml = mix.left_;
    int32_t* mr = mix.right_;
    size_t count = mix.count_;
    // repeat until all samples are done
    for (;;) {
        // render loop
        for (; count && info.frame_ < end; --count) {
            // buffer index point
            const uint64_t index = info.frame_ / 0x10000;
            // index wave file
            const int16_t sample_l = (src[index * 2 + 0] * info.volume_) / 0x100;
            const int16_t sample_r = (src[index * 2 + 1] * info.volume_) / 0x100;
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

bool audio_source_wave_t::_render(
    const mix_out_t& mix,
    info_wave_t& wave)
{

    const wave_t* sample = wave.wave_;
    assert(sample);
    switch (sample->bit_depth()) {
    case (8 /* bit */):
        switch (sample->num_channels()) {
        case (1):
            return _render_8_1(mix, wave);
        case (2):
            return _render_8_2(mix, wave);
        default:
            assert(!"unsupported channel count");
        }
    case (16 /* bit */):
        switch (sample->num_channels()) {
        case (1):
            return _render_16_1(mix, wave);
        case (2):
            return _render_16_2(mix, wave);
        default:
            assert(!"unsupported channel count");
        }
    default:
        assert(!"unsupported bit depth");
    }
    return true;
}

void audio_source_wave_t::_check_messages()
{

    // pop a message from the queue
    play_wave_t msg;
    while (queue_.pop(msg)) {
        auto itt = waves_.find(msg.wave_);
        if (itt == waves_.end()) {
            if (msg.volume_ <= 0.f) {
                return;
            } else {
                info_wave_t info;
                info.wave_ = msg.wave_;
                info.volume_ = uint32_t(msg.volume_ * 0x100);
                info.delta_ = uint64_t(msg.rate_ * 0x10000);
                info.frame_ = 0;
                info.looping_ = msg.looping_;
                waves_[info.wave_] = info;
            }
        } else {
            info_wave_t& info = itt->second;
            if (msg.volume_ <= 0.f) {
                waves_.erase(itt);
            } else {
                info.volume_ = uint32_t(msg.volume_ * 0x100);
                info.delta_ = uint64_t(msg.rate_ * 0x10000);
                info.looping_ = msg.looping_;
                if (msg.retrigger_) {
                    info.frame_ = 0;
                }
            }
        }
    } // while
}

void audio_source_wave_t::render(
    const mix_out_t& mix)
{

    // check pending messages
    _check_messages();
    // render out any pending wave samples
    for (auto itt = waves_.begin(); itt != waves_.end();) {

        assert(itt->first == itt->second.wave_);

        if (!_render(mix, itt->second)) {
            itt = waves_.erase(itt);
        } else {
            ++itt;
        }
    }
}
} // namespace tengu
