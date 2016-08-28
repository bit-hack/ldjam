#include "draw.h"

extern
std::array<SDL_Rect, E_SPRITE_COUNT__> s_sprite_offset = {
    SDL_Rect{00, 00, 24, 24},
    SDL_Rect{24, 00, 24, 24},
    SDL_Rect{48, 00, 24, 24},
    SDL_Rect{72, 00,  3,  9},
    SDL_Rect{96, 00,  3, 10},
    
    SDL_Rect{00, 24,  7, 10},
    SDL_Rect{24, 24,  7, 11},
    SDL_Rect{48, 24,  8,  8},
    SDL_Rect{72, 24,  9,  9},
    SDL_Rect{96, 24, 18, 18},
    
    SDL_Rect{00, 48, 24, 24},
    SDL_Rect{24, 48, 19, 19},
    SDL_Rect{48, 48, 13, 13},
    SDL_Rect{72, 48,  8,  8},
    SDL_Rect{96, 48,  5,  5},
};
