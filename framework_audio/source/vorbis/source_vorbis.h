#pragma once
#include <array>

#include "../../audio.h"

// defined in stb_vorbis.h
struct stb_vorbis;

namespace tengu {
struct audio_source_vorbis_t : public audio_source_t {

    audio_source_vorbis_t();
    ~audio_source_vorbis_t();

    struct play_vorbis_t {
        const char* name_;
        const struct vorbis_t* vorbis_;
        bool loop_;
        int32_t volume_;
    };

    bool play(const play_vorbis_t& info)
    {
        queue_.push(info);
        return true;
    }

    virtual void render(
        const mix_out_t& mix);

protected:
    void _clear_buffer();
    void _enqueue();
    void _close();
    void _render(const mix_out_t& mix);

    static const size_t C_CHANNELS = 2;
    std::array<int16_t, 4096 * C_CHANNELS> buffer_;
    size_t head_;
    size_t tail_;
    bool finished_;
    bool loop_;
    struct ::stb_vorbis* stb_;
    int32_t volume_;

    queue_t<play_vorbis_t> queue_;
};
} // namespace tengu
