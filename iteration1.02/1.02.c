#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define MAP_WIDTH 80     // width of the world.curMap
#define MAP_HEIGHT 21    // height of the world.curMap
#define NUM_REGIONS 5    // Number of regions
#define WORLD_HEIGHT 401 // world of all of the maps
#define WORLD_WIDTH 401

void createSingleCenterOrMart(char building);
char symbols[] = {'%', '^', ':', '.', '~'}; // Simplified symbols array
// typedef struct maps map_t;

// Define a structure to represent a region
struct Region
{
    int32_t fromX, fromY, toX, toY;
    char symbol;
};
// struct maps
// {
//     // map_t *world[401][401];
//     int x, y;
// };

typedef struct world
{
    char *world[WORLD_HEIGHT][WORLD_WIDTH]; // holds all of the maps
    char curMap[MAP_HEIGHT][MAP_WIDTH];    // pointer to the current world.curMap
    int32_t curX;                           // x of the current world.curMap
    int32_t curY;                           // y of the current world.curMap
} world_t;

world_t world;

int world_init()
{ // initializing each world.curMap of the world to NULL
    for (int i = 0; i < 401; i++)
    {
        for (int j = 0; j < 401; j++)
        {
            world.world[j][i] = NULL;
        }
    }
    return 0;
}

// Function to create the border of the world.curMap
void createBorder()
{
    for (int i = 0; i < MAP_WIDTH; i++)
    {
        if (world.curMap[0][i] != '#')
        {
            world.curMap[0][i] = '%';
        }
        if (world.curMap[MAP_HEIGHT - 1][i] != '#')
        {
            world.curMap[MAP_HEIGHT - 1][i] = '%';
        }
    }
    for (int i = 0; i < MAP_HEIGHT; i++)
    {
        if (world.curMap[i][0] != '#')
        {
            world.curMap[i][0] = '%';
        }
        if (world.curMap[i][MAP_WIDTH - 1] != '#')
        {
            world.curMap[i][MAP_WIDTH - 1] = '%';
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
    for (int i = 0; i < NUM_REGIONS + 1; i++)
    {
        regions[i].fromX = (i % 3) * (MAP_WIDTH / 3);
        regions[i].toX = ((i % 3) + 1) * (MAP_WIDTH / 3) - 1;
        regions[i].fromY = (i / 3) * (MAP_HEIGHT / ((NUM_REGIONS + 1) / 3)); // Dividing by 2 (NUM_REGIONS/3) for a 3x2 grid
        regions[i].toY = ((i / 3) + 1) * (MAP_HEIGHT / ((NUM_REGIONS + 1) / 3)) - 1;
    }
}

// Function to create the world.curMap using region information
void createMap(struct Region regions[NUM_REGIONS])
{
    // Initially fill the world.curMap with a default terrain to avoid empty spaces
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            world.curMap[y][x] = '.'; // Use '.' or another symbol as the default terrain
        }
    }

    // Then assign specific terrains based on regions
    for (int i = 0; i < NUM_REGIONS; i++)
    {
        for (int x = regions[i].fromX; x <= regions[i].toX; x++)
        {
            for (int y = regions[i].fromY; y <= regions[i].toY; y++)
            {
                world.curMap[y][x] = regions[i].symbol;
            }
        }
    }
}

