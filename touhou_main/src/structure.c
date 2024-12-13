#include "structure.h"
#include "helper.h"

sprite_t get_enemy_bullet_info(int type) {
    sprite_t ret;
    switch (type) {
    case 0:
        ret.x = 0;
        ret.y = 0;
        ret.width = 16;
        ret.height = 16;
        break;
    case 1:
        ret.x = 16;
        ret.y = 0;
        ret.width = 16;
        ret.height = 16;
        break;
    case 2:
        ret.x = 32;
        ret.y = 0;
        ret.width = 16;
        ret.height = 16;
        break;
    case 3:
        ret.x = 48;
        ret.y = 0;
        ret.width = 8;
        ret.height = 8;
        break;
    case 4:
        ret.x = 48;
        ret.y = 8;
        ret.width = 8;
        ret.height = 8;
        break;
    case 5:
        ret.x = 56;
        ret.y = 8;
        ret.width = 8;
        ret.height = 8;
        break;
    case 6:
        ret.x = 0;
        ret.y = 16;
        ret.width = 16;
        ret.height = 16;
        break;
    case 7:
        ret.x = 16;
        ret.y = 16;
        ret.width = 16;
        ret.height = 16;
        break;
    case 8:
        ret.x = 32;
        ret.y = 16;
        ret.width = 16;
        ret.height = 16;
        break;
    case 9:
        ret.x = 48;
        ret.y = 16;
        ret.width = 16;
        ret.height = 16;
        break;
    case 10:
        ret.x = 0;
        ret.y = 32;
        ret.width = 16;
        ret.height = 16;
        break;
    case 11:
        ret.x = 16;
        ret.y = 32;
        ret.width = 16;
        ret.height = 16;
        break;
    case 12:
        ret.x = 32;
        ret.y = 32;
        ret.width = 16;
        ret.height = 16;
        break;
    case 13:
        ret.x = 48;
        ret.y = 32;
        ret.width = 16;
        ret.height = 16;
        break;
    case 14:
        ret.x = 64;
        ret.y = 32;
        ret.width = 32;
        ret.height = 32;
        break;
    case 15:
        ret.x = 96;
        ret.y = 32;
        ret.width = 32;
        ret.height = 32;
        break;
    default: // case 0
        ret.x = 0;
        ret.y = 0;
        ret.width = 16;
        ret.height = 16;
        break;
    }
    return ret;
}
sprite_t get_player_bullet_info(int type) {
    sprite_t ret;
    switch (type) {
    case 0:
        ret.x = 32;
        ret.y = 48;
        ret.width = 16;
        ret.height = 16;
        break;
    case 1:
        ret.x = 48;
        ret.y = 48;
        ret.width = 16;
        ret.height = 16;
        break;
    case 2:
        ret.x = 64;
        ret.y = 96;
        ret.width = 64;
        ret.height = 12;
        break;
    case 3:
        ret.x = 64;
        ret.y = 108;
        ret.width = 64;
        ret.height = 20;
        break;
    default: // case 0
        ret.x = 32;
        ret.y = 48;
        ret.width = 16;
        ret.height = 16;
        break;
    }
    return ret;
}
sprite_t get_enemy_info(int type) {
    sprite_t ret;
    switch (type) {
    case 0:
        ret.x = 64;
        ret.y = 64;
        ret.width = 32;
        ret.height = 32;
        break;
    case 1:
        ret.x = 96;
        ret.y = 64;
        ret.width = 32;
        ret.height = 32;
        break;
    case 2:
        ret.x = 0;
        ret.y = 64;
        ret.width = 64;
        ret.height = 64;
        break;
    default: // case 0
        ret.x = 64;
        ret.y = 64;
        ret.width = 32;
        ret.height = 32;
        break;
    }
    return ret;
}
sprite_t get_player_info() {
    sprite_t ret;
    ret.x = 128;
    ret.y = 0;
    ret.width = 32;
    ret.height = 48;
    return ret;
}
sprite_t get_boss_info() {
    sprite_t ret;
    ret.x = 160;
    ret.y = 0;
    ret.width = 96;
    ret.height = 128;
    return ret;
}
sprite_t get_item_info(int type) {
    sprite_t ret;
    switch (type) {
    case 0:
        ret.x = 0;
        ret.y = 48;
        ret.width = 16;
        ret.height = 16;
        break;
    case 1:
        ret.x = 16;
        ret.y = 48;
        ret.width = 16;
        ret.height = 16;
        break;
    case 2:
        ret.x = 64;
        ret.y = 0;
        ret.width = 32;
        ret.height = 32;
        break;
    case 3:
        ret.x = 96;
        ret.y = 0;
        ret.width = 32;
        ret.height = 32;
        break;
    case 4:
        ret.x = 128;
        ret.y = 64;
        ret.width = 32;
        ret.height = 32;
        break;
    case 5:
        ret.x = 128;
        ret.y = 96;
        ret.width = 32;
        ret.height = 32;
        break;
    default: // case 0
        ret.x = 0;
        ret.y = 48;
        ret.width = 16;
        ret.height = 16;
        break;
    }
    return ret;
}