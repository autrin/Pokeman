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
Pokemons::Pokemons(int level, bool shiny, int gender, int IV) : level(init_level()), shiny(init_shiny()), gender(init_gender()), IV(init_IV()) {}
bool init_shiny()
{
    return rand() % 8192 == 0;
}
void Pokemons::levelUp()
{
    int moveLevel, speciesId, movePokemonId;

    for (const auto &moveRow : CsvFile::pokemon_moves_data)
    {
        if (moveRow.empty())
            continue;

        movePokemonId = std::stoi(moveRow.at(2)); // move_id is at idx 2 in tokens

        for (const auto &pokemonRow : CsvFile::pokemon_data)
        {
            if (pokemonRow.empty())
                continue;

            speciesId = std::stoi(pokemonRow.at(2));

            if (movePokemonId == speciesId && std::stoi(moveRow.at(3)) == 1)
            {
                moveLevel = std::stoi(moveRow.at(4));
                // now add them to the set
                levelUp_set.push_back(std::make_pair(moveLevel, movePokemonId));

                // std::cout << "Match found for Pokemon ID " << speciesId << ": " << std::endl;
                // std::cout << "Pokemon Data: ";
                // std::cout << std::endl;
                // std::cout << "\n\n";
            }
        }
    }
    // Add upto 2 moves
    int count = 0;
    for (const auto &row : levelUp_set)
    {
        if (row.first <= level && count <= 2)
        {
            moves.push_back(row.first);
            count++;
        }
    }
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
int Pokemons::init_gender()
{
    return rand() % 2 == 0 ? female : male;
}

int Pokemons::init_IV()
{
    return 0;
}
/*
TODO
gender DONE
IV
leveling up DONE
moves DONE
encountering chance

*/
