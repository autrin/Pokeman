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

#define MAP_WIDTH 80     // width of the map
#define MAP_HEIGHT 21    // height of the map
#define NUM_REGIONS 5    // Number of regions
#define WORLD_HEIGHT 401 // world of all of the maps
#define WORLD_WIDTH 401
#define SHRT_MAX __SHRT_MAX__
#define mapxy(x, y) (m->m[y][x])
int quit = 0;
void newMapCaller(void);
void display(void);
void get_input(void);
char symbols[] = { '%', '^', ':', '.', '~' }; // Simplified symbols array

typedef struct path
{
    heap_node_t* hn;
    uint8_t pos[2];
    uint8_t from[2];
    int32_t cost;
} path_t;

// Define a structure to represent a region
struct Region
{
    int32_t fromX, fromY, toX, toY;
    char symbol;
};

// Global or struct to store valid positions
typedef struct
{
    int x, y;
} Position;

Position validPositionsForBuildings[MAP_WIDTH * MAP_HEIGHT]; // Adjust size accordingly
int validPositionsCount = 0;                                 // Keep track of how many valid positions are stored

typedef int16_t pair_t[3];

typedef struct map
{
    char m[MAP_HEIGHT][MAP_WIDTH];
    int topExit, bottomExit, leftExit, rightExit;
} map_t;

typedef struct pc
{ // player character, which is '@'
    int32_t x;
    int32_t y;
    heap_node_t* heap_node;
} pc_t;

// Define character types
typedef enum {
    PC,
    Hiker,
    Rival,
    Swimmer,
    Other,
    Num_Character_Types // Keeps track of the number of character types
} CharacterType;

// direction vectors: N, NE, E, SE, S, SW, W, NW
static Position directions[8] = { {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1} };

typedef struct character {
    int32_t x, y; // Position
    uint32_t next_turn;
    uint32_t sequence_number;
    CharacterType type;
    char symbol;
    heap_node_t* heap_node;
    uint32_t direction;
    int lost;
    int changedMove;
} character_t;

typedef struct world
{
    map_t* w[WORLD_HEIGHT][WORLD_WIDTH];
    int32_t curX; // x of the current map
    int32_t curY; // y of the current map
    character_t pc;
    int hikerDist[MAP_HEIGHT][MAP_WIDTH];
    int rivalDist[MAP_HEIGHT][MAP_WIDTH];
    // character_t* npcs[MAP_HEIGHT * MAP_WIDTH];
    character_t* npcs[MAP_HEIGHT][MAP_WIDTH];
    int npc_count;
    uint32_t global_sequence_number;

} world_t;

world_t world;

// Define terrain types
typedef enum {
    BOULDER, TREE, PATH, MART, CENTER, GRASS, CLEARING, MOUNTAIN, FOREST, WATER, GATE,
    Num_Terrain_Types // Keeps track of the number of terrain types
} TerrainType;

// Store the cost of each character going through every terrain type 
int32_t cost[Num_Character_Types][Num_Terrain_Types] = {
    // BOULDER, TREE, PATH, MART, CENTER, GRASS, CLEARING, MOUNTAIN, FOREST, WATER, GATE
    [PC] = {SHRT_MAX, SHRT_MAX, 10, 10, 10, 20, 10, SHRT_MAX, SHRT_MAX, SHRT_MAX, 10},
    [Hiker] = {SHRT_MAX, SHRT_MAX, 10, 50, 50, 15, 10, 15, 15, SHRT_MAX, SHRT_MAX},
    [Rival] = {SHRT_MAX, SHRT_MAX, 10, 50, 50, 20, 10, SHRT_MAX, SHRT_MAX, SHRT_MAX, SHRT_MAX},
    [Swimmer] = {SHRT_MAX, SHRT_MAX, SHRT_MAX, SHRT_MAX, SHRT_MAX, SHRT_MAX, SHRT_MAX, SHRT_MAX, SHRT_MAX, 7, SHRT_MAX},
    [Other] = {SHRT_MAX, SHRT_MAX, 10, 50 , 50 , 20, 10 , SHRT_MAX, SHRT_MAX, SHRT_MAX, SHRT_MAX,}
};

int32_t get_cost(char terrainChar, int x, int y, CharacterType character) {
    TerrainType terrain;
    switch (terrainChar) {
    case '%':
        terrain = BOULDER;
        break;
    case '^':
        terrain = TREE;
        break;
    case '#':
        if ((x == world.w[world.curY][world.curX]->topExit && y == 0) ||
            (x == world.w[world.curY][world.curX]->bottomExit && y == MAP_HEIGHT - 1) ||
            (y == world.w[world.curY][world.curX]->leftExit && x == 0) ||
            (y == world.w[world.curY][world.curX]->rightExit && x == MAP_WIDTH - 1)) {
            terrain = GATE;
        }
        else {
            terrain = PATH;
        }
        break;
    case 'M':
        terrain = MART;
        break;
    case ':':
        terrain = GRASS;
        break;
    case '.':
        terrain = CLEARING;
        break;
    case '~':
        terrain = WATER;
        break;
    case 'C':
        terrain = CENTER;
        break;
    case '@':
        return 10;
        // default:
        //     // terrain = Other;
        //     printf("Error in get_cost(): entered default.");
    }
    return cost[character][terrain];
}

heap_t event_heap;

int32_t characters_turn_comp(const void* key, const void* with)
{
    return ((((character_t*)key)->next_turn == ((character_t*)with)->next_turn) ? (((character_t*)key)->sequence_number - ((character_t*)with)->sequence_number)
        : (((character_t*)key)->next_turn - ((character_t*)with)->next_turn)); // If there is a tie, compare the sequence numbers.
}
character_t* create_character(Position pos, CharacterType type, char symbol) {
    character_t* new_char = malloc(sizeof(character_t));
    if (!new_char) {
        // Handle malloc failure
        return NULL;
    }
    new_char->x = pos.x;
    new_char->y = pos.y;
    new_char->next_turn = 0; // Initialize to 0 for all characters
    new_char->sequence_number = world.global_sequence_number++;
    new_char->type = type;
    new_char->symbol = symbol;
    new_char->heap_node = NULL; // Initialize heap_node to NULL
    new_char->lost = 0;
    new_char->changedMove = 0;
    return new_char;
}

