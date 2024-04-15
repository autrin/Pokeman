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
#include <cstdlib>
#include "heap.h"
#include "poke327.h"
#include "character.h"
#include "io.h"
#include "pokemons.h"

int whatSpecies = rand() % CsvFile::pokemon_species_data.size();
const pokemon_species &species = pokemon_species::get_species_by_id(whatSpecies);

Pokemons::Pokemons() : Pokemons(init_level()) {}
Pokemons::Pokemons(int level) : level(level), shiny(init_shiny()), gender(init_gender())
{
    levelUp();
}
bool init_shiny()
{
    return rand() % 8192 == 0;
}
void Pokemons::levelUp()
{
    // std::vector<std::string> *species;
    int moveLevel, speciesId, movePokemonId;
    bool stop;
    if (!species.getLevelUpMoveSet().size())
    {
        for (const auto &moveRow : CsvFile::pokemon_moves_data)
        {
            if (moveRow.empty())
                continue;

            movePokemonId = std::stoi(moveRow.at(2)); // move_id is at idx 2 in tokens

            // for (const auto &pokemonRow : CsvFile::pokemon_data)
            // {
            //     if (pokemonRow.empty())
            //         continue;

            //     speciesId = std::stoi(pokemonRow.at(2));

            if (movePokemonId == species.getId() && std::stoi(moveRow.at(3)) == 1)
            {
                int i;
                for (stop = false, i = 0; !stop && i < species.getLevelUpMoveSet().size(); i++)
                {
                    if (movePokemonId == species.getLevelUpMoveSet().at(i).second)
                    {
                        stop = true;
                    }
                }
                const_cast<pokemon_species &>(species).baseStats[0] = std::stoi(CsvFile::pokemon_stats_data.at(whatSpecies * 6 - 5).at(2));
                const_cast<pokemon_species &>(species).baseStats[1] = std::stoi(CsvFile::pokemon_stats_data.at(whatSpecies * 6 - 4).at(2));
                const_cast<pokemon_species &>(species).baseStats[2] = std::stoi(CsvFile::pokemon_stats_data.at(whatSpecies * 6 - 3).at(2));
                const_cast<pokemon_species &>(species).baseStats[3] = std::stoi(CsvFile::pokemon_stats_data.at(whatSpecies * 6 - 2).at(2));
                const_cast<pokemon_species &>(species).baseStats[4] = std::stoi(CsvFile::pokemon_stats_data.at(whatSpecies * 6 - 1).at(2));
                const_cast<pokemon_species &>(species).baseStats[5] = std::stoi(CsvFile::pokemon_stats_data.at(whatSpecies * 6).at(2));
            }
            // }
        }
    }
    // Add upto 2 moves
    // int count = 0;
    int i;
    for (i = 0;
         i < species.levelUp_move_set.size() && species.levelUp_move_set[i].first <= level;
         i++)
        ;
    whatMove[0] = whatMove[1] = whatMove[2] = whatMove[3] = 0;
    if (i)
    {

        whatMove[0] = species.levelUp_move_set[rand() % i].second;
        if (i != 1)
        {
            int j;
            do
            {
                j = rand() % i;
            } while (species.levelUp_move_set[j].second == whatMove[0]);
            whatMove[1] = species.levelUp_move_set[j].second;
        }
    }
    init_IV();
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

void Pokemons::init_IV()
{
    for (int i = 0; i < 6; i++)
    {                        // There are 6 species.
        IV[i] = rand() % 16; // is this 16 inclusive?
        if (i != 0)
        {
            stats[i] = ((species.baseStats[i] + IV[i]) * 2 * level) / 100 + 5;
        }
        else
        {
            stats[i] = ((species.baseStats[i] + IV[i]) * 2 * level) / 100 + level + 10;
        }
    }
}

const char *Pokemons::get_species() const
{
    return const_cast<pokemon_species &>(species).getIdentifier().c_str();
}

int Pokemons::get_hp() const
{
    return stats[hp];
}

int Pokemons::get_attack() const
{
    return stats[attack];
}

int Pokemons::get_defense() const
{
    return stats[defense];
}

int Pokemons::get_secial_attack() const
{
    return stats[special_attack];
}

int Pokemons::get_special_defense() const
{
    return stats[special_defense];
}

int Pokemons::get_speed() const
{
    return stats[speed];
}

const char *Pokemons::get_gender_toString() const
{
    return gender == female ? "female" : "male";
}

bool Pokemons::is_shiny() const
{
    return shiny;
}

const char *Pokemons::get_move(int i) const
{
    if (i < 4 && whatMove[i])
    {
        return CsvFile::moves_data.at(whatMove[i]).at(1).c_str();
    }
    else
    {
        return "";
    }
}
