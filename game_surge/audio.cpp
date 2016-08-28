#include "audio.h"

audio_t::audio_t()
    : music_(nullptr)
{
    chunk_.fill(nullptr);
}

bool audio_t::init() {

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048)) {
        return false;
    }

    music_ = Mix_LoadMUS("music.ogg");
    if (music_) {
        Mix_PlayMusic(music_, -1);
    }

    chunk_[e_sound_fire]           = Mix_LoadWAV("sound_fire.wav");
    chunk_[e_sound_bomb]           = Mix_LoadWAV("sound_bomb.wav");
    chunk_[e_sound_powerup_drop]   = Mix_LoadWAV("sound_powerup_drop.wav");
    chunk_[e_sound_powerup_pickup] = Mix_LoadWAV("sound_powerup_pickup.wav");
    chunk_[e_sound_powerup_miss]   = Mix_LoadWAV("sound_powerup_miss.wav");
    chunk_[e_sound_explode_1]      = Mix_LoadWAV("sound_explode_1.wav");
    chunk_[e_sound_explode_2]      = Mix_LoadWAV("sound_explode_2.wav");
    chunk_[e_sound_explode_3]      = Mix_LoadWAV("sound_explode_3.wav");
    chunk_[e_sound_explode_4]      = Mix_LoadWAV("sound_explode_4.wav");
    chunk_[e_sound_wave_spawn]     = Mix_LoadWAV("sound_wave_spawn.wav");
    chunk_[e_sound_armour]         = Mix_LoadWAV("sound_armour.wav");
    chunk_[e_sound_boss_fire]      = Mix_LoadWAV("sound_boss_fire.wav");
    chunk_[e_sound_player_die]     = Mix_LoadWAV("sound_player_die.wav");
    chunk_[e_sound_boss_spawn]     = Mix_LoadWAV("sound_boss_spawn.wav");
    chunk_[e_sound_boss_hurt]      = Mix_LoadWAV("sound_boss_hurt.wav");

    return true;
}

void audio_t::play_sound(sound_t sound) {

    Mix_Chunk * chunk = chunk_[sound];
    if (chunk) {
        Mix_PlayChannel(-1, chunk, 0);
    }
}
