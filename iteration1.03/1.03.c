#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <sys/time.h>
#include "heap.h"
#define MAP_WIDTH 80     // width of the map
#define MAP_HEIGHT 21    // height of the map
#define NUM_REGIONS 5    // Number of regions
#define WORLD_HEIGHT 401 // world of all of the maps
#define WORLD_WIDTH 401
#define INT_MAX __INT_MAX__ // used for dijkstra
#define mapxy(x, y) (m->m[y][x])
void newMapCaller(void);

char symbols[] = {'%', '^', ':', '.', '~'}; // Simplified symbols array

typedef struct path
{
    heap_node_t *hn;
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
    // int ewX[MAP_WIDTH * 2];  // The x-axis of east-west path //* sizes are multiplied by 2 because the paths are not straigh but curved
    // int ewY[MAP_HEIGHT * 2]; // The y-axis of east-west path
    // int nsX[MAP_WIDTH * 2];  // The x-axis North-south path
    // int nsY[MAP_HEIGHT * 2]; // The y-axis North-south path
} map_t;

typedef struct pc
{ // player character, which is '@'
    int32_t x;
    int32_t y;
} pc_t;
typedef struct world
{
    map_t *w[WORLD_HEIGHT][WORLD_WIDTH];
    int32_t curX; // x of the current map
    int32_t curY; // y of the current map
    pc_t pc;
    int hikerDist[MAP_HEIGHT][MAP_WIDTH];
    int rivalDist[MAP_HEIGHT][MAP_WIDTH];
} world_t;

world_t world;

// pc_t pc; // making it global since there is only one world.pc in the world.

// Function to add a valid position (ensure not to add duplicates)
void addValidPosition(int x, int y)
{
    validPositionsForBuildings[validPositionsCount].x = x;
    validPositionsForBuildings[validPositionsCount].y = y;
    validPositionsCount++;
}

// After createPaths, populate validPositionsForBuildings with adjacent non-path tiles
void collectValidPositions(map_t *m)
{
    validPositionsCount = 0; // Reset count
    // Iterate over the map to find and add valid positions
    for (int y = 1; y < MAP_HEIGHT - 1; y++)
    {
        for (int x = 1; x < MAP_WIDTH - 1; x++)
        {
            // Example condition: adjacent to path and not a building or border
            if (m->m[y][x] == '#' &&
                (m->m[y + 1][x] == '.' || m->m[y - 1][x] == '.' ||
                 m->m[y][x + 1] == '.' || m->m[y][x - 1] == '.'))
            {
                addValidPosition(x, y);
            }
        }
    }
}

void createSingleCenterOrMart(map_t *m, char building);

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

// Function to add a valid position (ensure not to add duplicates)
void addValidPosition(int x, int y)
{
    validPositionsForBuildings[validPositionsCount].x = x;
    validPositionsForBuildings[validPositionsCount].y = y;
    validPositionsCount++;
}

// After createPaths, populate validPositionsForBuildings with adjacent non-path tiles
void collectValidPositions(map_t *m)
{
    validPositionsCount = 0; // Reset count
    // Iterate over the map to find and add valid positions
    for (int y = 1; y < MAP_HEIGHT - 1; y++)
    {
        for (int x = 1; x < MAP_WIDTH - 1; x++)
        {
            // Example condition: adjacent to path and not a building or border
            if (m->m[y][x] == '#' &&
                (m->m[y + 1][x] == '.' || m->m[y - 1][x] == '.' ||
                 m->m[y][x + 1] == '.' || m->m[y][x - 1] == '.'))
            {
                addValidPosition(x, y);
            }
        }
    }
}

// Function to create the border of the map
void createBorder(map_t *m)
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

void setRegionCoordinates(struct Region regions[NUM_REGIONS])
{
    // Assuming NUM_REGIONS is now 6 for even distribution
    for (int i = 0; i < NUM_REGIONS; i++) // ! this was NUM_REGIONS + 1 before.
    // TODO now you need to refactor the lines below to make it randomized
    {
        regions[i].fromX = (i % 3) * (MAP_WIDTH / 3);
        regions[i].toX = ((i % 3) + 1) * (MAP_WIDTH / 3) - 1;
        regions[i].fromY = (i / 3) * (MAP_HEIGHT / ((NUM_REGIONS + 1) / 3)); // Dividing by 2 (NUM_REGIONS/3) for a 3x2 grid
        regions[i].toY = ((i / 3) + 1) * (MAP_HEIGHT / ((NUM_REGIONS + 1) / 3)) - 1;
    }
}

// Function to create the map using region information
void createMap(map_t *m, struct Region regions[NUM_REGIONS])
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


void createSingleCenterOrMart(map_t *m, char building)
{
    if (validPositionsCount == 0)
        return; // No valid positions available

    int idx = rand() % validPositionsCount;
    Position pos = validPositionsForBuildings[idx];

    // Example logic to place building next to selected path position
    if (m->m[pos.y][pos.x + 1] == '.')
    {
        m->m[pos.y][pos.x + 1] = building;
    }
    else if (m->m[pos.y][pos.x - 1] == '.')
    {
        m->m[pos.y][pos.x - 1] = building;
    }
    else if (m->m[pos.y + 1][pos.x] == '.')
    {
        m->m[pos.y + 1][pos.x] = building;
    }
    else if (m->m[pos.y - 1][pos.x] == '.')
    {
        m->m[pos.y - 1][pos.x] = building;
    }
}

void createCC(map_t *m)
{
    createSingleCenterOrMart(m, 'C');
}

void createPokemart(map_t *m)
{
    createSingleCenterOrMart(m, 'M');
}

void printMap(map_t *m)
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

void sprinkle(map_t *m)
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