bool is_position_valid_for_npc(int32_t x, int32_t y, CharacterType npc_type) {
    // Check bounds first to ensure we're looking within the map's dimensions
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) { // Might need this for PC but npcs check this in their move functions
        return false; // Position is out of bounds           // this function is only for npc so delete this?
    }

    // Check if the position is an exit (assuming exits are marked with '#')
    if (world.w[world.curY][world.curX]->m[y][x] == '#' && ((x == world.w[world.curY][world.curX]->topExit && y == 0) ||
        (x == world.w[world.curY][world.curX]->bottomExit && y == MAP_HEIGHT - 1) ||
        (y == world.w[world.curY][world.curX]->leftExit && x == 0) ||
        (y == world.w[world.curY][world.curX]->rightExit && x == MAP_WIDTH - 1))) { //* PC can go through gates for later iterations
        return false; // NPCs should avoid exits
    }
    // Check for the presence of another NPC or the PC at the position
    if (world.npcs[y][x] || (x == world.pc.x && y == world.pc.y)) {
        return false; // Position is already occupied by another NPC or the PC
    }

    // Specific terrain checks based on NPC type
    char terrain = world.w[world.curY][world.curX]->m[y][x];
    switch (npc_type) {
    case Swimmer:
        // Swimmers can only move in water
        if (terrain != '~') {
            return false;
        }
        break;
    case Hiker:
    case Rival:
        // Hikers and Rivals cannot move through boulders or water
        if (terrain == '%' || terrain == '~' || terrain == '^') {
            return false;
        }
        break;
    default:
        // Other types have specific restrictions or may be more flexible
        if (terrain == '%' || terrain == '^' || terrain == '~') {
            return false;
        }
        break;
    }

    return true; // If none of the checks failed, the position is valid for this NPC
}
Position find_valid_position_for_npc(CharacterType npc_type) {
    Position pos;
    do {
        pos.x = rand() % MAP_WIDTH;
        pos.y = rand() % MAP_HEIGHT;
    } while (!is_position_valid_for_npc(pos.x, pos.y, npc_type));
    return pos;
}
// Update character's next turn and reinsert into the event heap
void update_character_turn(character_t* character) {
    if (character->heap_node) {
        heap_remove_min(&event_heap);
    }
    character->heap_node = heap_insert(&event_heap, character);
}
void generate_npcs(int numtrainers, map_t* map) {
    for (int i = 0; i < numtrainers; i++) {
        // Adjusted to avoid selecting PC and Num_Character_Types as NPC types
        CharacterType npc_type = 1 + rand() % (Num_Character_Types - 1); // Randomly select NPC type, skipping PC and Num_Character_Types

        Position pos = find_valid_position_for_npc(npc_type);

        char symbol = 'N'; // Default symbol
        switch (npc_type) {
        case Hiker: symbol = 'h'; break;
        case Rival: symbol = 'r'; break;
        case Swimmer: symbol = 'm'; break;
        case Other: {
            int subtype = rand() % 4; // Differentiating sub-types of 'Other'
            switch (subtype) {
            case 0: symbol = 'p'; break; // Pacers
            case 1: symbol = 'w'; break; // Wanderers
            case 2: symbol = 'e'; break; // Explorers
            case 3: symbol = 's'; break; // Sentries
            }
        } break;
            // Explicitly ignoring PC and Num_Character_Types since they should not be selected
        case PC:
        case Num_Character_Types:
            continue; // Skip the rest of the loop iteration if these values are somehow selected
        }
        character_t* npc = create_character(pos, npc_type, symbol);
        if (npc) {
            npc->heap_node = heap_insert(&event_heap, npc);
            world.npcs[pos.y][pos.x] = npc;
            world.npc_count++;
        }
    }
}

void move_character(character_t* character, int direction_x, int direction_y, map_t* m) {
    int old_x = character->x;
    int old_y = character->y;
    int32_t movement_cost = get_cost(m->m[direction_y][direction_x], direction_x, direction_y, character->type);
    // Update the character's next turn based on movement cost
    character->next_turn += movement_cost;

    // Move the character to the new position
    if (character->type != PC) {
        character->x = direction_x;
        character->y = direction_y;
    }
    // Empty the npc map's old location
    world.npcs[old_y][old_x] = NULL;
    // Update the npc map's new location
    world.npcs[direction_y][direction_x] = character;
}

void move_pacer(character_t* npc) {
    // Assuming npc->direction stores the pacer's current direction (e.g., 0 for horizontal, 1 for vertical)
    int dx = (npc->direction == 0) ? 1 : 0; // Move horizontally if direction is 0
    int dy = (npc->direction == 1) ? 1 : 0; // Move vertically if direction is 1

    // Check if the next position in the current direction is valid
    if (npc->x + dx > 0 && npc->x + dx < MAP_WIDTH - 1 && npc->y + dy > 0 && npc->y + dy < MAP_HEIGHT - 1 && is_position_valid_for_npc(npc->x + dx, npc->y + dy, npc->type)) {
        move_character(npc, npc->x + dx, npc->y + dy, world.w[world.curY][world.curX]);
    }
    else {
        // If blocked, reverse direction
        npc->direction = (npc->direction + 1) % 2; // Toggle direction
        dx = (npc->direction == 0) ? 1 : 0;
        dy = (npc->direction == 1) ? 1 : 0;
        move_character(npc, npc->x - dx, npc->y - dy, world.w[world.curY][world.curX]); // Move in the opposite direction
    }
}

void move_wanderer(character_t* npc) {
    bool moved = false;
    int attempts = 0;
    while (!moved && attempts < 8) {
        // Example for a randomly moving NPC (like a wanderer)
        int dir_index = rand() % 8; // Choose a random direction
        int new_x = npc->x + directions[dir_index].x;
        int new_y = npc->y + directions[dir_index].y;
        if (new_x > 0 && new_x < MAP_WIDTH - 1 && new_y > 0 && new_y < MAP_HEIGHT - 1 && is_position_valid_for_npc(new_x, new_y, npc->type)) {
            move_character(npc, new_x, new_y, world.w[world.curY][world.curX]);
            moved = true;
        }
        else {
            attempts++;
        }
    }
    // If after 8 attempts no valid move is found, NPC stays in place (or handle differently)
    if (!moved) {
        // If unable to move after 8 attempts, the NPC stays in place.
        // Update npc->next_turn based on the cost of the terrain they are staying on.
        char current_terrain = world.w[world.curY][world.curX]->m[npc->y][npc->x];
        int32_t stay_cost = get_cost(current_terrain, npc->x, npc->y, npc->type);
        npc->next_turn += stay_cost; // Adjust next_turn based on the terrain cost

        // update_character_turn(npc);//!
    }
}

void move_towards_player_hiker(character_t* npc) {
    int bestDist = SHRT_MAX; // Initialize with maximum possible distance
    Position bestMove = { 0, 0 };
    int base = rand() & 0x7;
    int newX;
    int newY;
    // Iterate over all possible directions, including diagonals
    for (int i = base; i < 8 + base; i++) {
        newX = npc->x + directions[i & 0x7].x;
        newY = npc->y + directions[i & 0x7].y;
        // Ensure the new position is within bounds and valid
        if (newX > 0 && newX < MAP_WIDTH - 1 && newY > 0 && newY < MAP_HEIGHT - 1 && is_position_valid_for_npc(newX, newY, npc->type)) {
            int dist = world.hikerDist[newY][newX]; // Distance to the player from the new position
            if (dist < bestDist) {
                bestDist = dist;
                bestMove.x = newX;
                bestMove.y = newY;
            }
        }
    }
    // Move the NPC if a better position was found
    if (bestDist != SHRT_MAX) {
        move_character(npc, bestMove.x, bestMove.y, world.w[world.curY][world.curX]);
    }
    else {
        char current_terrain = world.w[world.curY][world.curX]->m[npc->y][npc->x];
        int32_t stay_cost = get_cost(current_terrain, npc->x, npc->y, npc->type);
        npc->next_turn += stay_cost; // Adjust next_turn based on the terrain cost  

        // update_character_turn(npc);//!
    }
}

