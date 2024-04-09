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
Pokemons::Pokemons(int level, int shiny) : level(level), shiny(shiny){}

void Pokemons::levelUp() const
{
    
}
int Pokemons::init_level() const // *TEST this
{
    if(world.cur_idx[dim_x] - WORLD_SIZE / 2 + world.cur_idx[dim_y] - WORLD_SIZE / 2 <= 200)
    {
        return 1 + abs((world.cur_idx[dim_x] - WORLD_SIZE / 2) + abs(world.cur_idx[dim_y] - WORLD_SIZE / 2)) / 2;
    }
    else
    {
        return (world.cur_idx[dim_x] - WORLD_SIZE / 2 + world.cur_idx[dim_y] - WORLD_SIZE / 2 - 200) / 2;
    }
}