void createPaths(map_t *m, int topExit, int leftExit, int bottomExit, int rightExit) //! The issue with north (either first map or all of them) is that I think the gate does not exist on the first map
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

void placePlayer(map_t *m)
{
    if (validPositionsCount == 0)
        return; // No valid positions available

    int idx = rand() % validPositionsCount;
    Position pos = validPositionsForBuildings[idx];

    m->m[pos.y][pos.x] = '@';
    world.pc.x = pos.x;
    world.pc.y = pos.y;
}

static int32_t path_cmp(const void *key, const void *with)
{
    return ((path_t *)key)->cost - ((path_t *)with)->cost;
}

void dijkstra(map_t *m, pair_t from, pair_t to)
{
    // shortest path that a hiker and a rival can travel to get to the world.pc.
    // we need the world.pc's information.
    static path_t path[MAP_HEIGHT][MAP_WIDTH]; // maintain the value accross function calls.
    static uint32_t initialized = 0;
    static path_t *p;
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
    for (y = 0; y < MAP_HEIGHT; y++)
    { // iterate through all the nodes
        for (x = 0; x < MAP_WIDTH; x++)
        {
            path[y][x].cost = INT_MAX;
        }
    }
    heap_init(&h, path_cmp, NULL); // deleting things that are not dynamic is an error

    for (y = 1; y < MAP_HEIGHT - 1; y++)
    {
        for (x = 1; x < MAP_WIDTH - 1; x++)
        {
            path[y][x].hn = heap_insert(&h, &path[y][x]);
        }
    }
    while ((p = heap_remove_min(&h)))
    {
        p->hn = NULL;
        if ((p->pos[1] == to[1]) && (p->pos[0] == to[0]))
        {
            for (x = to[0], y = to[1]; x != from[0] || y != from[1]; p = &path[y][x],
                x = p->from[0], y = p->from[1])
            {
                // we don't want to overwrite the gates
                if (x != to[0] || y != to[1]) // ?
                {
                    mapxy(x, y) = '#'; // ?
                    // heightxy(x, y) = 0
                }
            }
            heap_delete(&h);
            return;
        }

        if ((path[p->pos[1] - 1][p->pos[0]].hn) && // is the north neighbor in the heap?
            (path[p->pos[1] - 1][p->pos[0]].cost > // copmparing the cost
             ((p->cost + heightpair(p->pos)) *             // then update the cost
              edge_penalty(p->pos[0], p->pos[1] - 1))))
        {
            path[p->pos[1] - 1][p->pos[0]].cost =
                ((p->cost + heightpair(p->pos)) *
                 edge_penalty(p->pos[0], p->pos[1] - 1));
            path[p->pos[1] - 1][p->pos[0]].from[1] = p->pos[1]; // might not need this
            path[p->pos[1] - 1][p->pos[0]].from[0] = p->pos[0]; // might not need this
            heap_decrease_key_no_replace(&h, path[p->pos[1] - 1]
                                                 [p->pos[0]]
                                                     .hn);
        }
        if ((path[p->pos[1]][p->pos[0] - 1].hn) &&
            (path[p->pos[1]][p->pos[0] - 1].cost >
             ((p->cost + heightpair(p->pos)) *
              edge_penalty(p->pos[0] - 1, p->pos[1]))))
        {
            path[p->pos[1]][p->pos[0] - 1].cost =
                ((p->cost + heightpair(p->pos)) *
                 edge_penalty(p->pos[0] - 1, p->pos[1]));
            path[p->pos[1]][p->pos[0] - 1].from[1] = p->pos[1];
            path[p->pos[1]][p->pos[0] - 1].from[0] = p->pos[0];
            heap_decrease_key_no_replace(&h, path[p->pos[1]]
                                                 [p->pos[0] - 1]
                                                     .hn);
        }
        if ((path[p->pos[1]][p->pos[0] + 1].hn) &&
            (path[p->pos[1]][p->pos[0] + 1].cost >
             ((p->cost + heightpair(p->pos)) *
              edge_penalty(p->pos[0] + 1, p->pos[1]))))
        {
            path[p->pos[1]][p->pos[0] + 1].cost =
                ((p->cost + heightpair(p->pos)) *
                 edge_penalty(p->pos[0] + 1, p->pos[1]));
            path[p->pos[1]][p->pos[0] + 1].from[1] = p->pos[1];
            path[p->pos[1]][p->pos[0] + 1].from[0] = p->pos[0];
            heap_decrease_key_no_replace(&h, path[p->pos[1]]
                                                 [p->pos[0] + 1]
                                                     .hn);
        }
        if ((path[p->pos[1] + 1][p->pos[0]].hn) &&
            (path[p->pos[1] + 1][p->pos[0]].cost >
             ((p->cost + heightpair(p->pos)) *
              edge_penalty(p->pos[0], p->pos[1] + 1))))
        {
            path[p->pos[1] + 1][p->pos[0]].cost =
                ((p->cost + heightpair(p->pos)) *
                 edge_penalty(p->pos[0], p->pos[1] + 1));
            path[p->pos[1] + 1][p->pos[0]].from[1] = p->pos[1];
            path[p->pos[1] + 1][p->pos[0]].from[0] = p->pos[0];
            heap_decrease_key_no_replace(&h, path[p->pos[1] + 1]
                                                 [p->pos[0]]
                                                     .hn);
        }
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

int main(int argc, char *argv[])
{
    srand(time(NULL));
    world_init();
    newMapCaller(); // This should automatically use world.curY and world.curX
    collectValidPositions(world.w[world.curY][world.curX]);
    placePlayer(world.w[world.curY][world.curX]); // place '@' on road, called once bc there is only one player in the world
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
