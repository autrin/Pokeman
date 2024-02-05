#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define MAP_WIDTH 80     // width of the map
#define MAP_HEIGHT 21    // height of the map
#define NUM_REGIONS 5    // Number of regions
#define WORLD_HEIGHT 401 // world of all of the maps
#define WORLD_WIDTH 401

void createSingleCenterOrMart(char map[MAP_HEIGHT][MAP_WIDTH], char building);
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
    char *cur_map[MAP_HEIGHT][MAP_WIDTH];   // pointer to the current map
    int32_t curX;                           // x of the current map
    int32_t curY;                           // y of the current map
} world_t;

world_t world;

int world_init()
{ // initializing each map of the world to NULL
    for (int i = 0; i < 401; i++)
    {
        for (int j = 0; j < 401; j++)
        {
            world.world[j][i] = NULL;
        }
    }
    return 0;
}

// Function to create the border of the map
void createBorder(char map[MAP_HEIGHT][MAP_WIDTH])
{
    for (int i = 0; i < MAP_WIDTH; i++)
    {
        if (map[0][i] != '#')
        {
            map[0][i] = '%';
        }
        if (map[MAP_HEIGHT - 1][i] != '#')
        {
            map[MAP_HEIGHT - 1][i] = '%';
        }
    }
    for (int i = 0; i < MAP_HEIGHT; i++)
    {
        if (map[i][0] != '#')
        {
            map[i][0] = '%';
        }
        if (map[i][MAP_WIDTH - 1] != '#')
        {
            map[i][MAP_WIDTH - 1] = '%';
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

// Function to create the map using region information
void createMap(char map[MAP_HEIGHT][MAP_WIDTH], struct Region regions[NUM_REGIONS])
{
    // Initially fill the map with a default terrain to avoid empty spaces
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            map[y][x] = '.'; // Use '.' or another symbol as the default terrain
        }
    }

    // Then assign specific terrains based on regions
    for (int i = 0; i < NUM_REGIONS; i++)
    {
        for (int x = regions[i].fromX; x <= regions[i].toX; x++)
        {
            for (int y = regions[i].fromY; y <= regions[i].toY; y++)
            {
                map[y][x] = regions[i].symbol;
            }
        }
    }
}

void createPaths(char map[MAP_HEIGHT][MAP_WIDTH], int topExit, int leftExit)
{
    map[0][topExit] = '#';  // top exit
    map[leftExit][0] = '#'; // left exit

    // Create North-South path
    for (int y = 1; y < MAP_HEIGHT; y++)
    {
        map[y][topExit] = '#';
        if (y == MAP_HEIGHT - 1)
        {
            break; // to prevent having more than 1 gate on this side
        }
        // Random deviation for the path
        if (rand() % 5 == 0 && topExit > 2 && topExit < MAP_WIDTH - 4)
        {
            topExit--;             // Move path left
            map[y][topExit] = '#'; // to prevent diagonal paths and moves
                                   // You can comment it out to allow diagonal paths
        }
        else if (rand() % 3 == 0 && topExit < MAP_WIDTH - 5)
        {
            topExit++;             // Move path right
            map[y][topExit] = '#'; // to prevent diagonal paths and moves
        }
    }

    // Create East-West path
    for (int x = 1; x < MAP_WIDTH; x++)
    {
        map[leftExit][x] = '#';
        if (x == MAP_WIDTH - 1)
        {
            break; // to prevent having more than 1 gate on this side
        }
        // Random deviation for the path
        if (rand() % 5 == 0 && leftExit > 2 && leftExit < MAP_HEIGHT - 4)
        {
            leftExit--;             // Move path up
            map[leftExit][x] = '#'; // to prevent diagonal paths and moves. You can comment it to allow diagonal paths
        }
        else if (rand() % 3 == 0 && leftExit < MAP_HEIGHT - 5)
        {
            leftExit++;             // Move path down
            map[leftExit][x] = '#'; // to prevent diagonal paths and moves
        }
    }
}
void createSingleCenterOrMart(char map[MAP_HEIGHT][MAP_WIDTH], char building)
{
    while (true)
    {
        int xRand = (rand() % (MAP_WIDTH - 7)) + 3;
        int yRand = (rand() % (MAP_HEIGHT - 6)) + 3;
        // Check if the location is next to a path and is a clear spot
        if (map[yRand][xRand] == '.' &&
            (map[yRand - 1][xRand] == '#' || map[yRand + 1][xRand] == '#' ||
             map[yRand][xRand - 1] == '#' || map[yRand][xRand + 1] == '#'))
        {
            map[yRand][xRand] = building; // Place either a Pokémon Center ('C') or a Pokémart ('M')
            return;                       // Exit once placed
        }
    }
}

void createCC(char map[MAP_HEIGHT][MAP_WIDTH])
{
    createSingleCenterOrMart(map, 'C');
}

void createPokemart(char map[MAP_HEIGHT][MAP_WIDTH])
{
    createSingleCenterOrMart(map, 'M');
}

void printMap(char map[MAP_HEIGHT][MAP_WIDTH])
{
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            printf("%c", map[y][x]);
        }
        printf("\n");
    }
}

