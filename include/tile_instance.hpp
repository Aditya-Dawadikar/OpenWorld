#pragma once
#include "tile_type.hpp"

struct TileInstance {
    TileType type;
    int gridX, gridY;
    int height;
};
