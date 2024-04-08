#ifndef MAP_H
#define MAP_H

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
#include "character.h"
#define MAP_WIDTH 80     // width of the map
#define MAP_HEIGHT 21    // height of the map
#define NUM_REGIONS 5    // Number of regions
#define WORLD_HEIGHT 401 // world of all of the maps
#define WORLD_WIDTH 401
#define SHRT_MAX __SHRT_MAX__
#define mapxy(x, y) (m->m[y][x])
int quit = 0;
int numtrainers = 10; // Default
int validPositionsCount = 0; 
extern class World world;
// Global or struct to store valid positions


void newMapCaller(int moveMap);
void display(void);
void get_input(Position* pos);
char symbols[] = { '%', '^', ':', '.', '~' }; // Simplified symbols array

typedef struct path_t {
    heap_node_t* hn;
    uint8_t pos[2];
    uint8_t from[2];
    int32_t cost;
}path_t;

// Define a structure to represent a region
class Region
{
public:
    int32_t fromX, fromY, toX, toY;
    char symbol;
};
Position validPositionsForBuildings[MAP_WIDTH * MAP_HEIGHT]; // Adjust size accordingly
                                                              // Keep track of how many valid positions are stored

class map
{
public:
    char m[MAP_HEIGHT][MAP_WIDTH];
    int topExit, bottomExit, leftExit, rightExit;
    character* npcs[MAP_HEIGHT][MAP_WIDTH];
    int npc_count;
    heap_t event_heap;
};
class World : public Position
{
public:
    map* w[WORLD_HEIGHT][WORLD_WIDTH];
    character pc;
    int hikerDist[MAP_HEIGHT][MAP_WIDTH];
    int rivalDist[MAP_HEIGHT][MAP_WIDTH];
    // character* npcs[MAP_HEIGHT][MAP_WIDTH]; // Moved to map
    // int npc_count;// Moved to map
    uint32_t global_sequence_number;
};
/* Using Perlin's noise to make the map prettier and very very unexpected.*/
typedef struct {
    float x, y;
} Vector2D;

typedef struct {
    Vector2D grad[2][2]; // Gradients for the corners of a grid cell
} GradientGrid;

// void createSingleCenterOrMart(map* m, char building);
void world_init(void);
// void createBorder(map* m);
// void initializeRegions(struct Region regions[NUM_REGIONS]);
// void assignRegions(struct Region regions[NUM_REGIONS]);
// float lerp(float a, float b, float t);
// float smoothstep(float t);
// float dot(Vector2D a, Vector2D b);
// Vector2D randomGradient();
// float perlinNoise(float x, float y, GradientGrid* grid);
// TerrainType terrainTypeBasedOnNoiseValue(float noise);
// void generateTerrainWithNoise(map* m);
// void setRegionCoordinates(struct Region regions[NUM_REGIONS]);
// void createMap(map* m, struct Region regions[NUM_REGIONS]);
// void createSingleCenterOrMart(map* m, char building);
// void createCC(map* m);
// void createPokemart(map* m);
// void printMap(map* m);
// void sprinkle(map* m);
// void createPaths(map* m);
void freeAllMaps(void);
// void newMapCaller(int moveMap);

#endif