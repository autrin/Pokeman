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

void createSingleCenterOrMart(char **map, char building);
void newMapCaller(void);

char symbols[] = {'%', '^', ':', '.', '~'}; // Simplified symbols array

// Define a structure to represent a region
struct Region
{
    int32_t fromX, fromY, toX, toY;
    char symbol;
};

typedef struct world
{
    char *(*world[WORLD_HEIGHT][WORLD_WIDTH]); // This is now a pointer to a pointer to char //!
    int32_t curX;                              // x of the current map
    int32_t curY;                              // y of the current map
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
    world.curX = 200; // the starting point is the center, which is (200, 200) internally to us developers,
                      // but (0, 0) externally in the output.
    world.curY = 200;
    return 0;
}

// Function to create the border of the map
void createBorder(char **map)
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
void createMap(char **map, struct Region regions[NUM_REGIONS])
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

void createSingleCenterOrMart(char **map, char building)
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

void createCC(char **map)
{
    createSingleCenterOrMart(map, 'C');
}

void createPokemart(char **map)
{
    createSingleCenterOrMart(map, 'M');
}

void printMap(char **map)
{ // needs to accept a double pointer to char (char **map) since
  // it will be printing a dynamically allocated 2D array.
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            printf("%c", map[y][x]);
        }
        printf("\n");
    }
}

void sprinkle(char **map)
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
    world.curX = internalX;
    world.curY = internalY;

    // Check if a map exists at the new location
    if (world.world[world.curY][world.curX] == NULL)
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

void createPaths(char **map, int *topExit, int *leftExit, int *bottomExit, int *rightExit)
{
    // Initialize gate positions randomly if they're not set by adjacent maps
    if (*topExit == -1)
        *topExit = (rand() % (MAP_WIDTH - 7)) + 3;
    if (*leftExit == -1)
        *leftExit = (rand() % (MAP_HEIGHT - 6)) + 3;
    if (*bottomExit == -1)
        *bottomExit = (rand() % (MAP_WIDTH - 7)) + 3;
    if (*rightExit == -1)
        *rightExit = (rand() % (MAP_HEIGHT - 6)) + 3;

    int currentX = *topExit;
    // For vertical path
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        map[y][currentX] = '#';
        // Random deviation in the first half
        if (y < MAP_HEIGHT / 2)
        {
            int deviation = (rand() % 3) - 1; // Random deviation: -1, 0, 1
            currentX += deviation;
            currentX = currentX < 1 ? 1 : (currentX >= MAP_WIDTH - 1 ? MAP_WIDTH - 2 : currentX);
        }
        else
        {
            // Adjust direction towards bottomExit in the second half, ensuring no division by zero
            if (currentX != *bottomExit)
            {
                int direction = (*bottomExit - currentX) / abs(*bottomExit - currentX);
                currentX += direction;
                // Ensure currentX does not overshoot or undershoot the target exit
                if ((direction > 0 && currentX > *bottomExit) || (direction < 0 && currentX < *bottomExit))
                {
                    currentX = *bottomExit;
                }
            }
        }
    }

    int currentY = *leftExit;
    // For horizontal path
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        map[currentY][x] = '#';
        // Random deviation in the first half
        if (x < MAP_WIDTH / 2)
        {
            int deviation = (rand() % 3) - 1; // Random deviation: -1, 0, 1
            currentY += deviation;
            currentY = currentY < 1 ? 1 : (currentY >= MAP_HEIGHT - 1 ? MAP_HEIGHT - 2 : currentY);
        }
        else
        {
            // Adjust direction towards rightExit in the second half, ensuring no division by zero
            if (currentY != *rightExit)
            {
                int direction = (*rightExit - currentY) / abs(*rightExit - currentY);
                currentY += direction;
                // Ensure currentY does not overshoot or undershoot the target exit
                if ((direction > 0 && currentY > *rightExit) || (direction < 0 && currentY < *rightExit))
                {
                    currentY = *rightExit;
                }
            }
        }
    }
}

