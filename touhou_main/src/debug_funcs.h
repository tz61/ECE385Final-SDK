#ifndef DEBUG_FUNCS
#define DEBUG_FUNCS
#include "helper.h"
#include "structure.h"
typedef enum { ENEMY_BULLET = 0, PLAYER_BULLET = 1, ENEMY = 2, PLAYER = 3, BOSS = 4, ITEM = 5, NUM_TYPES = 6 } type_object;
void test_keys();
void draw_board_strips(void *fb_ptr);
void draw_board_color(void *fb_ptr, uint32_t color);
void soft_draw_board_sprite(void *fb_ptr, type_object category, int type, int dest_x, int dest_y);

#endif