void move_towards_player_rival(character_t* npc) {
    int bestDist = SHRT_MAX; // Initialize with maximum possible distance
    Position bestMove = { 0, 0 };
    int base = rand() & 0x7;
    // Iterate over all possible directions, including diagonals
    for (int i = base; i < 8 + base; i++) {
        int newX = npc->x + directions[i & 0x7].x;
        int newY = npc->y + directions[i & 0x7].y;

        // Ensure the new position is within bounds and valid
        if (newX > 0 && newX < MAP_WIDTH - 1 && newY > 0 && newY < MAP_HEIGHT - 1 && is_position_valid_for_npc(newX, newY, npc->type)) {
            int dist = world.rivalDist[newY][newX]; // Distance to the player from the new position
            if (dist < bestDist) {
                bestDist = dist;
                bestMove.x = newX;
                bestMove.y = newY;
            }
        }
    }

    // Move the NPC if a better position was found
    if (bestDist != SHRT_MAX) {
        move_character(npc, bestMove.x, bestMove.y, world.w[world.curY][world.curX]);
    }
    else {
        char current_terrain = world.w[world.curY][world.curX]->m[npc->y][npc->x];
        int32_t stay_cost = get_cost(current_terrain, npc->x, npc->y, npc->type);
        npc->next_turn += stay_cost; // Adjust next_turn based on the terrain cost
        // update_character_turn(npc);//!
    }
}
void move_explorer(character_t* npc) {
    // Explorers try to avoid water but can move freely otherwise
    map_t* m = world.w[world.curY][world.curX];
    bool moved = false;
    int base = rand() & 0x7;
    for (int i = base; i < 8 + base; i++) {
        Position new_pos = { npc->x + directions[i & 0x7].x, npc->y + directions[i & 0x7].y };
        // Check if the new position is valid and not water
        if (is_position_valid_for_npc(new_pos.x, new_pos.y, npc->type) && mapxy(new_pos.x, new_pos.y) != '~') {
            move_character(npc, new_pos.x, new_pos.y, world.w[world.curY][world.curX]);
            moved = true;
            break; // Move has been made, exit loop
        }
    }
    if (!moved) {
        char current_terrain = world.w[world.curY][world.curX]->m[npc->y][npc->x];
        int32_t stay_cost = get_cost(current_terrain, npc->x, npc->y, npc->type);
        npc->next_turn += stay_cost; // Adjust next_turn based on the terrain cost
        // update_character_turn(npc);//!
    }
}
void move_swimmer(character_t* npc) {
    // Swimmer moves randomly within water tiles
    bool moved = false;
    // int base = rand() & 0x7;
    int attempts = 0;
    while (!moved && attempts < 8) { // Limit attempts to avoid infinite loops
        int dir_index = rand() % 8; // Choose a random direction
        int new_x = npc->x + directions[dir_index].x;
        int new_y = npc->y + directions[dir_index].y;

        // Check if the new position is within map bounds
        if (new_x > 0 && new_x < MAP_WIDTH - 1 && new_y > 0 && new_y < MAP_HEIGHT - 1) {
            char terrain = world.w[world.curY][world.curX]->m[new_y][new_x];

            // Ensure the new position is a water tile and not occupied by another character
            if (terrain == '~') {
                // && !character_at_position(new_x, new_y
                move_character(npc, new_x, new_y, world.w[world.curY][world.curX]);
                moved = true; // Successfully moved
            }
        }
        attempts++;
    }
    // If the swimmer couldn't move, it simply waits (i.e., increment its next turn without changing position)
    if (!moved) {
        char current_terrain = world.w[world.curY][world.curX]->m[npc->y][npc->x];
        int32_t stay_cost = get_cost(current_terrain, npc->x, npc->y, npc->type);
        npc->next_turn += stay_cost; // Adjust next_turn based on the terrain cost
        // update_character_turn(npc);//!
    }
}

void move_npc(character_t* npc) {
    switch (npc->type) {
    case Hiker:
        if (npc->changedMove) {
            move_pacer(npc);
        }
        else {
            move_towards_player_hiker(npc);
        }
        break;
    case Rival:
        if (npc->changedMove) {
            move_pacer(npc);
        }
        else {
            move_towards_player_rival(npc);
        }
        break;
    case Swimmer:
        move_swimmer(npc);
        break;
    case Other:
        switch (npc->symbol) {
        case 'p':
            move_pacer(npc);
            break;
        case 'w':
            move_wanderer(npc);
            break;
        case 's':
            // Sentries do not move.
            npc->next_turn += get_cost(world.w[world.curY][world.curX]->m[npc->y][npc->x], npc->x, npc->y, npc->type);
            break;
        case 'e':
            move_explorer(npc);
            break;
        default:
            printf("Error in move_npc()! Unknown NPC type.\n");
            npc->next_turn += get_cost(world.w[world.curY][world.curX]->m[npc->y][npc->x], npc->x, npc->y, npc->type);
            break;
        }
        break;
    default:
        printf("Error in move_npc()! Unhandled NPC type.\n");
        npc->next_turn += get_cost(world.w[world.curY][world.curX]->m[npc->y][npc->x], npc->x, npc->y, npc->type);
        break;
    }
}

// Function to add a valid position (ensure not to add duplicates)
void addValidPosition(int x, int y)
{
    validPositionsForBuildings[validPositionsCount].x = x;
    validPositionsForBuildings[validPositionsCount].y = y;
    validPositionsCount++;
}

// After createPaths, populate validPositionsForBuildings with adjacent non-path tiles
void collectValidPositions(map_t* m)
{
    validPositionsCount = 0; // Reset count
    // Iterate over the map to find and add valid positions
    for (int y = 2; y < MAP_HEIGHT - 2; y++)
    {
        for (int x = 2; x < MAP_WIDTH - 2; x++)
        {
            if (m->m[y][x] == '#')
            {
                addValidPosition(x, y);
            }
        }
    }
}

void createSingleCenterOrMart(map_t* m, char building);

void world_init() {
    int i, j;
    world.curX = WORLD_WIDTH / 2;
    world.curY = WORLD_HEIGHT / 2;
    for (i = 0; i < WORLD_HEIGHT; i++) {
        for (j = 0; j < WORLD_WIDTH; j++) {
            world.w[i][j] = NULL;
        }
    }
    // heap_init(&event_heap, characters_turn_comp, NULL); //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! event heap?
    world.global_sequence_number = 0;
    world.npc_count = 0;
}


// Function to create the border of the map
void createBorder(map_t* m)
{
    for (int i = 0; i < MAP_WIDTH; i++)
    {
        if (m->m[0][i] != '#')
        {
            m->m[0][i] = '%';
        }
        if (m->m[MAP_HEIGHT - 1][i] != '#')
        {
            m->m[MAP_HEIGHT - 1][i] = '%';
        }
    }
    for (int i = 0; i < MAP_HEIGHT; i++)
    {
        if (m->m[i][0] != '#')
        {
            m->m[i][0] = '%';
        }
        if (m->m[i][MAP_WIDTH - 1] != '#')
        {
            m->m[i][MAP_WIDTH - 1] = '%';
        }
    }
}

// Function to initialize regions
void initializeRegions(struct Region regions[NUM_REGIONS])
{
    for (int i = 0; i < NUM_REGIONS; i++)
    {
        regions[i].symbol = ' '; // Initialize symbol as empty
    }
}

