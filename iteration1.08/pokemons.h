#ifndef POKEMONS_H
#define POKEMONS_H
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
#include <vector>
#include "heap.h"
#include "poke327.h"
#include "character.h"
#include "io.h"
enum p_stats
{
    hp,
    attack,
    defense,
    special_attack,
    special_defense,
    speed
};
enum genders
{
    female,
    male,
    genderless
};

class Pokemons
{
private:
    int level;
    bool shiny;
    int IV[6];
    int gender;
    int hp;
    int otherStat;
    // int whatSpecies;
    int whatMove[4];
    int stats[6];

public:
    Pokemons();
    Pokemons(int level);
    // ~Pokemons();
    void levelUp();
    int init_level() const;
    int init_gender();
    void init_IV();
    const char *get_species() const;
    int get_hp() const;
    int get_attack() const;
    int get_defense() const;
    int get_secial_attack() const;
    int get_special_defense() const;
    int get_speed() const;
    const char *get_gender_toString() const;
    bool is_shiny() const;
    const char *get_move(int i) const;
};

#endif
