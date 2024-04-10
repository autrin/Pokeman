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

class Pokemons
{
private:
    int level;
    bool shiny;
    int IV;
    int gender;
    std::vector<std::pair<int, int>> levelUp_set;
    std::vector<int> moves;

public:
    Pokemons();
    Pokemons(int level, bool shiny, int gender, int IV);
    ~Pokemons();
    void levelUp();
    int init_level() const;
    int init_gender();
    int init_IV();
};

#endif