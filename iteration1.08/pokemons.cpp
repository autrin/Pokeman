#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include <memory>
#include <unordered_map>
#include <string>
#include <functional>
#include <iostream>

#include "heap.h"
#include "poke327.h"
#include "character.h"
#include "io.h"
#include "pokemons.h"

Pokemons::Pokemons() : level(0), shiny(0) {}
Pokemons::Pokemons(int level, bool shiny) : level(init_level()), shiny(is_shiny()) {}
bool is_shiny()
{
    return rand() % 8192 == 0;
}
void Pokemons::levelUp() const
{
    p
}
int Pokemons::init_level() const // *TEST this
{
    if (world.cur_idx[dim_x] - WORLD_SIZE / 2 + world.cur_idx[dim_y] - WORLD_SIZE / 2 <= 200)
    {
        return 1 + abs((world.cur_idx[dim_x] - WORLD_SIZE / 2) + abs(world.cur_idx[dim_y] - WORLD_SIZE / 2)) / 2;
    }
    else
    {
        return (world.cur_idx[dim_x] - WORLD_SIZE / 2 + world.cur_idx[dim_y] - WORLD_SIZE / 2 - 200) / 2;
    }
}
/*
TODO
gender
IV
leveling up
moves
encountering chance

*/
