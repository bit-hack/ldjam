#include <SDL/SDL.h>
#include "../../framework_audio/audio.h"

namespace {

audio_t s_audio;

void callback(void *user, uint8_t *data, int size) {
    int16_t * stream = reinterpret_cast<int16_t*>(data);
    const uint32_t count = (size / sizeof(int16_t)) / 2;
    s_audio.render(stream, count);
}

bool init() {

    if (SDL_Init(SDL_INIT_AUDIO)) {
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

    if (!init()) {
        return 1;
    }

    wave_t wave;
    if (!wave_t::load_wav(
            "/home/flipper/repos/ldjam/tests/assets/sound1.wav",
            wave)) {
        return 1;
    }

    audio_t::play_wave_t play;
    play.looping_ = false;
    play.rate_ = 0.9f;
    play.retrigger_ = true;
    play.volume_ = 1.f;
    play.wave_ = &wave;

    s_audio.play(play);

    bool active = true;
    while (active) {

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                active = false;
            }
        }
    }

    return 0;
}