void assignRegions(struct Region regions[NUM_REGIONS])
{
    // Initially assign a unique symbol to each region to ensure coverage
    for (int i = 0; i < NUM_REGIONS; i++)
    {
        regions[i].symbol = symbols[i];
        // symbols is {'%', '^', ':', ':', '.', '.', '~'}
    }
    bool notSame = false;
    int firstWater = rand() % NUM_REGIONS;
    regions[firstWater].symbol = '~';
    notSame = false;
    int tallGrass;
    while (!notSame)
    {
        tallGrass = rand() % NUM_REGIONS;
        if (tallGrass != firstWater)
        { // Not to overlap with water that we already have
            regions[tallGrass].symbol = ':';
            notSame = true;
        }
    }
    notSame = false;
    int firstRock;
    while (!notSame)
    {
        firstRock = rand() % NUM_REGIONS;
        if (firstRock != firstWater && firstRock != tallGrass)
        { // Not to overlap with water that we already have
            regions[firstRock].symbol = '%';
            notSame = true;
        }
    }
    notSame = false;
    int firstTallGrass;
    int secondTallGrass;
    while (!notSame)
    {
        firstTallGrass = rand() % NUM_REGIONS;
        if (firstTallGrass != firstWater && firstTallGrass != tallGrass && firstTallGrass != firstRock)
        { // Not to overlap with water that we already have
            regions[firstTallGrass].symbol = ':';
            notSame = true;
        }
        secondTallGrass = rand() % NUM_REGIONS;
        if (firstTallGrass != secondTallGrass && firstTallGrass + 1 != secondTallGrass && secondTallGrass != firstWater && secondTallGrass != tallGrass && secondTallGrass != firstRock)
        { // making sure there are 2 separate tall grass regions
            regions[secondTallGrass].symbol = ':';
            notSame = true;
        }
        else
        {
            if (secondTallGrass + 1 < NUM_REGIONS && secondTallGrass + 1 != firstWater && secondTallGrass + 1 != firstTallGrass &&
                secondTallGrass + 1 != firstRock && secondTallGrass + 1 != tallGrass)
            {
                regions[secondTallGrass + 1].symbol = ':';
                notSame = true;
            }
            else if (secondTallGrass - 1 >= 0 && secondTallGrass - 1 != firstWater && secondTallGrass - 1 != firstTallGrass &&
                secondTallGrass - 1 != firstRock && secondTallGrass - 1 != tallGrass)
            {
                regions[secondTallGrass - 1].symbol = ':';
                notSame = true;
            }
        }
    }
}

/* Using Perlin's noise to make the map prettier and very very unexpected.*/
typedef struct {
    float x, y;
} Vector2D;

typedef struct {
    Vector2D grad[2][2]; // Gradients for the corners of a grid cell
} GradientGrid;

// Linear interpolation
float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

// Smoothstep function for smoother transitions
float smoothstep(float t) {
    return t * t * (3 - 2 * t);
}

// Dot product of two Vector2D
float dot(Vector2D a, Vector2D b) {
    return a.x * b.x + a.y * b.y;
}

// Generate a random gradient vector
Vector2D randomGradient() {
    float angle = (float)rand() / (float)RAND_MAX * 2 * M_PI;
    Vector2D v;
    v.x = cos(angle);
    v.y = sin(angle);
    return v;
}

// Perlin noise function using improved gradients
float perlinNoise(float x, float y, GradientGrid* grid) {
    // Determine grid cell coordinates
    int x0 = (int)floor(x);
    int y0 = (int)floor(y);

    // Local coordinates within the grid cell
    float localX = x - (float)x0;
    float localY = y - (float)y0;

    // Fetch gradients for the corners of the grid cell
    Vector2D g00 = grid->grad[0][0];
    Vector2D g10 = grid->grad[1][0];
    Vector2D g01 = grid->grad[0][1];
    Vector2D g11 = grid->grad[1][1];

    // Calculate noise contributions from each of the four corners
    float n00 = dot(g00, (Vector2D) { localX, localY });
    float n10 = dot(g10, (Vector2D) { localX - 1, localY });
    float n01 = dot(g01, (Vector2D) { localX, localY - 1 });
    float n11 = dot(g11, (Vector2D) { localX - 1, localY - 1 });

    // Interpolate along x
    float ix0 = lerp(n00, n10, smoothstep(localX));
    float ix1 = lerp(n01, n11, smoothstep(localX));

    // Interpolate along y and return the final noise value
    float value = lerp(ix0, ix1, smoothstep(localY));

    return value;
}

// Map noise values to terrain types
TerrainType terrainTypeBasedOnNoiseValue(float noise) {
    if (noise < -0.1) {
        return WATER;
    }
    else if (noise < 0.0) {
        return GRASS;
    }
    else if (noise < 0.1) {
        return CLEARING;
    }
    else if (noise < 0.2) {
        return TREE;
    }
    else {
        return BOULDER;
    }
}

void generateTerrainWithNoise(map_t* m) {
    GradientGrid grid;
    // Assign random gradients to each corner of the grid
    grid.grad[0][0] = randomGradient();
    grid.grad[1][0] = randomGradient();
    grid.grad[0][1] = randomGradient();
    grid.grad[1][1] = randomGradient();

    double scale = 0.1; // Adjust for more or less frequency in terrain changes
    TerrainType terrain;
    double noiseValue;
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            noiseValue = perlinNoise(x * scale, y * scale, &grid);
            terrain = terrainTypeBasedOnNoiseValue(noiseValue);
            switch (terrain) {
            case GRASS:
                m->m[y][x] = ':';
                break;
            case CLEARING:
                m->m[y][x] = '.';
                break;
            case TREE:
                m->m[y][x] = '^';
                break;
            case BOULDER:
                m->m[y][x] = '%';
                break;
            case WATER:
                m->m[y][x] = '~';
                break;
            default:
                printf("\nError! Unexpected TerrainType was found in generateTerrainWithNoise()\n");
            }
        }
    }
}


void setRegionCoordinates(struct Region regions[NUM_REGIONS])
{
    for (int i = 0; i < NUM_REGIONS; i++)
    {
        regions[i].fromX = (i % 3) * (MAP_WIDTH / 3);
        regions[i].toX = ((i % 3) + 1) * (MAP_WIDTH / 3) - 1;
        regions[i].fromY = (i / 3) * (MAP_HEIGHT / ((NUM_REGIONS + 1) / 3)); // Dividing by 2 (NUM_REGIONS/3) for a 3x2 grid
        regions[i].toY = ((i / 3) + 1) * (MAP_HEIGHT / ((NUM_REGIONS + 1) / 3)) - 1;
    }
}

// Function to create the map using region information
void createMap(map_t* m, struct Region regions[NUM_REGIONS])
{
    // Initially fill the map with a default terrain to avoid empty spaces
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            m->m[y][x] = '.'; // Default fill
        }
    }

    // Then assign specific terrains based on regions
    for (int i = 0; i < NUM_REGIONS; i++)
    {
        for (int x = regions[i].fromX; x <= regions[i].toX; x++)
        {
            for (int y = regions[i].fromY; y <= regions[i].toY; y++)
            {
                m->m[y][x] = regions[i].symbol;
            }
        }
    }
}

