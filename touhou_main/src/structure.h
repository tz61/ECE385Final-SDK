#ifndef STRUCTURE_H
#define STRUCTURE_H
#include <stdint.h>
typedef struct {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
} sprite_t;
sprite_t get_enemy_bullet_info(int type);
sprite_t get_player_bullet_info(int type);
sprite_t get_enemy_info(int type);
sprite_t get_player_info();
sprite_t get_boss_info();
sprite_t get_item_info(int type);
#endif