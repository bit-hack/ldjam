#include "stb_vorbis.h"
#include "vorbis.h"
#include "source_vorbis.h"

audio_source_vorbis_t::audio_source_vorbis_t()
    : buffer_()
    , head_(0)
    , tail_(0)
    , finished_(true)
    , loop_(false)
    , stb_(nullptr)
    , queue_()
    , volume_(0xff)
{
}

audio_source_vorbis_t::~audio_source_vorbis_t() {
    _close();
}

void audio_source_vorbis_t::_render(const mix_out_t & mix) {

    assert(stb_);
    size_t count = mix.count_ * 2;
    int32_t * left = mix.left_;
    int32_t * right = mix.right_;
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
            *(left++) += (int32_t(buffer_[i+0]) * volume_) >> 8;
            *(right++) += (int32_t(buffer_[i+1]) * volume_) >> 8;
        }
        tail_ = notch;
    }
}

void audio_source_vorbis_t::_close() {
    // close decode stream
    if (stb_) {
        stb_vorbis_close(stb_);
        stb_ = nullptr;
    }
    // clear decode buffer
    _clear_buffer();
}

void audio_source_vorbis_t::_enqueue() {
    if (stb_) {
        stb_vorbis_close(stb_);
        stb_ = nullptr;
    }
    play_vorbis_t v;
    if (queue_.pop(v)) {
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
            volume_ = v.volume_;
            _clear_buffer();
        }
    }
}

void audio_source_vorbis_t::_clear_buffer() {
    head_ = tail_ = 0;
}

void audio_source_vorbis_t::render(
        const mix_out_t & mix) {

    if (!stb_ || finished_) {
        _enqueue();
    }
    if (stb_) {
        _render(mix);
    }
}