void newMapCaller()
{ // calls all of the functions neccessary to create a single map
    struct Region regions[NUM_REGIONS];
    // Only proceed if the map does not exist
    if (!world.world[world.curY][world.curX])
    {
        // Allocate memory for each row
        char **map = malloc(MAP_HEIGHT * sizeof(char *));//!
        for (int i = 0; i < MAP_HEIGHT; i++)
        {
            map[i] = malloc(MAP_WIDTH * sizeof(char));
            // Initialize the map's row here, if necessary
        }

        initializeRegions(regions);
        assignRegions(regions); // ? Is one universal regions enough??
        setRegionCoordinates(regions);
        createMap(map, regions); // add gates parameters

        int topExit = -1, leftExit = -1, bottomExit = -1, rightExit = -1;
        // Adjust gate positions based on existing neighboring maps
        // Top neighbor
        if (world.curY > 0 && world.world[world.curY - 1][world.curX])
        {
            char **topMap = world.world[world.curY - 1][world.curX];
            for (int x = 0; x < MAP_WIDTH; x++)
            {
                if (topMap[MAP_HEIGHT - 1][x] == '#')
                {
                    topExit = x; // error: operand of '*' must be a pointer but has type "int"C/C++(75)

                    break;
                }
            }
        }
        // Bottom neighbor
        if (world.curY < WORLD_HEIGHT - 1 && world.world[world.curY + 1][world.curX])
        {
            char **bottomMap = world.world[world.curY + 1][world.curX];
            for (int x = 0; x < MAP_WIDTH; x++)
            {
                if (bottomMap[0][x] == '#')
                {
                    bottomExit = x; // error: operand of '*' must be a pointer but has type "int"C/C++(75)

                    break;
                }
            }
        }

        // Left neighbor
        if (world.curX > 0 && world.world[world.curY][world.curX - 1])
        {
            char **leftMap = world.world[world.curY][world.curX - 1];
            for (int y = 0; y < MAP_HEIGHT; y++)
            {
                if (leftMap[y][MAP_WIDTH - 1] == '#')
                {
                    leftExit = y; // error: operand of '*' must be a pointer but has type "int"C/C++(75)
                    break;
                }
            }
        }

        // Right neighbor
        if (world.curX < WORLD_WIDTH - 1 && world.world[world.curY][world.curX + 1])
        {
            char **rightMap = world.world[world.curY][world.curX + 1];
            for (int y = 0; y < MAP_HEIGHT; y++)
            {
                if (rightMap[y][0] == '#')
                {
                    rightExit = y; // error: operand of '*' must be a pointer but has type "int"C/C++(75)
                    break;
                }
            }
        }

        createPaths(map, &topExit, &leftExit, &bottomExit, &rightExit);
        createBorder(map);

        int d = abs(world.curX - (WORLD_WIDTH / 2)) + abs(world.curY - (WORLD_HEIGHT / 2)); // Manhattan distance from the center

        // Calculate the probability of placing buildings based on the distance
        double probOfBuildings = d > 200 ? 5.0 : (50.0 - (45.0 * d) / 200.0);

        // Generate a Pokémon Center if a random number is below the calculated probability or if we're at the center of the world
        if ((rand() % 100) < probOfBuildings || !d)
        {                  // Using d == 0 to explicitly check for the center
            createCC(map); // Place a Pokémon Center
        }

        // Similarly, generate a Pokémart under the same conditions
        if ((rand() % 100) < probOfBuildings || !d)
        {
            createPokemart(map); // Place a Pokémart
        }

        sprinkle(map);
        // After generating the map, store the pointer in the world
        world.world[world.curY][world.curX] = map;
    }
}

void freeMap(int y, int x)//!
{
    if (world.world[y][x]) // (indicating that it has been allocated)
    {                              // to make sure we are not freeing a NULL pointer.
        for (int i = 0; i < MAP_HEIGHT; i++)
        {
            free(world.world[y][x][i]); // Free each row
        }
        free(world.world[y][x]);  // Free the array of row pointers
        world.world[y][x] = NULL; // Set the pointer to NULL after freeing
    }
}

void freeAllMaps()//!
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

    // input commands
    char c;

    do
    {
        if (world.world[world.curY][world.curX] != NULL)
        {
            printMap(world.world[world.curY][world.curX]);
            printf("(%d, %d)\n", world.curX - 200, world.curY - 200); // display the coordinates
        }
        else
        {
            printf("No map exists at this location.\n");
        }

        printf("Enter command: ");
        c = getchar(); // Read a single character command
        getchar();     // Consume the newline character after the command

        int fx, fy; // flying coordinates

        switch (c)
        {
        case 'q': // quit the game
            freeAllMaps();
            break;
        case 'f': // fly to the (x, y) coordinate
            printf("Enter coordiantes in this form: x y\n");
            scanf("%d %d", &fx, &fy);
            getchar(); // Consume the newline character after the coordinates
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
        }
    } while (c != 'q');

    return 0;
}
