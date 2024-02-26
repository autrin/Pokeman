#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <sys/time.h>
#include <limits.h> 
#include <math.h>

#include "heap.h"

#define MAP_WIDTH 80     // width of the map
#define MAP_HEIGHT 21    // height of the map
#define NUM_REGIONS 5    // Number of regions
#define WORLD_HEIGHT 401 // world of all of the maps
#define WORLD_WIDTH 401
#define SHRT_MAX __SHRT_MAX__
#define mapxy(x, y) (m->m[y][x])

void newMapCaller(void);

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
} pc_t;
typedef struct world
{
    map_t* w[WORLD_HEIGHT][WORLD_WIDTH];
    int32_t curX; // x of the current map
    int32_t curY; // y of the current map
    pc_t pc;
    int hikerDist[MAP_HEIGHT][MAP_WIDTH];
    int rivalDist[MAP_HEIGHT][MAP_WIDTH];
} world_t;

world_t world;

// Define character types
typedef enum {
    PC,
    Hiker,
    Rival,
    Swimmer,
    Other,
    Num_Character_Types // Keeps track of the number of character types
} CharacterType; //? Do I need other?

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
    default:
        printf("@Error in get_cost()! Terrain type '%c' is unidentified.\n", terrainChar);
        // Return a high cost for unidentified terrain to avoid using it in pathfinding.
        return SHRT_MAX;
    }
    return cost[character][terrain];
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
            // Example condition: adjacent to path and not a building or border
            if (m->m[y][x] == '#'
                // &&(m->m[y + 1][x] == '.' || m->m[y - 1][x] == '.' ||
                //     m->m[y][x + 1] == '.' || m->m[y][x - 1] == '.' ||
                //     m->m[y + 1][x] == ':' || m->m[y - 1][x] == ':' ||
                //     m->m[y][x + 1] == ':' || m->m[y][x - 1] == ':' ||
                //     m->m[y + 1][x] == '^' || m->m[y - 1][x] == '.' ||
                //     m->m[y][x + 1] == '.' || m->m[y][x - 1] == '.' )
                )
            {
                addValidPosition(x, y);
            }
        }
    }
}

void createSingleCenterOrMart(map_t* m, char building);

