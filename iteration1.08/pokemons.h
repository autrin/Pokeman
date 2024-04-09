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

public:
    Pokemons();
    Pokemons(int level, bool shiny);
    ~Pokemons();
    void levelUp() const;
    int init_level() const;
};

#endif