void createPaths(int topExit, int leftExit)
{
    world.curMap[0][topExit] = '#';  // top exit
    world.curMap[leftExit][0] = '#'; // left exit

    // Create North-South path
    for (int y = 1; y < MAP_HEIGHT; y++)
    {
        world.curMap[y][topExit] = '#';
        if (y == MAP_HEIGHT - 1)
        {
            break; // to prevent having more than 1 gate on this side
        }
        // Random deviation for the path
        if (rand() % 5 == 0 && topExit > 2 && topExit < MAP_WIDTH - 4)
        {
            topExit--;             // Move path left
            world.curMap[y][topExit] = '#'; // to prevent diagonal paths and moves
                                   // You can comment it out to allow diagonal paths
        }
        else if (rand() % 3 == 0 && topExit < MAP_WIDTH - 5)
        {
            topExit++;             // Move path right
            world.curMap[y][topExit] = '#'; // to prevent diagonal paths and moves
        }
    }

    // Create East-West path
    for (int x = 1; x < MAP_WIDTH; x++)
    {
        world.curMap[leftExit][x] = '#';
        if (x == MAP_WIDTH - 1)
        {
            break; // to prevent having more than 1 gate on this side
        }
        // Random deviation for the path
        if (rand() % 5 == 0 && leftExit > 2 && leftExit < MAP_HEIGHT - 4)
        {
            leftExit--;             // Move path up
            world.curMap[leftExit][x] = '#'; // to prevent diagonal paths and moves. You can comment it to allow diagonal paths
        }
        else if (rand() % 3 == 0 && leftExit < MAP_HEIGHT - 5)
        {
            leftExit++;             // Move path down
            world.curMap[leftExit][x] = '#'; // to prevent diagonal paths and moves
        }
    }

    // Now check for gates on the edge of the world and eliminate them.
    if (world.curY == 0)
    { // we're at the top of the world (row = 0), there are not gates at the top of the current world.curMap
        for (int i = 0; i < MAP_WIDTH; i++)
        {
            if (rand() % 3 == 0)
            {
                world.curMap[0][i] = '%';
            }
            else
            {
                world.curMap[0][i] = '^';
            }
        }
    }
    if (world.curY == WORLD_HEIGHT - 1)
    { // we're at the bottom of the world, no gates on the bottom side of the current world.curMap
        for (int i = 0; i < MAP_WIDTH; i++)
        {
            if (rand() % 3 == 0)
            {
                world.curMap[WORLD_HEIGHT - 1][i] = '%';
            }
            else
            {
                world.curMap[WORLD_HEIGHT - 1][i] = '^';
            }
        }
    }
    if (world.curX == 0)
    { // we're on the left side of the world, no gates on the left side of the current world.curMap
        for (int i = 0; i < MAP_HEIGHT; i++)
        {
            if (rand() % 3 == 0)
            {
                world.curMap[i][0] = '%';
            }
            else
            {
                world.curMap[i][0] = '^';
            }
        }
    }
    if (world.curX == WORLD_WIDTH - 1)
    { // we're on the right side of the world, no gates on the right side of the current world.curMap
        for (int i = 0; i < MAP_HEIGHT; i++)
        {
            if (rand() % 3 == 0)
            {
                world.curMap[i][WORLD_WIDTH - 1] = '%';
            }
            else
            {
                world.curMap[i][WORLD_WIDTH - 1] = '^';
            }
        }
    }
}

void createSingleCenterOrMart(char building)
{
    while (true)
    {
        int xRand = (rand() % (MAP_WIDTH - 7)) + 3;
        int yRand = (rand() % (MAP_HEIGHT - 6)) + 3;
        // Check if the location is next to a path and is a clear spot
        if (world.curMap[yRand][xRand] == '.' &&
            (world.curMap[yRand - 1][xRand] == '#' || world.curMap[yRand + 1][xRand] == '#' ||
             world.curMap[yRand][xRand - 1] == '#' || world.curMap[yRand][xRand + 1] == '#'))
        {
            world.curMap[yRand][xRand] = building; // Place either a Pokémon Center ('C') or a Pokémart ('M')
            return;                       // Exit once placed
        }
    }
}

void createCC()
{
    createSingleCenterOrMart('C');
}

void createPokemart()
{
    createSingleCenterOrMart('M');
}

void printMap()
{
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            printf("%c", world.curMap[y][x]);
        }
        printf("\n");
    }
}

void sprinkle()
{

    for (int i = 0; i < 50; i++)
    {
        int y1 = (rand() % (MAP_HEIGHT - 6)) + 3;
        int x1 = (rand() % (MAP_WIDTH - 7)) + 3;
        int y2 = (rand() % (MAP_HEIGHT - 6)) + 3;
        int x2 = (rand() % (MAP_WIDTH - 7)) + 3;
        if (world.curMap[y1][x1] != '#' && world.curMap[y1][x1] != 'C' && world.curMap[y1][x1] != 'M')
        {
            world.curMap[y1][x1] = '^';
        }
        if (world.curMap[y2][x2] != '#' && world.curMap[y2][x2] != 'C' && world.curMap[y2][x2] != 'M' && (x2 % 7) == 0) // lower the possibility of adding more %
        {
            world.curMap[y2][x2] = '%';
        }
    }
}