void createSingleCenterOrMart(map_t* m, char building)
{
    if (validPositionsCount == 0)
        return; // No valid positions available
    int idx;
    Position pos;
    while (true) {
        idx = rand() % validPositionsCount;
        pos = validPositionsForBuildings[idx];

        // Example logic to place building next to selected path position
        if (m->m[pos.y][pos.x + 1] != 'M' && m->m[pos.y][pos.x + 1] != 'C' && m->m[pos.y][pos.x + 1] != '#')
        {
            m->m[pos.y][pos.x + 1] = building;
            return;
        }
        else if (m->m[pos.y][pos.x - 1] != 'M' && m->m[pos.y][pos.x - 1] != 'C' && m->m[pos.y][pos.x - 1] != '#')
        {
            m->m[pos.y][pos.x - 1] = building;
            return;
        }
        else if (m->m[pos.y + 1][pos.x] != 'M' && m->m[pos.y + 1][pos.x] != 'C' && m->m[pos.y + 1][pos.x] != '#')
        {
            m->m[pos.y + 1][pos.x] = building;
            return;
        }
        else if (m->m[pos.y - 1][pos.x] != 'M' && m->m[pos.y - 1][pos.x] != 'C' && m->m[pos.y + 1][pos.x] != '#')
        {
            m->m[pos.y - 1][pos.x] = building;
            return;
        }
    }
}

void createCC(map_t* m)
{
    createSingleCenterOrMart(m, 'C');
}

void createPokemart(map_t* m)
{
    createSingleCenterOrMart(m, 'M');
}

void printMap(map_t* m)
{
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            if (world.npcs[y][x]) {
                printf("%c", world.npcs[y][x]->symbol);
            }
            else {
                printf("%c", m->m[y][x]);
            }
        }
        printf("\n");
    }
}

void sprinkle(map_t* m)
{
    for (int i = 0; i < 50; i++)
    {
        int y1 = (rand() % (MAP_HEIGHT - 6)) + 3;
        int x1 = (rand() % (MAP_WIDTH - 7)) + 3;
        int y2 = (rand() % (MAP_HEIGHT - 6)) + 3;
        int x2 = (rand() % (MAP_WIDTH - 7)) + 3;
        if (m->m[y1][x1] != '#' && m->m[y1][x1] != 'C' && m->m[y1][x1] != 'M')
        {
            m->m[y1][x1] = '^';
        }
        if (m->m[y2][x2] != '#' && m->m[y2][x2] != 'C' && m->m[y2][x2] != 'M' && (x2 % 7) == 0) // lower the possibility of adding more %
        {
            m->m[y2][x2] = '%';
        }
    }
}

void createPaths(map_t* m)
//! The issue with north (either first map or all of them) is that I think the gate does not exist on the first map
// ! But I think they exist because of the if statements. Anyways fix the bottom gates.
{
    int topExit = -1, leftExit = -1, bottomExit = -1, rightExit = -1;

    // Correctly align top and bottom exits with adjacent maps if they exist
    if (world.curY > 0 && world.w[world.curY - 1][world.curX])
    {
        // Align top exit with the bottom exit of the map above
        topExit = world.w[world.curY - 1][world.curX]->bottomExit;
    }
    if (world.curY < WORLD_HEIGHT - 1 && world.w[world.curY + 1][world.curX])
    {
        // Preemptively align bottom exit with the top exit of the map below
        bottomExit = world.w[world.curY + 1][world.curX]->topExit;
    }
    // Left neighbor
    if (world.curX > 0 && world.w[world.curY][world.curX - 1])
    {

        leftExit = world.w[world.curY][world.curX - 1]->rightExit;
    }
    // Right neighbor
    if (world.curX < WORLD_WIDTH - 1 && world.w[world.curY][world.curX + 1])
    {

        rightExit = world.w[world.curY][world.curX + 1]->leftExit;
    }

    // Initialize gate positions randomly if they're not set by adjacent maps
    if (topExit == -1)
        topExit = (rand() % (MAP_WIDTH - 7)) + 3;
    if (leftExit == -1)
        leftExit = (rand() % (MAP_HEIGHT - 6)) + 3;
    if (bottomExit == -1)
        bottomExit = (rand() % (MAP_WIDTH - 7)) + 3;
    if (rightExit == -1)
        rightExit = (rand() % (MAP_HEIGHT - 6)) + 3;

    int currentX = topExit;
    // For vertical path
    int deviation;
    int direction;

    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        if (y == 0)
        { // for easier access
            m->topExit = currentX;
        }
        else if (y == MAP_HEIGHT - 1)
        {
            m->bottomExit = currentX;
        }
        m->m[y][currentX] = '#';

        if (y >= 2 && y < MAP_HEIGHT - 2 && currentX >= 2 && currentX < MAP_WIDTH - 2) {
            addValidPosition(currentX, y);
        }

        // Random deviation in the first half
        if (y < MAP_HEIGHT / 2)
        {
            deviation = (rand() % 3) - 1; // Random deviation: -1, 0, 1
            currentX += deviation;
            currentX = currentX < 1 ? 1 : (currentX >= MAP_WIDTH - 1 ? MAP_WIDTH - 2 : currentX);
        }
        else
        {
            // Adjust direction towards bottomExit in the second half, ensuring no division by zero
            if (currentX != bottomExit)
            {
                direction = (bottomExit - currentX) / abs(bottomExit - currentX);
                currentX += direction;
                // Ensure currentX does not overshoot or undershoot the target exit
                if ((direction > 0 && currentX > bottomExit) || (direction < 0 && currentX < bottomExit))
                {
                    currentX = bottomExit;
                }
            }
        }
    }

    int currentY = leftExit;
    // For horizontal path
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        if (x == 0)
        { // for easier access
            m->leftExit = currentY;
        }
        else if (x == MAP_WIDTH - 1)
        {
            m->rightExit = currentY;
        }
        m->m[currentY][x] = '#';

        if (currentY >= 2 && currentY < MAP_HEIGHT - 2 && x >= 2 && x < MAP_WIDTH - 2) {
            addValidPosition(x, currentY);
        }
        // Random deviation in the first half
        if (x < MAP_WIDTH / 2)
        {
            deviation = (rand() % 3) - 1; // Random deviation: -1, 0, 1
            currentY += deviation;
            currentY = currentY < 1 ? 1 : (currentY >= MAP_HEIGHT - 1 ? MAP_HEIGHT - 2 : currentY);
        }
        else
        {
            // Adjust direction towards rightExit in the second half, ensuring no division by zero
            if (currentY != rightExit)
            {
                direction = (rightExit - currentY) / abs(rightExit - currentY);
                currentY += direction;
                // Ensure currentY does not overshoot or undershoot the target exit
                if ((direction > 0 && currentY > rightExit) || (direction < 0 && currentY < rightExit))
                {
                    currentY = rightExit;
                }
            }
        }
    }
}

character_t* create_pc(map_t* m) {
    if (validPositionsCount == 0)
        return NULL; // No valid positions available

    int idx = rand() % validPositionsCount;
    Position pos = validPositionsForBuildings[idx];
    character_t* pc = create_character(pos, PC, '@');
    // m->m[pos.y][pos.x] = '@';
    world.npcs[pos.y][pos.x] = pc; //!
    return pc;
}


static int32_t compRival(const void* key, const void* with) { // comparator for rivals
    return (world.rivalDist[((path_t*)key)->pos[1]][((path_t*)key)->pos[0]] - world.rivalDist[((path_t*)with)->pos[1]][((path_t*)with)->pos[0]]);
}
static int32_t compHiker(const void* key, const void* with) { // comparator for rivals
    return (world.hikerDist[((path_t*)key)->pos[1]][((path_t*)key)->pos[0]] - world.hikerDist[((path_t*)with)->pos[1]][((path_t*)with)->pos[0]]);
}