int world_init()
{ // initializing each map of the world to NULL
    for (int i = 0; i < WORLD_HEIGHT; i++)
    {
        for (int j = 0; j < WORLD_WIDTH; j++)
        {
            world.w[j][i] = NULL;
        }
    }
    world.curX = WORLD_WIDTH / 2; // the starting point is the center, which is (200, 200) internally to us developers,
    // but (0, 0) externally in the output.
    world.curY = WORLD_HEIGHT / 2;
    return 0;
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
            printf("%c", m->m[y][x]);
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

void fly(int newX, int newY)
{
    // Convert newX and newY to internal world coordinates if necessary
    int internalX = newX + 200; // Assuming the center (0,0) is at (200,200)
    int internalY = newY + 200;

    // Validate coordinates to ensure they're within bounds
    if (internalX < 0 || internalX >= WORLD_WIDTH || internalY < 0 || internalY >= WORLD_HEIGHT)
    {
        printf("Cannot fly to (%d, %d): Out of bounds.\n", newX, newY);
        return;
    }
    // Update current position
    world.curY = internalY;

    // Check if a map exists at the new location
    world.curX = internalX;
    if (world.w[world.curY][world.curX] == NULL)
    {
        // If not, generate a new map for this location
        newMapCaller(); // Assumes newMapCaller uses world.curX and world.curY
    }
    else
    {
        // If the map already exists, there's nothing else we need to do
        // The map will be rendered in the next iteration of the main loop
        printf("Flying to (%d, %d).\n", newX, newY);
    }
}

void createPaths(map_t* m, int topExit, int leftExit, int bottomExit, int rightExit)
//! The issue with north (either first map or all of them) is that I think the gate does not exist on the first map
// ! But I think they exist because of the if statements. Anyways fix the bottom gates.
{

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

void placePlayer(map_t* m)
{
    if (validPositionsCount == 0)
        return; // No valid positions available

    int idx = rand() % validPositionsCount;
    Position pos = validPositionsForBuildings[idx];

    m->m[pos.y][pos.x] = '@';
    world.pc.x = pos.x;
    world.pc.y = pos.y;
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
    // deleting things that are not dynamic is an error

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

void printHiker_RivalMap() {
    int i, j;
    for (i = 0; i < MAP_HEIGHT; i++) {
        for (j = 0; j < MAP_WIDTH; j++) {
            if (world.hikerDist[i][j] == SHRT_MAX) { // If infinity then print space 
                printf("   ");
            }
            else {
                printf("%02d ", world.hikerDist[i][j] % 100);
            }
        }
        printf("\n");
    }
    printf("\n");

    for (i = 0; i < MAP_HEIGHT; i++) {
        for (j = 0; j < MAP_WIDTH; j++) {
            if (world.rivalDist[i][j] == SHRT_MAX) { // If infinity then print space
                printf("   ");
            }
            else {
                printf("%02d ", world.rivalDist[i][j] % 100);
            }
        }
        printf("\n");
    }
}

void newMapCaller()
{ // calls all of the functions neccessary to create a single map
    struct Region regions[NUM_REGIONS];
    // Only proceed if the map does not exist
    if (!world.w[world.curY][world.curX])
    {
        // map_t map;
        // world.w[world.curY][world.curX] = malloc(sizeof(*world.w[world.curY][world.curX]));
        world.w[world.curY][world.curX] = malloc(sizeof(map_t));
        initializeRegions(regions);
        assignRegions(regions);
        setRegionCoordinates(regions);
        createMap(world.w[world.curY][world.curX], regions); // add gates parameters
        generateTerrainWithNoise(world.w[world.curY][world.curX]);

        int topExit = -1, leftExit = -1, bottomExit = -1, rightExit = -1;

        /* Adjust gate positions based on existing neighboring maps */
        // Top neighbor
        if (world.curY > 0 && world.w[world.curY - 1][world.curX])
        {
            topExit = world.w[world.curY - 1][world.curX]->bottomExit;
        }
        // Bottom neighbor
        if (world.curY < WORLD_HEIGHT - 1 && world.w[world.curY + 1][world.curX])
        {
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

        createPaths(world.w[world.curY][world.curX], topExit, leftExit, bottomExit, rightExit);
        collectValidPositions(world.w[world.curY][world.curX]);
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
    }
}

void freeMap(int y, int x)
{
    if (world.w[y][x])
    {                         // Check if the map at the location has been allocated.
        free(world.w[y][x]);  // Free the map structure.
        world.w[y][x] = NULL; // Set the pointer to NULL after freeing.
    }
}

void freeAllMaps()
{
    for (int y = 0; y < WORLD_HEIGHT; y++)
    {
        for (int x = 0; x < WORLD_WIDTH; x++)
        {
            freeMap(y, x); // Free each map if it has been allocated
        }
    }
}

int main(int argc, char* argv[])
{
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

    world_init();
    newMapCaller(); // This should automatically use world.curY and world.curX
    collectValidPositions(world.w[world.curY][world.curX]);
    placePlayer(world.w[world.curY][world.curX]); // place '@' on road, called once bc there is only one player in the world
    dijkstra(world.w[world.curY][world.curX]);
    printMap(world.w[world.curY][world.curX]);
    printHiker_RivalMap();
    freeAllMaps();
    return 0; // We only need the code upto this part for this iteration.

    // input commands
    char c;
    int fx, fy; // flying coordinates

    do
    {
        if (world.w[world.curY][world.curX] != NULL)
        {
            printMap(world.w[world.curY][world.curX]);
            printf("(%d, %d)\n", world.curX - 200, world.curY - 200); // display the coordinates
        }
        else
        {
            printf("No map exists at this location.\n");
        }
        printf("Enter command: ");
        // c = getchar(); // Read a single character command
        // getchar();     // Consume the newline character after the command

        if (scanf(" %c", &c) != 1) // it is going to read one char and put the rest in place for later
        {
            /* To handle EOF */
            putchar('\n');
            break;
        }

        switch (c)
        {
        case 'q': // quit the game
            freeAllMaps();
            break;
        case 'f': // fly to the (x, y) coordinate
            printf("Enter coordiantes in this form: x y\n");
            scanf("%d %d", &fx, &fy);
            // getchar(); // Consume the newline character after the coordinates
            fly(fx, fy);
            break;
        case 'n': // move to the north map
            if (world.curY - 1 >= 0)
            {
                world.curY--;
                newMapCaller();
            }
            else
            {
                printf("Oops! Not a valid move!\n");
            }
            break;
        case 's': // move to the south map
            if (world.curY + 1 < WORLD_HEIGHT)
            {
                world.curY++;
                newMapCaller();
            }
            else
            {
                printf("Oops! Not a valid move!\n");
            }
            break;
        case 'w': // move to the west map
            if (world.curX - 1 >= 0)
            {
                world.curX--;
                newMapCaller();
            }
            else
            {
                printf("Oops! Not a valid move!\n");
            }
            break;
        case 'e': // move to the east map
            if (world.curX + 1 < WORLD_WIDTH)
            {
                world.curX++;
                newMapCaller();
            }
            else
            {
                printf("Oops! Not a valid move!\n");
            }
            break;
        default:
            fprintf(stderr, "%c: Invalid input.\n", c);
            break;
        }
    } while (c != 'q');

    return 0;
}
