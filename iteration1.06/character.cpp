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
#include "character.h"
// direction vectors: N, NE, E, SE, S, SW, W, NW
static Position directions[8] = { {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1} };

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
        if ((x == world.w[world.y][world.x]->topExit && y == 0) ||
            (x == world.w[world.y][world.x]->bottomExit && y == MAP_HEIGHT - 1) ||
            (y == world.w[world.y][world.x]->leftExit && x == 0) ||
            (y == world.w[world.y][world.x]->rightExit && x == MAP_WIDTH - 1)) {
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

int32_t characters_turn_comp(const void* key, const void* with)
{
    return ((((character*)key)->next_turn == ((character*)with)->next_turn) ? (((character*)key)->sequence_number - ((character*)with)->sequence_number)
        : (((character*)key)->next_turn - ((character*)with)->next_turn)); // If there is a tie, compare the sequence numbers.
}
character* create_character(Position pos, CharacterType type, char symbol) {
    character* new_char = (character*)malloc(sizeof(character));
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
    new_char->direction = rand() % 2;
    return new_char;
}

bool is_position_valid_for_npc(int32_t x, int32_t y, CharacterType npcype) {
    // Check bounds first to ensure we're looking within the map's dimensions
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) { // Might need this for PC but npcs check this in their move functions
        return false; // Position is out of bounds           // this function is only for npc so delete this?
    }

    // Check if the position is an exit (assuming exits are marked with '#')
    if (world.w[world.y][world.x]->m[y][x] == '#' && ((x == world.w[world.y][world.x]->topExit && y == 0) ||
        (x == world.w[world.y][world.x]->bottomExit && y == MAP_HEIGHT - 1) ||
        (y == world.w[world.y][world.x]->leftExit && x == 0) ||
        (y == world.w[world.y][world.x]->rightExit && x == MAP_WIDTH - 1))) { //* PC can go through gates for later iterations
        return false; // NPCs should avoid exits
    }
    // Check for the presence of another NPC or the PC at the position
    if (world.w[world.y][world.x]->npcs[y][x] || (x == world.pc.x && y == world.pc.y)) {
        return false; // Position is already occupied by another NPC or the PC
    }

    // Specific terrain checks based on NPC type
    char terrain = world.w[world.y][world.x]->m[y][x];
    switch (npcype) {
    case Swimmer:
        // Swimmers can only move in water
        if (terrain != '~') {
            return false;
        }
        break;
    case PC:
        if (terrain == '%' || terrain == '^' || terrain == '~') {
            return false;
        }
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
// void rand_pos(Position *pos)
// {
//   pos->x = (rand() % (MAP_WIDTH - 2)) + 1;
//   pos->y = (rand() % (MAP_HEIGHT - 2)) + 1;
// }
// Position find_valid_position_for_npc(CharacterType npcype) {
//     Position pos;
//     switch (npcype) {
//     case Hiker:
//         do {
//             pos.x = (rand() % (MAP_WIDTH - 2)) + 1;
//             pos.y = (rand() % (MAP_HEIGHT - 2)) + 1;
//         } while (world.hikerDist[pos.y][pos.x] == SHRT_MAX ||
//             world.w[world.y][world.x]->npcs[pos.y][pos.x]
//             || pos.x < 3 || pos.x > MAP_WIDTH - 4 ||
//             pos.y < 3 || pos.y > MAP_HEIGHT - 4);
//         break;
//     case Rival:
//         do {
//             pos.x = (rand() % (MAP_WIDTH - 2)) + 1;
//             pos.y = (rand() % (MAP_HEIGHT - 2)) + 1;
//         } while (world.rivalDist[pos.y][pos.x] == SHRT_MAX || world.rivalDist[pos.y][pos.x] < 0
//             || world.w[world.y][world.x]->npcs[pos.y][pos.x]
//             || pos.x < 3 || pos.x > MAP_WIDTH - 4 ||
//             pos.y < 3 || pos.y > MAP_HEIGHT - 4);
//         break;
//     case Swimmer:
//         do {
//             pos.x = (rand() % (MAP_WIDTH - 2)) + 1;
//             pos.y = (rand() % (MAP_HEIGHT - 2)) + 1;
//         } while (world.w[world.y][world.x]->m[pos.y][pos.x] != '~' || world.w[world.y][world.x]->npcs[pos.y][pos.x]);
//         break;
//     case Other:
//         do {
//             pos.x = (rand() % (MAP_WIDTH - 2)) + 1;
//             pos.y = (rand() % (MAP_HEIGHT - 2)) + 1;
//         } while (world.rivalDist[pos.y][pos.x] == SHRT_MAX || world.rivalDist[pos.y][pos.x] < 0
//             || world.w[world.y][world.x]->npcs[pos.y][pos.x]
//             || pos.x < 3 || pos.x > MAP_WIDTH - 4 ||
//             pos.y < 3 || pos.y > MAP_HEIGHT - 4);
//         break;
//     case PC:
//         do {
//             pos.x = rand() % ((MAP_WIDTH - 2)) + 1;
//             pos.y = rand() % ((MAP_HEIGHT - 2)) + 1;
//         } while (world.w[world.y][world.x]->npcs[pos.y][pos.x] ||
//             get_cost(world.w[world.y][world.x]->m[pos.y][pos.x], pos.x, pos.y, PC) == SHRT_MAX ||
//             world.rivalDist[pos.y][pos.x] < 0);
//             break;
//     case Num_characterypes:
//     default:
//         printf("Error in find_valid_position_for_npc()");
//         break;
//     }
//     return pos;
// }
Position find_valid_position_for_npc(CharacterType npcype) {
    // Position pos = { -1, -1 }; // Initialize to an invalid position
    // for (int y = 1; y < MAP_HEIGHT - 1; y++) {
    //     for (int x = 1; x < MAP_WIDTH - 1; x++) {
    //         switch (npcype) {
    //         case Hiker:
    //             if (world.hikerDist[y][x] != SHRT_MAX &&
    //                 !world.w[world.y][world.x]->npcs[y][x]) {
    //                 pos.x = x;
    //                 pos.y = y;
    //                 return pos; // Found a valid position, return immediately
    //             }
    //             break;
    //         case Rival:
    //             if (world.rivalDist[y][x] != SHRT_MAX && world.rivalDist[y][x] >= 0
    //                 && !world.w[world.y][world.x]->npcs[y][x]) {
    //                 pos.x = x;
    //                 pos.y = y;
    //                 return pos;
    //             }
    //             break;
    //         case Swimmer:
    //             if (world.w[world.y][world.x]->m[y][x] == '~' && !world.w[world.y][world.x]->npcs[y][x]) {
    //                 pos.x = x;
    //                 pos.y = y;
    //                 return pos;
    //             }
    //             break;
    //         case Other:
    //             if ((world.rivalDist[y][x] != SHRT_MAX && world.rivalDist[y][x] >= 0) && !world.w[world.y][world.x]->npcs[y][x]) {
    //                 pos.x = x;
    //                 pos.y = y;
    //                 return pos;
    //             }
    //             break;
    //         case PC:
    //             // Assuming `get_cost()` is a function that determines if the position is valid for a PC
    //             if (!world.w[world.y][world.x]->npcs[y][x] && get_cost(world.w[world.y][world.x]->m[y][x], x, y, PC) != SHRT_MAX &&
    //                 world.rivalDist[y][x] >= 0) {
    //                 pos.x = x;
    //                 pos.y = y;
    //                 return pos;
    //             }
    //             break;
    //         case Num_characterypes:
    //         default:
    //             // Handle unexpected npcype
    //             printf("Error in find_valid_position_for_npc() with npcype: %d", npcype);
    //             return pos; // Return the invalid position
    //         }
    //     }
    // }
    // // No valid position found, return the invalid position
    // return pos;
    Position pos;
    for (int y = 1; y < MAP_HEIGHT - 1; y++) {
        for (int x = 1; x < MAP_WIDTH - 1; x++) {
            pos.x = rand() % MAP_WIDTH;
            pos.y = rand() % MAP_HEIGHT;
            if (is_position_valid_for_npc(pos.x, pos.y, npcype)) {
                return pos;
            }
        }
    }
    return pos;
}

// Update character's next turn and reinsert into the event heap
void update_character_turn(character* character) {
    if (character->heap_node) {
        heap_remove_min(&world.w[world.y][world.x]->event_heap);
    }
    character->heap_node = heap_insert(&world.w[world.y][world.x]->event_heap, character);
}
void generate_npcs(int numtrainers, map* map) {
    for (int i = 0; i < numtrainers; i++) {
        // Adjusted to avoid selecting PC and Num_characterypes as NPC types
        CharacterType npcype = (CharacterType)(1 + rand() % (Num_characterypes - 1)); // Randomly select NPC type, skipping PC and Num_characterypes

        Position pos = find_valid_position_for_npc(npcype);

        char symbol = 'N'; // Default symbol
        switch (npcype) {
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
            // Explicitly ignoring PC and Num_characterypes since they should not be selected
        case PC:
        case Num_characterypes:
            continue; // Skip the rest of the loop iteration if these values are somehow selected
        }
        character* npc = create_character(pos, npcype, symbol);
        if (npc) {
            npc->heap_node = heap_insert(&world.w[world.y][world.x]->event_heap, npc);
            world.w[world.y][world.x]->npcs[pos.y][pos.x] = npc;
            world.w[world.y][world.x]->npc_count++;
        }
    }
}

void move_character(character* character, int direction_x, int direction_y, map* m, Position* pos) {
    int old_x = character->x;
    int old_y = character->y;
    int32_t movement_cost = get_cost(m->m[direction_y][direction_x], direction_x, direction_y, character->type);
    // Update the character's next turn based on movement cost
    character->next_turn += movement_cost;

    // Move the character to the new position
    if (is_position_valid_for_npc(direction_x, direction_y, character->type)) {
        character->x = direction_x;
        character->y = direction_y;
    }
    // Empty the npc map's old location
    world.w[world.y][world.x]->npcs[old_y][old_x] = NULL;
    // Update the npc map's new location
    world.w[world.y][world.x]->npcs[character->y][character->x] = character;
    // printf("%d %d\n", pos.x, pos.y);
    if (character->type == PC) {
        // world.pc.x = pos->x;
        // world.pc.y = pos->y;
        world.pc.x = character->x;
        world.pc.y = character->y;
    }
}

void move_pacer(character* npc) {
    // Assuming npc->direction stores the pacer's current direction (e.g., 0 for horizontal, 1 for vertical)
    int dx = (npc->direction == 0) ? 1 : 0; // Move horizontally if direction is 0
    int dy = (npc->direction == 1) ? 1 : 0; // Move vertically if direction is 1
    Position* pos = NULL;
    // Check if the next position in the current direction is valid
    if (npc->x + dx > 0 && npc->x + dx < MAP_WIDTH - 1 && npc->y + dy > 0 && npc->y + dy < MAP_HEIGHT - 1 && is_position_valid_for_npc(npc->x + dx, npc->y + dy, npc->type)) {
        move_character(npc, npc->x + dx, npc->y + dy, world.w[world.y][world.x], pos);
    }
    else {
        // If blocked, reverse direction
        npc->direction = (npc->direction + 1) % 2; // Toggle direction
        dx = (npc->direction == 0) ? 1 : 0;
        dy = (npc->direction == 1) ? 1 : 0;
        move_character(npc, npc->x - dx, npc->y - dy, world.w[world.y][world.x], pos); // Move in the opposite direction
    }
}

void move_wanderer(character* npc) {
    bool moved = false;
    int attempts = 0;
    Position* pos = NULL;
    while (!moved && attempts < 8) {
        // Example for a randomly moving NPC (like a wanderer)
        int dir_index = rand() % 8; // Choose a random direction
        int new_x = npc->x + directions[dir_index].x;
        int new_y = npc->y + directions[dir_index].y;
        if (new_x > 0 && new_x < MAP_WIDTH - 1 && new_y > 0 && new_y < MAP_HEIGHT - 1 && is_position_valid_for_npc(new_x, new_y, npc->type)) {
            move_character(npc, new_x, new_y, world.w[world.y][world.x], pos);
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
        char current_terrain = world.w[world.y][world.x]->m[npc->y][npc->x];
        int32_t stay_cost = get_cost(current_terrain, npc->x, npc->y, npc->type);
        npc->next_turn += stay_cost; // Adjust next_turn based on the terrain cost

        // update_character_turn(npc);//!
    }
}

void move_towards_player_hiker(character* npc) {
    int bestDist = SHRT_MAX; // Initialize with maximum possible distance
    Position bestMove = { 0, 0 };
    int base = rand() & 0x7;
    int newX;
    int newY;
    Position* pos = NULL;
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
        move_character(npc, bestMove.x, bestMove.y, world.w[world.y][world.x], pos);
    }
    else {
        char current_terrain = world.w[world.y][world.x]->m[npc->y][npc->x];
        int32_t stay_cost = get_cost(current_terrain, npc->x, npc->y, npc->type);
        npc->next_turn += stay_cost; // Adjust next_turn based on the terrain cost  

        // update_character_turn(npc);//!
    }
}

void move_towards_player_rival(character* npc) {
    int bestDist = SHRT_MAX; // Initialize with maximum possible distance
    Position bestMove = { 0, 0 };
    Position* pos = NULL;
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
        move_character(npc, bestMove.x, bestMove.y, world.w[world.y][world.x], pos);
    }
    else {
        char current_terrain = world.w[world.y][world.x]->m[npc->y][npc->x];
        int32_t stay_cost = get_cost(current_terrain, npc->x, npc->y, npc->type);
        npc->next_turn += stay_cost; // Adjust next_turn based on the terrain cost
        // update_character_turn(npc);//!
    }
}
void move_explorer(character* npc) {
    // Explorers try to avoid water but can move freely otherwise
    map* m = world.w[world.y][world.x];
    bool moved = false;
    Position* pos = NULL;
    int base = rand() & 0x7;
    for (int i = base; i < 8 + base; i++) {
        Position new_pos = { npc->x + directions[i & 0x7].x, npc->y + directions[i & 0x7].y };
        // Check if the new position is valid and not water
        if (is_position_valid_for_npc(new_pos.x, new_pos.y, npc->type) && mapxy(new_pos.x, new_pos.y) != '~') {
            move_character(npc, new_pos.x, new_pos.y, world.w[world.y][world.x], pos);
            moved = true;
            break; // Move has been made, exit loop
        }
    }
    if (!moved) {
        char current_terrain = world.w[world.y][world.x]->m[npc->y][npc->x];
        int32_t stay_cost = get_cost(current_terrain, npc->x, npc->y, npc->type);
        npc->next_turn += stay_cost; // Adjust next_turn based on the terrain cost
        // update_character_turn(npc);//!
    }
}
void move_swimmer(character* npc) {
    // Swimmer moves randomly within water tiles
    bool moved = false;
    Position* pos = NULL;
    // int base = rand() & 0x7;
    int attempts = 0;
    while (!moved && attempts < 8) { // Limit attempts to avoid infinite loops
        int dir_index = rand() % 8; // Choose a random direction
        int new_x = npc->x + directions[dir_index].x;
        int new_y = npc->y + directions[dir_index].y;

        // Check if the new position is within map bounds
        if (new_x > 0 && new_x < MAP_WIDTH - 1 && new_y > 0 && new_y < MAP_HEIGHT - 1) {
            char terrain = world.w[world.y][world.x]->m[new_y][new_x];

            // Ensure the new position is a water tile and not occupied by another character
            if (terrain == '~') {
                // && !character_at_position(new_x, new_y
                move_character(npc, new_x, new_y, world.w[world.y][world.x], pos);
                moved = true; // Successfully moved
            }
        }
        attempts++;
    }
    // If the swimmer couldn't move, it simply waits (i.e., increment its next turn without changing position)
    if (!moved) {
        char current_terrain = world.w[world.y][world.x]->m[npc->y][npc->x];
        int32_t stay_cost = get_cost(current_terrain, npc->x, npc->y, npc->type);
        npc->next_turn += stay_cost; // Adjust next_turn based on the terrain cost
        // update_character_turn(npc);//!
    }
}

void move_npc(character* npc) {
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
            npc->next_turn += get_cost(world.w[world.y][world.x]->m[npc->y][npc->x], npc->x, npc->y, npc->type);
            break;
        case 'e':
            move_explorer(npc);
            break;
        default:
            printf("Error in move_npc()! Unknown NPC type.\n");
            npc->next_turn += get_cost(world.w[world.y][world.x]->m[npc->y][npc->x], npc->x, npc->y, npc->type);
            break;
        }
        break;
    default:
        printf("Error in move_npc()! Unhandled NPC type.\n");
        npc->next_turn += get_cost(world.w[world.y][world.x]->m[npc->y][npc->x], npc->x, npc->y, npc->type);
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

// After createPaths, populate validPositionsForBuildings with adjacent non-path_ttiles
void collectValidPositions(map* m)
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

character* create_pc(map* m) {
    if (validPositionsCount == 0)
        return NULL; // No valid positions available
    int idx = rand() % validPositionsCount;
    Position pos = validPositionsForBuildings[idx];
    character* pc = create_character(pos, PC, '@');
    world.w[world.y][world.x]->npcs[pos.y][pos.x] = pc;
    return pc;
}


int32_t compRival(const void* key, const void* with) { // comparator for rivals
    return (world.rivalDist[((path_t*)key)->pos[1]][((path_t*)key)->pos[0]] - world.rivalDist[((path_t*)with)->pos[1]][((path_t*)with)->pos[0]]);
}
int32_t compHiker(const void* key, const void* with) { // comparator for rivals
    return (world.hikerDist[((path_t*)key)->pos[1]][((path_t*)key)->pos[0]] - world.hikerDist[((path_t*)with)->pos[1]][((path_t*)with)->pos[0]]);
}


// A function to find the shortest path_tfor hiker and rival to get to the pc.
void dijkstra(map* m)
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
    while ((p = (path_t*)heap_remove_min(&h)))
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
    while ((p = (path_t*)heap_remove_min(&h)))
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


void cleanup_characters(void* v) {

    free((character*)v);
}
// void free_npcs() {
//     world.w[world.y][world.x]->npc_count = 0; // Reset the NPC count after cleanup
//     for (int y = 0; y < WORLD_HEIGHT; y++)
//     {
//         for (int x = 0; x < WORLD_WIDTH; x++)
//         {
//             if (world.w[world.y][world.x]->npcs[y][x]) {
//                 // printf("ERROR on line 1249");
//                 free(world.w[world.y][world.x]->npcs[y][x]);
//                 world.w[world.y][world.x]->npcs[y][x] = NULL;
//             }
//         }
//     }
// }

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
const char* characterype_to_string(CharacterType type) {
    switch (type) {
    case PC: return "PC";
    case Hiker: return "Hiker";
    case Rival: return "Rival";
    case Swimmer: return "Swimmer";
    case Other: return "Trainer";
    default: return "Unknown";
    }
}

void display_trainers(character** c, int count) {
    int i;
    mvprintw(1, 0, "You know of %d trainers:", count);

    for (i = 0; i < count && i < LINES - 3; ++i) {
        const char* typeStr = characterype_to_string(c[i]->type);
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
            if (world.w[world.y][world.x]->npcs[y][x]) {
                mvaddch(y + 1, x, world.w[world.y][world.x]->npcs[y][x]->symbol);
            }
            else {
                switch (world.w[world.y][world.x]->m[y][x]) {
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
void move_pc_func(character* character, map* m, Position* pos) {
    display();
    get_input(pos);
    // move_character(character, character->x, character->y, m, pos);
}
void list_trainers() {
    character** c;
    int x, y, count = 0;

    c = (character**)malloc(world.w[world.y][world.x]->npc_count * sizeof(*c));
    // Check for malloc failure
    if (!c) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }

    /* Populate the list of trainers */
    for (y = 1; y < MAP_HEIGHT - 1; y++) {
        for (x = 1; x < MAP_WIDTH - 1; x++) {
            if (world.w[world.y][world.x]->npcs[y][x] && (y != world.pc.y || x != world.pc.x)) {
                c[count++] = world.w[world.y][world.x]->npcs[y][x];
            }
        }
    }
    display_trainers(c, count);
    free(c);
}

void fly(Position* pos)
{
    // Make the old pc's position empty
    world.w[world.y][world.x]->npcs[world.pc.y][world.pc.x] = NULL;
    echo(); // show the coordiantes on the screen 
    curs_set(1);
    int newX, newY;
    do
    {
        mvprintw(0, 0, "Enter x destination. Remember the valid range is [-200,200]: ");
        refresh();
        mvscanw(0, 61, "%d", &newX);
    } while (newX < -200 || newX > 200);
    do
    {
        mvprintw(0, 0, "Enter y destination. Remember the valid range is [-200,200]: ");
        refresh();
        mvscanw(0, 64, "%d", &newY);
    } while (newY < -200 || newY > 200);
    refresh();
    noecho();
    curs_set(0);
    // Convert newX and newY to internal world coordinates if necessary
    int internalX = newX + WORLD_WIDTH / 2; // Assuming the center (0,0) is at (200,200)
    int internalY = newX + WORLD_HEIGHT / 2;
    // Update current position
    world.y = internalY;
    world.x = internalX;
    newMapCaller(1); //checks for the existance of the map itself
    // Now try to place the pc
    do {
        pos->x = rand() % ((MAP_WIDTH - 2)) + 1;
        pos->y = rand() % ((MAP_HEIGHT - 2)) + 1;
    } while (world.w[world.y][world.x]->npcs[pos->y][pos->x] ||
        get_cost(world.w[world.y][world.x]->m[pos->y][pos->x], pos->x, pos->y, PC) == SHRT_MAX ||
        world.rivalDist[pos->y][pos->x] < 0);
}
void battle(character* character) { //TODO needs to be changed in the future
    display();
    mvprintw(0, 0, "Are you ready for a battle?");
    refresh();
    getch();
    character->lost = 1;
    if (character->type == Hiker || character->type == Rival) {
        character->changedMove = 1; // Will no longer path_tto the PC. It moves like pacers
    }
}
uint32_t move_pc(uint32_t input, Position* pos) {
    pos->y = world.pc.y; // Line 1466
    pos->x = world.pc.x;
    switch (input) {
    case 1:
    case 2:
    case 3:
        pos->y++;
        break;
    case 4:
    case 5:
    case 6:
        break;
    case 7:
    case 8:
    case 9:
        pos->y--;
        break;
    }
    switch (input) {
    case 1:
    case 4:
    case 7:
        pos->x--;
        break;
    case 2:
    case 5:
    case 8:
        break;
    case 3:
    case 6:
    case 9:
        pos->x++;
        break;
    case '>':
        if (world.w[world.y][world.x]->m[world.pc.y][world.pc.x] == 'M') {
            enter_pokemart();
        }
        if (world.w[world.y][world.x]->m[world.pc.y][world.pc.x] == 'C') {
            enter_pokemon_center();
        }
        break;
    }
    if (world.w[world.y][world.x]->npcs[pos->y][pos->x] && world.w[world.y][world.x]->npcs[pos->y][pos->x]->lost) {
        return 1;
    }
    // Battle
    else if (world.w[world.y][world.x]->npcs[pos->y][pos->x]) { // Not lost
        battle(world.w[world.y][world.x]->npcs[pos->y][pos->x]);
        pos->y = world.pc.y;
        pos->x = world.pc.x;
    }
    if (((pos->x == world.w[world.y][world.x]->topExit && pos->y == 0) ||
        (pos->x == world.w[world.y][world.x]->bottomExit && pos->y == MAP_HEIGHT - 1) ||
        (pos->y == world.w[world.y][world.x]->leftExit && pos->x == 0) ||
        (pos->y == world.w[world.y][world.x]->rightExit && pos->x == MAP_WIDTH - 1)) && pos->x != world.pc.x && pos->y != world.pc.y) {
        return 1; // not on the gate yet
    }
    if (get_cost(world.w[world.y][world.x]->m[pos->y][pos->x], pos->x, pos->y, PC) == SHRT_MAX) {
        return 1;
    }
    return 0;
}

void get_input(Position* pos) {
    int input;
    uint32_t out;
    // pos = (Position*)malloc(sizeof(Position));
    // if (!pos) {
    // }
    do {
        switch (input = getch()) {
        case 1:
        case 'b':
        case KEY_END:
            out = move_pc(1, pos);
            break;
        case 2:
        case 'j':
        case KEY_DOWN:
            out = move_pc(2, pos);
            break;
        case 3:
        case 'n':
        case KEY_NPAGE:
            out = move_pc(3, pos);
            break;
        case 4:
        case 'h':
        case KEY_LEFT:
            out = move_pc(4, pos);
            break;
        case 5:
        case ' ':
        case '.':
        case KEY_B2:
            pos->y = world.pc.y;
            pos->x = world.pc.x;
            out = 0;
            break;
        case 6:
        case 'l':
        case KEY_RIGHT:
            out = move_pc(6, pos);
            break;
        case 7:
        case 'y':
        case KEY_HOME:
            out = move_pc(7, pos);
            break;
        case 8:
        case 'k':
        case KEY_UP:
            out = move_pc(8, pos);
            break;
        case 9:
        case 'u':
        case KEY_PPAGE:
            out = move_pc(9, pos);
            break;
        case '>':
            out = move_pc('>', pos);
            break;
        case 't':
            list_trainers(); //?
            out = 1;
            break;
        case 'f':
            flushinp();
            fly(pos);
            out = 0;
            break;
        case 'Q':
            pos->y = world.pc.y;
            pos->x = world.pc.x;
            quit = 1;
            break;
        default:
            mvprintw(0, 0, "Unbound key: %#o", input);
            out = 1;
        }
        refresh();
    } while (out);
    move_character(&world.pc, pos->x, pos->y, world.w[world.y][world.x], pos);
    // if(pos){
    // free(pos);
    // pos = NULL;
    // }
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
    newMapCaller(0);

    // input numtrainers
    for (int i = 1; i < argc; i++) { // Starting from 1 because the first el. of argv is the name of the program. So skip it
        if (strcmp(argv[i], "--numtrainers") == 0 && i + 1 < argc && argv[i + 1][0] != '\0') {
            numtrainers = atoi(argv[i + 1]);
            i++; // Skip next argument since we've processed it
        }
    }
    character* current_char;
    Position* pos = NULL;
    // int32_t cost;
    while (!quit) {
        current_char = (character*)heap_remove_min(&world.w[world.y][world.x]->event_heap);
        pos = (Position*)malloc(sizeof(Position));
        if (!current_char) {
            // No characters left to process, potentially exit the loop or handle the end of the game
            break;
        }
        // printf("\n%c: (%d,%d) next_turn: %d sequence_number: (%d)\n", current_char->symbol, current_char->x,
        // current_char->y, current_char->next_turn, current_char->sequence_number);  
        if (current_char->type == PC) {
            // move the pc
            collectValidPositions(world.w[world.y][world.x]);
            move_pc_func(current_char, world.w[world.y][world.x], pos);
            // If the pc is going to another map
            if (pos->x == 0 || pos->y == 0 || pos->x == MAP_WIDTH - 1 || pos->y == MAP_HEIGHT - 1) { // Line 1730
                if (pos->x == 0) {
                    world.x--;
                }
                else if (pos->y == 0) {
                    world.y--;
                }
                else if (pos->x == MAP_WIDTH - 1) {
                    world.x++;
                }
                else if (pos->y == MAP_HEIGHT - 1) {
                    world.y++;
                }
                newMapCaller(0);
                pos->y = current_char->y;
                pos->x = current_char->x;
            }
        }
        else {
            collectValidPositions(world.w[world.y][world.x]);
            move_npc(current_char);
        }
        // Reinsert the current character back into the event heap
        heap_insert(&world.w[world.y][world.x]->event_heap, current_char);
        if (pos) {
            free(pos);
            pos = NULL;
        }
    }
    freeAllMaps();
    // cleanup_characters();
    endwin();
    return 0;
}