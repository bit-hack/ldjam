#include <array>
#include <cstdint>
#include <cstdio>
#include <memory>

#include <SDL/SDL.h>

#include "../../framework_core/file.h"
#include "../../framework_core/common.h"
#include "../../framework_audio/source/vorbis/stb_vorbis.h"

namespace {
SDL_Surface * s_screen = nullptr;

struct vorbis_1_t {

    vorbis_1_t() {
        memset(&data_, 0, sizeof(data_));
    }

    ~vorbis_1_t() {
        close();
    }

    bool open(const char * path) {
        data_.num_samples_ = stb_vorbis_decode_filename(
            path, &data_.channels_, &data_.sample_rate_, &data_.samples_);
        return data_.num_samples_>0;
    }

    void close() {
        if (data_.samples_) {
            free(data_.samples_);
            data_.samples_ = nullptr;
        }
        memset(&data_, 0, sizeof(data_));
    }

    void render(int16_t * stream, int32_t count) {
        for (int i=0; i<count; ++i) {
            stream[i] = data_.samples_[(data_.index_+i) % data_.num_samples_];
        }
        data_.index_ += count;
    }

    struct {
        int32_t channels_;
        int32_t sample_rate_;
        int16_t * samples_;
        int32_t num_samples_;
        int32_t index_;
    } data_;
};

struct vorbis_2_t {

    vorbis_2_t() 
        : stb_(nullptr)
        , raw_()
        , raw_size_(0)
        , head_(0)
        , tail_(0)
    {
    }

    ~vorbis_2_t() {
        close();
    }

    bool open(const char * path) {
        file_reader_t file;
        if (!file.open(path)) {
            return false;
        }
        if (!file.size(raw_size_)) {
            return false;
        }
        raw_.reset(new uint8_t[raw_size_]);
        if (!file.read(raw_.get(), raw_size_)) {
            return false;
        }
        int error = 0;
        stb_ = stb_vorbis_open_memory(
            raw_.get(), 
            int(raw_size_),
            &error, 
            nullptr);
        if (!stb_) {
            close();
            return false;
        }
        // reset the buffer
        head_ = tail_ = 0;
        finished_ = false;
        loop_ = true;
        return true;
    }

    void close() {
        raw_.reset();
        raw_size_ = 0;
        if (stb_) {
            stb_vorbis_close(stb_);
            stb_ = nullptr;
        }
        head_ = tail_ = 0;
        finished_ = true;
    }

    void render(int16_t * stream, int32_t count) {
        using namespace tengu;

        const int _CHANNELS = 2;

        while (!finished_ && count) {
            // if the buffer is empty
            if (head_ == tail_) {
                // decode more data into the buffer
                head_ = tail_ = 0;
                head_ = stb_vorbis_get_frame_short_interleaved(
                    stb_, 
                    _CHANNELS,
                    buffer_.data(),
                    int(buffer_.size()))*_CHANNELS;
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
            // find max number of samples we can write
            int32_t notch = minv(tail_+count, head_);
            const int32_t c_num = notch-tail_;
            count -= c_num;
            // render out these samples
#if 1
            memcpy(stream, buffer_.data()+tail_, (notch-tail_)*sizeof(int16_t));
            stream += c_num;
#else
            for (int32_t i = tail_; i<notch; ++i) {
                *(stream++) = buffer_[i];
            }
#endif
            tail_ = notch;
        }
    }

    std::array<int16_t, 4096*2> buffer_;
    int32_t head_;
    int32_t tail_;

    stb_vorbis * stb_;
    std::unique_ptr<uint8_t[]> raw_;
    size_t raw_size_;

    bool finished_;
    bool loop_;
};

vorbis_2_t s_vorbis;

void callback(void *user, uint8_t *data, int size) {
    int16_t * stream = reinterpret_cast<int16_t*>(data);
    const uint32_t count = (size / sizeof(int16_t));

    s_vorbis.render(stream, count);
}

bool init() {

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        return false;
    }

    s_screen = SDL_SetVideoMode(320, 240, 32, 0);
    if (!s_screen) {
        return false;
    }

    SDL_AudioSpec spec, desired;
    memset(&spec, 0, sizeof(desired));
    desired.size = sizeof(desired);
    desired.channels = 2;
    desired.freq = 44100;
    desired.samples = 1024 * 4;
    desired.format = AUDIO_S16SYS;
    desired.callback = callback;

    if (SDL_OpenAudio(&desired, &spec) < 0) {
        return false;
    }

    SDL_PauseAudio(SDL_FALSE);
    return true;
}

} // namespace {}

int main(const int argc, char *args[]) {

    if (!s_vorbis.open("assets/may.ogg")) {
        return 1;
    }

    if (!init()) {
        return 1;
    }

    bool active = true;
    while (active) {

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                active = false;
            }
        }

        SDL_Delay(10);
    }

    return 0;
}