void sprinkle(char map[MAP_HEIGHT][MAP_WIDTH])
{

    for (int i = 0; i < 50; i++)
    {
        int y1 = (rand() % (MAP_HEIGHT - 6)) + 3;
        int x1 = (rand() % (MAP_WIDTH - 7)) + 3;
        int y2 = (rand() % (MAP_HEIGHT - 6)) + 3;
        int x2 = (rand() % (MAP_WIDTH - 7)) + 3;
        if (map[y1][x1] != '#' && map[y1][x1] != 'C' && map[y1][x1] != 'M')
        {
            map[y1][x1] = '^';
        }
        if (map[y2][x2] != '#' && map[y2][x2] != 'C' && map[y2][x2] != 'M' && (x2 % 7) == 0) // lower the possibility of adding more %
        {
            map[y2][x2] = '%';
        }
    }
}

void fly(int newX, int newY, char *world[WORLD_HEIGHT][WORLD_WIDTH])
{
    // need to call createMap()
    // world[y][x] = createMap()
}

void newMapCaller()
{ // calls all of the functions neccessary to create a single map
    struct Region regions[NUM_REGIONS];
    // check if the map exits
    if (world.world[world.curY][world.curX])
    {
        //...
        // world.world
    }
    initializeRegions(regions);
    assignRegions(regions);
    setRegionCoordinates(regions);
    createMap(world.cur_map, regions); // add gates parameters
    // Correctly position exits within map borders
    // For the top gate, the range is from 3 to 76 (total 74 positions)
    // We subtract 7 from MAP_WIDTH (76 - 3 + 1 = 74) and then add 3 to the result
    int topExit = (rand() % (MAP_WIDTH - 7)) + 3;
    // For the left gate, the range is from 3 to 17 (total 15 positions)
    // We subtract 6 from MAP_HEIGHT (17 - 3 + 1 = 15) and then add 3 to the result
    int leftExit = (rand() % (MAP_HEIGHT - 6)) + 3;
    createPaths(world.cur_map, topExit, leftExit);
    createBorder(world.cur_map); // Ensure borders are created last
    int d = abs(world.curX - (WORLD_WIDTH / 2)) + abs(world.curY - (WORLD_HEIGHT / 2));
    int probOfBuildings = d > 200 ? 5 : (((-45 * d) / 200) + 50) / 100; // the probablity of having pokeman centers and pokemarts
                                                                        // it will be 5 (small number) if the manhattan distance is bigger than 200
    if (probOfBuildings > rand() % 100 || !d)
    { // or if d is 0 because the first map in the center of the world must have the buildings
        createCC(world.cur_map);
    }
    if (probOfBuildings > rand() % 100 || !d)
    {
        createPokemart(world.cur_map);
    }
    sprinkle(world.cur_map);
    // add the current map to the array of pointers, world
    // world.world =
    printMap(world.cur_map);
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    // char map[MAP_HEIGHT][MAP_WIDTH];

    // input commands
    char c;
    char fx, fy; // flying coordinates
    // int x = 200; // the starting coordinate
    // int y = 200; // the starting coordinate
    do
    {
        newMapCaller();
        // printCoordinates();
        printf("\n");
        printf("(%d, %d)", world.curX - 200, world.curY - 200); // display the coordinates
        switch (c)
        {
        case 'q': // quit the game
            exit(0);
            // break;
        case 'f': // fly to the (x, y) coordinate
            scanf("%d %d", &fx, &fy);
            fly(fx, fy, world.world);
            // break;
        case 'n':                  // move to the north map
            if (world.curY-- >= 0) // don't be out of bounds
            {
                world.curY--; // even if the map exits, you still want to move there
                newMapCaller();
                // move to the n map and display
            }
            // break;
        case 's': // move to the south map
            if (world.curY-- <= WORLD_HEIGHT - 1)
            {
                world.curY++;
                // move to the s map and display
            }
            // break;
        case 'w': // move to the west map
            if (world.curX-- >= 0)
            {
                // move to the w map and display
                world.curX--;
            }
            // break;
        case 'e': // move to the east map
            if (world.curX <= WORLD_WIDTH - 1)
            {
                // move to the e map and display
                world.curX++;
            }
            // break;
        }
    } while ((c = getc(stdin)));

    return 0;
}
