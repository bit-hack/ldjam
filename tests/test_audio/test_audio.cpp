#include <array>
#include <SDL/SDL.h>
#include "../../framework_core/random.h"
#include "../../framework_audio/audio.h"
#include "../../framework_audio/source_wave.h"
#include "../../framework_audio/source_vorbis.h"

namespace {

std::array<wave_t, 2> s_wave;
std::array<vorbis_t, 2> s_vorbis;

audio_source_vorbis_t s_source_vorbis;
audio_source_wave_t s_source_wave;
audio_t s_audio;

SDL_Surface * s_screen;
random_t s_rand(0x12345);

void callback(void *user, uint8_t *data, int size) {
    int16_t * stream = reinterpret_cast<int16_t*>(data);
    const uint32_t count = (size / sizeof(int16_t)) / 2;
    s_audio.render(stream, count);
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

bool play_sound(int num) {

    audio_source_wave_t::play_wave_t play;
    play.looping_ = false;
    play.rate_ = 1.f + s_rand.randfs() * .7f;
    play.retrigger_ = true;
    play.volume_ = s_rand.randfu();
    play.wave_ = &s_wave[num];
    s_source_wave.play(play);
    return true;
}

} // namespace {}

int main(const int argc, char *args[]) {

    s_audio.add(&s_source_wave);
    s_audio.add(&s_source_vorbis);

    if (!init()) {
        return 1;
    }

    if (!wave_t::load_wav(
            "assets/sound1.wav",
            s_wave[0])) {
        return 1;
    }

    if (!wave_t::load_wav(
            "assets/sound_boss_spawn.wav",
            s_wave[1])) {
        return 1;
    }

    // test ogg vorbis playback
    {
        s_vorbis[0].open("assets/may.ogg");
        if (s_vorbis[0].valid()) {
            audio_source_vorbis_t::play_vorbis_t play;
            play.name_ = "may";
            play.vorbis_ = &s_vorbis[0];
            play.loop_ = false;
            play.volume_ = 0x3f;
            s_source_vorbis.play(play);
        }
        s_vorbis[1].open("assets/poly.ogg");
        if (s_vorbis[1].valid()) {
            audio_source_vorbis_t::play_vorbis_t play;
            play.name_ = "poly";
            play.vorbis_ = &s_vorbis[1];
            play.loop_ = true;
            play.volume_ = 0x7f;
            s_source_vorbis.play(play);
        }
    }

    bool active = true;
    while (active) {

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                active = false;
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case (SDLK_1):
                    play_sound(0);
                    break;
                case (SDLK_2):
                    play_sound(1);
                    break;
                }
            }
        }

        SDL_Delay(10);
    }

    return 0;
}