// A function to find the shortest path for hiker and rival to get to the pc.
void dijkstra(map_t* m)
{
    static path_t path[MAP_HEIGHT][MAP_WIDTH]; // maintain the value accross function calls.
    static uint32_t initialized = 0;
    static path_t* p;
    heap_t h;
    uint32_t x, y;
    if (!initialized)
    {
        for (y = 0; y < MAP_HEIGHT; y++)
        {
            for (x = 0; x < MAP_WIDTH; x++)
            {
                path[y][x].pos[1] = y; // willl never change again
                path[y][x].pos[0] = x; // willl never change again
            }
        }
        initialized = 1;
    }
    for (y = 0; y < MAP_HEIGHT; y++) // Initialize the d value of hiker dist and rival dist and assign them infinity
    {
        for (x = 0; x < MAP_WIDTH; x++)
        {
            world.hikerDist[y][x] = SHRT_MAX;
            world.rivalDist[y][x] = SHRT_MAX;
        }
    }
    // Distance for pc
    world.hikerDist[world.pc.y][world.pc.x] = 0;
    world.rivalDist[world.pc.y][world.pc.x] = 0;

    /*Rival*/
    heap_init(&h, compRival, NULL); // initialize the heap for rival

    for (y = 1; y < MAP_HEIGHT - 1; y++)
    {
        for (x = 1; x < MAP_WIDTH - 1; x++)
        {
            // Don't add the infinity in the heap because they've not been reached yet
            if (get_cost(m->m[y][x], x, y, Rival) != SHRT_MAX) {
                path[y][x].hn = heap_insert(&h, &path[y][x]);
            }
            else {
                path[y][x].hn = NULL;
            }
        }
    }
    while ((p = heap_remove_min(&h)))
    {
        p->hn = NULL;
        if ((path[p->pos[1] - 1][p->pos[0]].hn) && // is the north neighbor in the heap?
            (world.rivalDist[p->pos[1] - 1][p->pos[0]] > world.rivalDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Rival))) // copmparing the cost
        {
            world.rivalDist[p->pos[1] - 1][p->pos[0]] = world.rivalDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Rival);
            heap_decrease_key_no_replace(&h, path[p->pos[1] - 1][p->pos[0]].hn);
        }

        if ((path[p->pos[1]][p->pos[0] - 1].hn) &&
            (world.rivalDist[p->pos[1]][p->pos[0] - 1] > world.rivalDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Rival)))
        {
            world.rivalDist[p->pos[1]][p->pos[0] - 1] = world.rivalDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Rival);
            heap_decrease_key_no_replace(&h, path[p->pos[1]][p->pos[0] - 1].hn);
        }

        if ((path[p->pos[1]][p->pos[0] + 1].hn) &&
            (world.rivalDist[p->pos[1]][p->pos[0] + 1] > world.rivalDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Rival)))
        {
            world.rivalDist[p->pos[1]][p->pos[0] + 1] = world.rivalDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Rival);
            heap_decrease_key_no_replace(&h, path[p->pos[1]][p->pos[0] + 1].hn);
        }

        if ((path[p->pos[1] + 1][p->pos[0]].hn) &&
            (world.rivalDist[p->pos[1] + 1][p->pos[0]] > world.rivalDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Rival)))
        {
            world.rivalDist[p->pos[1] + 1][p->pos[0]] = world.rivalDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Rival);
            heap_decrease_key_no_replace(&h, path[p->pos[1] + 1][p->pos[0]].hn);
        }

        if ((path[p->pos[1] + 1][p->pos[0] - 1].hn) &&
            (world.rivalDist[p->pos[1] + 1][p->pos[0] - 1] > world.rivalDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Rival)))
        {
            world.rivalDist[p->pos[1] + 1][p->pos[0] - 1] = world.rivalDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Rival);
            heap_decrease_key_no_replace(&h, path[p->pos[1] + 1][p->pos[0] - 1].hn);
        }

        if ((path[p->pos[1] - 1][p->pos[0] - 1].hn) &&
            (world.rivalDist[p->pos[1] - 1][p->pos[0] - 1] > world.rivalDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Rival)))
        {
            world.rivalDist[p->pos[1] - 1][p->pos[0] - 1] = world.rivalDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Rival);
            heap_decrease_key_no_replace(&h, path[p->pos[1] - 1][p->pos[0] - 1].hn);
        }

        if ((path[p->pos[1] + 1][p->pos[0] + 1].hn) &&
            (world.rivalDist[p->pos[1] + 1][p->pos[0] + 1] > world.rivalDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Rival)))
        {
            world.rivalDist[p->pos[1] + 1][p->pos[0] + 1] = world.rivalDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Rival);
            heap_decrease_key_no_replace(&h, path[p->pos[1] + 1][p->pos[0] + 1].hn);
        }

        if ((path[p->pos[1] - 1][p->pos[0] + 1].hn) &&
            (world.rivalDist[p->pos[1] - 1][p->pos[0] + 1] > world.rivalDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Rival)))
        {
            world.rivalDist[p->pos[1] - 1][p->pos[0] + 1] = world.rivalDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Rival);
            heap_decrease_key_no_replace(&h, path[p->pos[1] - 1][p->pos[0] + 1].hn);
        }
    }

    heap_delete(&h);

    /*Hiker*/
    heap_init(&h, compHiker, NULL); // initialize the heap for rival

    for (y = 1; y < MAP_HEIGHT - 1; y++)
    {
        for (x = 1; x < MAP_WIDTH - 1; x++)
        {
            // Don't add the infinity in the heap because they've not been reached yet
            if (get_cost(m->m[y][x], x, y, Hiker) != SHRT_MAX) {
                path[y][x].hn = heap_insert(&h, &path[y][x]);
            }
            else {
                path[y][x].hn = NULL;
            }
        }
    }
    while ((p = heap_remove_min(&h)))
    {
        p->hn = NULL;
        if ((path[p->pos[1] - 1][p->pos[0]].hn) && // is the north neighbor in the heap?
            (world.hikerDist[p->pos[1] - 1][p->pos[0]] > world.hikerDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Hiker))) // copmparing the cost
        {
            world.hikerDist[p->pos[1] - 1][p->pos[0]] = world.hikerDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Hiker);
            heap_decrease_key_no_replace(&h, path[p->pos[1] - 1][p->pos[0]].hn);
        }

        if ((path[p->pos[1]][p->pos[0] - 1].hn) &&
            (world.hikerDist[p->pos[1]][p->pos[0] - 1] > world.hikerDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Hiker)))
        {
            world.hikerDist[p->pos[1]][p->pos[0] - 1] = world.hikerDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Hiker);
            heap_decrease_key_no_replace(&h, path[p->pos[1]][p->pos[0] - 1].hn);
        }

        if ((path[p->pos[1]][p->pos[0] + 1].hn) &&
            (world.hikerDist[p->pos[1]][p->pos[0] + 1] > world.hikerDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Hiker)))
        {
            world.hikerDist[p->pos[1]][p->pos[0] + 1] = world.hikerDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Hiker);
            heap_decrease_key_no_replace(&h, path[p->pos[1]][p->pos[0] + 1].hn);
        }

        if ((path[p->pos[1] + 1][p->pos[0]].hn) &&
            (world.hikerDist[p->pos[1] + 1][p->pos[0]] > world.hikerDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Hiker)))
        {
            world.hikerDist[p->pos[1] + 1][p->pos[0]] = world.hikerDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Hiker);
            heap_decrease_key_no_replace(&h, path[p->pos[1] + 1][p->pos[0]].hn);
        }

        if ((path[p->pos[1] + 1][p->pos[0] - 1].hn) &&
            (world.hikerDist[p->pos[1] + 1][p->pos[0] - 1] > world.hikerDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Hiker)))
        {
            world.hikerDist[p->pos[1] + 1][p->pos[0] - 1] = world.hikerDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Hiker);
            heap_decrease_key_no_replace(&h, path[p->pos[1] + 1][p->pos[0] - 1].hn);
        }

        if ((path[p->pos[1] - 1][p->pos[0] - 1].hn) &&
            (world.hikerDist[p->pos[1] - 1][p->pos[0] - 1] > world.hikerDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Hiker)))
        {
            world.hikerDist[p->pos[1] - 1][p->pos[0] - 1] = world.hikerDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Hiker);
            heap_decrease_key_no_replace(&h, path[p->pos[1] - 1][p->pos[0] - 1].hn);
        }

        if ((path[p->pos[1] + 1][p->pos[0] + 1].hn) &&
            (world.hikerDist[p->pos[1] + 1][p->pos[0] + 1] > world.hikerDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Hiker)))
        {
            world.hikerDist[p->pos[1] + 1][p->pos[0] + 1] = world.hikerDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Hiker);
            heap_decrease_key_no_replace(&h, path[p->pos[1] + 1][p->pos[0] + 1].hn);
        }

        if ((path[p->pos[1] - 1][p->pos[0] + 1].hn) &&
            (world.hikerDist[p->pos[1] - 1][p->pos[0] + 1] > world.hikerDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Hiker)))
        {
            world.hikerDist[p->pos[1] - 1][p->pos[0] + 1] = world.hikerDist[p->pos[1]][p->pos[0]] + get_cost(m->m[p->pos[1]][p->pos[0]], p->pos[0], p->pos[1], Hiker);
            heap_decrease_key_no_replace(&h, path[p->pos[1] - 1][p->pos[0] + 1].hn);
        }
    }

    heap_delete(&h);
}


