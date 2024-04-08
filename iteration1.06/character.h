#ifndef CHARACTER_H
#define CHARACTER_H
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <sys/time.h>
#include <limits.h> 
#include <math.h>
#include <unistd.h>
#include <curses.h>
#include "heap.h"
#include "map.h"
                           
// Define character types
typedef enum __attribute__((__packed__)) CharacterType {
    PC,
        Hiker,
        Rival,
        Swimmer,
        Other,
        Num_characterypes
} CharacterType;
// int16_t pair_t[3];
class Position
{
public:
    int32_t x, y;
};
class character : public Position {
public:
    uint32_t next_turn;
    uint32_t sequence_number;
    CharacterType type;
    char symbol;
    heap_node_t* heap_node;
    uint32_t direction;
    int lost;
    int changedMove;
};


// Define terrain types
typedef enum __attribute__((__packed__)) TerrainType {
    BOULDER, TREE, PATH, MART, CENTER, GRASS, CLEARING, MOUNTAIN, FOREST, WATER, GATE,
        Num_Terrain_Types // Keeps track of the number of terrain types
} TerrainType;
// extern map;
// Store the cost of each character going through every terrain type 
int32_t cost[Num_characterypes][Num_Terrain_Types] = {
    // BOULDER, TREE, PATH, MART, CENTER, GRASS, CLEARING, MOUNTAIN, FOREST, WATER, GATE
    [PC] = {SHRT_MAX, SHRT_MAX, 10, 10, 10, 20, 10, SHRT_MAX, SHRT_MAX, SHRT_MAX, 10},
    [Hiker] = {SHRT_MAX, SHRT_MAX, 10, 50, 50, 15, 10, 15, 15, SHRT_MAX, SHRT_MAX},
    [Rival] = {SHRT_MAX, SHRT_MAX, 10, 50, 50, 20, 10, SHRT_MAX, SHRT_MAX, SHRT_MAX, SHRT_MAX},
    [Swimmer] = {SHRT_MAX, SHRT_MAX, SHRT_MAX, SHRT_MAX, SHRT_MAX, SHRT_MAX, SHRT_MAX, SHRT_MAX, SHRT_MAX, 7, SHRT_MAX},
    [Other] = {SHRT_MAX, SHRT_MAX, 10, 50 , 50 , 20, 10 , SHRT_MAX, SHRT_MAX, SHRT_MAX, SHRT_MAX,}
};
// int32_t get_cost(char terrainChar, int x, int y, CharacterType character);
// int32_t characters_turn_comp(const void* key, const void* with);
// character* create_character(Position pos, CharacterType type, char symbol);
// bool is_position_valid_for_npc(int32_t x, int32_t y, CharacterType npcype);
// Position find_valid_position_for_npc(CharacterType npcype);
// void update_character_turn(character* character);
// void generate_npcs(int numtrainers, class map* map);
// void move_character(character* character, int direction_x, int direction_y, map* m, Position* pos);
// void move_pacer(character* npc);
// void move_wanderer(character* npc);
// void move_towards_player_hiker(character* npc);
// void move_towards_player_rival(character* npc);
// void move_explorer(character* npc);
// void move_swimmer(character* npc);
// void move_npc(character* npc);
// void addValidPosition(int x, int y);
// void collectValidPositions(map* m);
// character* create_pc(map* m);
// int32_t compRival(const void* key, const void* with) ;
//  int32_t compHiker(const void* key, const void* with);
// void dijkstra(map* m);
// void cleanup_characters(void* v);
// void enter_pokemart();
// void enter_pokemon_center();
// const char* characterype_to_string(CharacterType type);
//  void display_trainers(character** c, int count);
// void display(void);
// void move_pc_func(character* character, map* m, Position* pos);
//  void list_trainers(void);
// void fly(Position* pos);
// void battle(character* character) ;
// uint32_t move_pc(uint32_t input, Position* pos);
// void get_input(Position* pos);
// void init_io(void);



#endif