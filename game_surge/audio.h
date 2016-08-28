#pragma once

#include <array>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

enum sound_t {

    e_sound_fire,
    e_sound_bomb,
    e_sound_powerup_drop,
    e_sound_powerup_pickup,
    e_sound_powerup_miss,
    e_sound_explode_1,
    e_sound_explode_2,
    e_sound_explode_3,
    e_sound_explode_4,
    e_sound_wave_spawn,
    e_sound_armour,
    e_sound_boss_fire,
    e_sound_player_die,
    e_sound_boss_spawn,
    e_sound_boss_hurt,

    E_SOUND_COUNT__
};

struct audio_t {

    Mix_Music * music_;
    std::array<Mix_Chunk*, E_SOUND_COUNT__> chunk_;
    
    audio_t();

    bool init();

    void play_sound(sound_t sound);
};