void cleanup_characters(void *v) {
    free((character_t *) v);
}
void free_npcs(){
    world.npc_count = 0; // Reset the NPC count after cleanup
    for (int y = 0; y < WORLD_HEIGHT; y++)
    {
        for (int x = 0; x < WORLD_WIDTH; x++)
        {
            if (world.npcs[y][x]) {
                free(world.npcs[y][x]);
                world.npcs[y][x] = NULL;
            }
        }
    }
}
void freeAllMaps()
{
    free_npcs();
    heap_delete(&event_heap); //* Need to iterate in the future
    for (int y = 0; y < WORLD_HEIGHT; y++)
    {
        for (int x = 0; x < WORLD_WIDTH; x++)
        {
            if (world.w[y][x])
            {
                free(world.w[y][x]);
                world.w[y][x] = NULL;
            }
        }
    }
}
/* calls all of the functions neccessary to create a single map */
void newMapCaller()
{
    struct Region regions[NUM_REGIONS];
    // Only proceed if the map does not exist
    if (!world.w[world.curY][world.curX])
    {
        world.w[world.curY][world.curX] = malloc(sizeof(map_t));
        initializeRegions(regions);
        assignRegions(regions);
        setRegionCoordinates(regions);
        createMap(world.w[world.curY][world.curX], regions); // add gates parameters
        generateTerrainWithNoise(world.w[world.curY][world.curX]);
        createPaths(world.w[world.curY][world.curX]);
        // collectValidPositions(world.w[world.curY][world.curX]); // Done in place to make it faster
        createBorder(world.w[world.curY][world.curX]);

        int d = abs(world.curX - (WORLD_WIDTH / 2)) + abs(world.curY - (WORLD_HEIGHT / 2)); // Manhattan distance from the center

        // Calculate the probability of placing buildings based on the distance
        double probOfBuildings = d > 200 ? 5.0 : (50.0 - (45.0 * d) / 200.0);

        // Generate a Pokémon Center if a random number is below the calculated probability or if we're at the center of the world
        if ((rand() % 100) < probOfBuildings || !d)
        {                                              // Using d == 0 to explicitly check for the center
            createCC(world.w[world.curY][world.curX]); // Place a Pokémon Center
        }

        // Similarly, generate a Pokémart under the same conditions
        if ((rand() % 100) < probOfBuildings || !d)
        {
            createPokemart(world.w[world.curY][world.curX]); // Place a Pokémart
        }

        sprinkle(world.w[world.curY][world.curX]);
        // After generating the map, store the pointer in the world
        // *world.w[world.curY][world.curX] = world.w[world.curY][world.curX]->m;
        heap_init(&event_heap, characters_turn_comp, cleanup_characters);
        collectValidPositions(world.w[world.curY][world.curX]);
        // placePlayer(world.w[world.curY][world.curX]); // place '@' on road, called once bc there is only one player in the world
        character_t* pc = create_pc(world.w[world.curY][world.curX]);
        world.pc = *pc;
        world.pc.heap_node = heap_insert(&event_heap, &world.pc);
        dijkstra(world.w[world.curY][world.curX]);
    }
}
void enter_pokemart() {
    mvprintw(0, 0, "Tried to enter the Pokemart");
    refresh();
    getch();
}
void enter_pokemon_center() {
    mvprintw(0, 0, "Tried to enter the Pokeman Center");
    refresh();
    getch();
}
static const char* character_type_to_string(CharacterType type) {
    switch (type) {
    case PC: return "PC";
    case Hiker: return "Hiker";
    case Rival: return "Rival";
    case Swimmer: return "Swimmer";
    case Other: return "Trainer";
    default: return "Unknown";
    }
}

static void display_trainers(character_t** c, uint32_t count) {
    uint32_t i;
    mvprintw(1, 0, "You know of %d trainers:", count);

    for (i = 0; i < count && i < LINES - 3; ++i) {
        const char* typeStr = character_type_to_string(c[i]->type);
        mvprintw(i + 2, 0, "%16s %c: %2d %s by %2d %s",
            typeStr,
            c[i]->symbol,
            abs(c[i]->y - world.pc.y),
            (c[i]->y - world.pc.y <= 0 ? "North" : "South"),
            abs(c[i]->x - world.pc.x),
            (c[i]->x - world.pc.x <= 0 ? "West" : "East"));
    }

    mvprintw(LINES - 1, 0, "Press any key to return. Use Arrow keys for more.");
    refresh();
}