void fly(int newX, int newY)
{
    // need to call createMap()
    // world[y][x] = createMap()
}

void newMapCaller()
{ // calls all of the functions neccessary to create a single world.curMap
    struct Region regions[NUM_REGIONS];
    // check if the world.curMap exits
    if (world.world[world.curY][world.curX]) // the world.curMap already exists
    {  // !
        // TODO
        // char *tmp;
        // tmp = world.curMap;
        // world.world[world.curY][world.curX] = world.curMap;
        world.curMap = world.world[world.curY][world.curX];
        // world.curMap[0][0] = malloc(sizeof(char) * world.curY );
        return; //? maybe???
    }
    
    initializeRegions(regions);
    assignRegions(regions); // ? Is one universal regions enough??
    setRegionCoordinates(regions);
    createMap(regions); // add gates parameters
    // Correctly position exits within world.curMap borders
    // For the top gate, the range is from 3 to 76 (total 74 positions)
    // We subtract 7 from MAP_WIDTH (76 - 3 + 1 = 74) and then add 3 to the result
    int topExit = (rand() % (MAP_WIDTH - 7)) + 3;
    // For the left gate, the range is from 3 to 17 (total 15 positions)
    // We subtract 6 from MAP_HEIGHT (17 - 3 + 1 = 15) and then add 3 to the result
    int leftExit = (rand() % (MAP_HEIGHT - 6)) + 3;
    createPaths(topExit, leftExit);
    createBorder(); // Ensure borders are created last
    int d = abs(world.curX - (WORLD_WIDTH / 2)) + abs(world.curY - (WORLD_HEIGHT / 2)); // manhattan distance
    int probOfBuildings = d > 200 ? 5 : (((-45 * d) / 200) + 50) / 100; // the probablity of having pokeman centers and pokemarts
                                                                        // it will be 5 (small number) if the manhattan distance is bigger than 200
    if (probOfBuildings > rand() % 100 || !d)
    { // or if d is 0 because the first world.curMap in the center of the world must have the buildings
        createCC();
    }
    if (probOfBuildings > rand() % 100 || !d)
    {
        createPokemart();
    }
    sprinkle();
    // add the current world.curMap to the array of pointers, world
    world.world[world.curY][world.curX] = world.curMap;
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    // char world.curMap[MAP_HEIGHT][MAP_WIDTH];
    world_init();
    // input commands
    char c;
    char fx, fy; // flying coordinates
    // int x = 200; // the starting coordinate
    // int y = 200; // the starting coordinate
    do
    {
        newMapCaller();
        // printCoordinates();
        printMap();
        printf("\n");
        printf("(%d, %d)", world.curX - 200, world.curY - 200); // display the coordinates
        switch (c)
        {
        case 'q': // quit the game
            exit(0);
            // break;
        case 'f': // fly to the (x, y) coordinate
            scanf("%d %d", &fx, &fy);
            fly(fx, fy);
            // break;
        case 'n':                  // move to the north world.curMap
            if (world.curY-- >= 0) // don't be out of bounds
            {
                world.curY--; // even if the world.curMap exits, you still want to move there
                newMapCaller();
                // move to the n world.curMap and display
            }
            // break;
        case 's': // move to the south world.curMap
            if (world.curY-- <= WORLD_HEIGHT - 1)
            {
                world.curY++;
                // move to the s world.curMap and display
            }
            // break;
        case 'w': // move to the west world.curMap
            if (world.curX-- >= 0)
            {
                // move to the w world.curMap and display
                world.curX--;
            }
            // break;
        case 'e': // move to the east world.curMap
            if (world.curX <= WORLD_WIDTH - 1)
            {
                // move to the e world.curMap and display
                world.curX++;
            }
            // break;
        }
    } while ((c = getc(stdin)));

    return 0;
}