void display() {
    uint32_t y, x;
    clear();
    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            if (world.npcs[y][x]) {
                mvaddch(y + 1, x, world.npcs[y][x]->symbol);
            }
            else {
                switch (world.w[world.curY][world.curX]->m[y][x]) {
                case '^':
                case ':':
                case '.':
                    attron(COLOR_PAIR(COLOR_GREEN));
                    mvaddch(y + 1, x, '^');
                    attroff(COLOR_PAIR(COLOR_GREEN));
                    break;
                case '%':
                    attron(COLOR_PAIR(COLOR_MAGENTA));
                    mvaddch(y + 1, x, '%');
                    attroff(COLOR_PAIR(COLOR_MAGENTA));
                    break;
                case '#':
                    attron(COLOR_PAIR(COLOR_RED));
                    mvaddch(y + 1, x, '#');
                    attroff(COLOR_PAIR(COLOR_RED));
                    break;
                case '~':
                    attron(COLOR_PAIR(COLOR_BLUE));
                    mvaddch(y + 1, x, '~');
                    attroff(COLOR_PAIR(COLOR_BLUE));
                    break;
                case 'M':
                    attron(COLOR_PAIR(COLOR_CYAN));
                    mvaddch(y + 1, x, 'M');
                    attroff(COLOR_PAIR(COLOR_CYAN));
                    break;
                case 'C':
                    attron(COLOR_PAIR(COLOR_YELLOW));
                    mvaddch(y + 1, x, 'C');
                    attroff(COLOR_PAIR(COLOR_YELLOW));
                    break;
                case '@':
                    attron(COLOR_PAIR(COLOR_WHITE));
                    mvaddch(y + 1, x, '@');
                    attroff(COLOR_PAIR(COLOR_WHITE));
                    break;
                default:
                    attron(COLOR_PAIR(COLOR_RED));
                    mvaddch(y + 1, x, '?');
                    attroff(COLOR_PAIR(COLOR_RED));
                }
            }
        }
    }
    refresh();
}
static void move_pc_func(character_t* character, map_t* m) {
    display();
    get_input();
    move_character(character, character->x, character->y, m);
}
static void list_trainers() {
    character_t** c;
    uint32_t x, y, count = 0;

    c = malloc(world.npc_count * sizeof(*c));
    // Check for malloc failure
    if (!c) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }

    /* Populate the list of trainers */
    for (y = 1; y < MAP_HEIGHT - 1; y++) {
        for (x = 1; x < MAP_WIDTH - 1; x++) {
            if (world.npcs[y][x] && (y != world.pc.y || x != world.pc.x)) {
                c[count++] = world.npcs[y][x];
            }
        }
    }
    display_trainers(c, count);
    free(c);
}

void battle(character_t* character) {
    display();
    mvprintw(0, 0, "Battle? You wanna fight?");
    refresh();
    getch();
    character->lost = 1;
    if (character->type == Hiker || character->type == Rival) {
        character->changedMove = 1; // Will no longer path to the PC. It moves like pacers
    }
}
uint32_t move_pc(uint32_t input) {
    Position pos;
    pos.y = world.pc.x;
    pos.x = world.pc.y;
    switch (input) {
    case 1:
    case 2:
    case 3:
        pos.y++;
        break;
    case 4:
    case 5:
    case 6:
        break;
    case 7:
    case 8:
    case 9:
        pos.y--;
        break;
    }
    switch (input) {
    case 1:
    case 4:
    case 7:
        pos.x--;
        break;
    case 2:
    case 5:
    case 8:
        break;
    case 3:
    case 6:
    case 9:
        pos.x++;
        break;
    case '>':
    case '<':
        if (world.w[world.curY][world.curX]->m[world.pc.y][world.pc.x] == 'M') {
            enter_pokemart();
        }
        if (world.w[world.curY][world.curX]->m[world.pc.y][world.pc.x] == 'C') {
            enter_pokemon_center();
        }
        break;
    }
    if ((world.pc.x == world.w[world.curY][world.curX]->topExit && world.pc.y == 0) ||
        (world.pc.x == world.w[world.curY][world.curX]->bottomExit && world.pc.y == MAP_HEIGHT - 1) ||
        (world.pc.y == world.w[world.curY][world.curX]->leftExit && world.pc.x == 0) ||
        (world.pc.y == world.w[world.curY][world.curX]->rightExit && world.pc.x == MAP_WIDTH - 1)) {
        return 1;
    }
    int cost = get_cost(world.w[world.curY][world.curX]->m[world.pc.y][world.pc.x], world.pc.x, world.pc.y, PC);
    if (world.npcs[world.pc.y][world.pc.x] && world.npcs[world.pc.y][world.pc.x]->lost) {
        return 1;
    }
    // Battle
    else if (world.npcs[world.pc.y][world.pc.x]) { // Not lost
        battle(world.npcs[world.pc.y][world.pc.x]);
    }
    if (cost == SHRT_MAX) {
        return 1;
    }
    return 0;
}

void get_input() {
    int input;
    uint32_t out;
    do {
        switch (input = getch()) {
        case 1:
        case 'b':
        case KEY_END:
            out = move_pc(1);
            break;
        case 2:
        case 'j':
        case KEY_DOWN:
            out = move_pc(2);
            break;
        case 3:
        case 'n':
        case KEY_NPAGE:
            out = move_pc(3);
            break;
        case 4:
        case 'h':
        case KEY_LEFT:
            out = move_pc(4);
            break;
        case 5:
        case ' ':
        case '.':
        case KEY_B2:
            out = 0;
            break;
        case 6:
        case 'l':
        case KEY_RIGHT:
            out = move_pc(6);
            break;
        case 7:
        case 'y':
        case KEY_HOME:
            out = move_pc(7);
            break;
        case 8:
        case 'k':
        case KEY_UP:
            out = move_pc(8);
            break;
        case 9:
        case 'u':
        case KEY_PPAGE:
            out = move_pc(9);
            break;
        case '>':
            out = move_pc('>');
            break;
        case 'Q':
            out = 0;
            quit = 1;
            break;
            break; //?
        case 't':
            list_trainers(); //?
            out = 1;
            break;
        default:
            mvprintw(0, 0, "Unbound key: %#o", input);
            out = 1;
        }

        refresh();
    } while (out);
}

void init_io() {
    initscr(); // Initializes the terminal in cursor mode.
    raw(); //Disable inline buffering
    noecho();// Switches off echoing
    curs_set(0); // Setting the appearence of curser to invisible
    keypad(stdscr, TRUE);
    start_color();
    init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
    init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
}

int main(int argc, char* argv[])
{
    // Random seed number generating
    struct timeval tv;
    uint32_t seed;
    if (argc == 2)
    {
        seed = atoi(argv[1]);
    }
    else
    {
        gettimeofday(&tv, NULL);
        seed = (tv.tv_usec ^ (tv.tv_sec << 20)) & 0xffffffff;
    }
    srand(seed);
    init_io();
    world_init();
    newMapCaller();
    // input numtrainers
    int numtrainers = 10; // Default
    for (int i = 1; i < argc; i++) { // Starting from 1 because the first el. of argv is the name of the program. So skip it
        if (strcmp(argv[i], "--numtrainers") == 0 && i + 1 < argc && argv[i + 1] > 0) {
            numtrainers = atoi(argv[i + 1]);
            i++; // Skip next argument since we've processed it
        }
    }
    generate_npcs(numtrainers, world.w[world.curY][world.curX]); // Place after map is created
    character_t* current_char;
    // int32_t cost;
    while (!quit) {
        current_char = heap_remove_min(&event_heap);
        if (!current_char) {
            // No characters left to process, potentially exit the loop or handle the end of the game
            break;
        }
        // printf("\n%c: (%d,%d) next_turn: %d sequence_number: (%d)\n", current_char->symbol, current_char->x,
        // current_char->y, current_char->next_turn, current_char->sequence_number);  
        if (current_char->type == PC) {
            // move the pc
            move_pc_func(current_char, world.w[world.curY][world.curX]);
        }
        else {
            move_npc(current_char);
        }
        // Reinsert the current character back into the event heap
        heap_insert(&event_heap, current_char);
    }
    freeAllMaps();
    // cleanup_characters();
    endwin();
    return 0;
}