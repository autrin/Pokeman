#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define MAP_WIDTH 80  // width of the map
#define MAP_HEIGHT 21 // height of the map

#define NUM_REGIONS 5 // Number of regions
char *symbols[] = {"BOULDER",
                   "TREE",
                   "TALL_GRASS",
                   "CLEARING",
                   "TERRAIN_WATER"};
// Define a structure to represent a region
struct Region
{
    int32_t fromX;
    int32_t fromY;
    int32_t toX;
    int32_t toY;
    int32_t part;
    char symbol; // Symbol to represent the region on the map
};

// Function to create the border of the map
void createBorder(char *map[MAP_HEIGHT][MAP_WIDTH])
{
    for (int i = 0; i < MAP_WIDTH; i++)
    {
        *map[0][i] = '%';
        *map[MAP_HEIGHT - 1][i] = '%';
    }
    for (int i = 0; i < MAP_HEIGHT; i++)
    {
        *map[i][0] = '%';
        *map[i][MAP_WIDTH - 1] = '%';
    }
}

// Function to initialize regions
void initializeRegions(struct Region *regions[])
{
    for (int i = 0; i < NUM_REGIONS; i++)
    {
        regions[i]->part = i + 1;
        regions[i]->symbol = ' '; // Initialize symbol as empty
    }
}

// Function to randomly assign regions to different parts
bool assignRegions(struct Region *regions[])
{
    for (int i = 0; i < NUM_REGIONS; i++)
    {
        int part = rand() % NUM_REGIONS + 1; // Generate a random part number
        regions[i]->part = part;
        regions[i]->symbol = *symbols[part];
        if (strcmp(symbols[part],"TALL_GRASS") == 0)
        {
            regions[i + 1]->symbol = *symbols[part]; // creating at least 2 of them
        }
        else if (strcmp(symbols[part], "CLEARING") == 0)
        {
            regions[i + 1]->symbol = *symbols[part]; // creating at least 2 of them
        }

        *symbols[part] = *symbols[part + 1]; // deleting it from the array so that it shows that we used it

        if (strcmp(symbols[part], &regions[i]->symbol) == 0)
        {
            printf("%s \n", "ERROR!!! FAILED TO REMOVE SYMBOL FROM symbols[] AFTER USING IT IN assignRegions()");
        }
    }
    // make sure all the symbols have been used in the map
    if (sizeof(symbols) / sizeof(char) != 0)
    {
        printf("%s", " ERROR!!! FAILED TO ASSIGN ALL OF THE REGION TYPE SYBMOLS IN assignRegions()");
        return false;
    }
}

// Function to set coordinates for each region
void setRegionCoordinates(struct Region *regions[])
{
    // Define the coordinates for each region
    regions[0]->fromX = 1;
    regions[0]->fromY = 1;
    regions[0]->toX = MAP_WIDTH / 3;
    regions[0]->toY = MAP_HEIGHT / 3;

    regions[1]->fromX = MAP_WIDTH / 3;
    regions[1]->fromY = 1;
    regions[1]->toX = 2 * MAP_WIDTH / 3;
    regions[1]->toY = MAP_HEIGHT / 3;

    regions[2]->fromX = 2 * MAP_WIDTH / 3;
    regions[2]->fromY = 1;
    regions[2]->toX = MAP_WIDTH - 2;
    regions[2]->toY = MAP_HEIGHT / 3;

    regions[3]->fromX = 1;
    regions[3]->fromY = MAP_HEIGHT / 3;
    regions[3]->toX = MAP_WIDTH / 3;
    regions[3]->toY = 2 * MAP_HEIGHT / 3;

    regions[4]->fromX = MAP_WIDTH / 3;
    regions[4]->fromY = MAP_HEIGHT / 3;
    regions[4]->toX = 2 * MAP_WIDTH / 3;
    regions[4]->toY = 2 * MAP_HEIGHT / 3;

    regions[5]->fromX = 2 * MAP_WIDTH / 3;
    regions[5]->fromY = MAP_HEIGHT / 3;
    regions[5]->toX = MAP_WIDTH - 2;
    regions[5]->toY = 2 * MAP_HEIGHT / 3;

    regions[6]->fromX = 1;
    regions[6]->fromY = 2 * MAP_HEIGHT / 3;
    regions[6]->toX = MAP_WIDTH / 3;
    regions[6]->toY = MAP_HEIGHT - 2;

    regions[7]->fromX = MAP_WIDTH / 3;
    regions[7]->fromY = 2 * MAP_HEIGHT / 3;
    regions[7]->toX = 2 * MAP_WIDTH / 3;
    regions[7]->toY = MAP_HEIGHT - 2;

    regions[8]->fromX = 2 * MAP_WIDTH / 3;
    regions[8]->fromY = 2 * MAP_HEIGHT / 3;
    regions[8]->toX = MAP_WIDTH - 2;
    regions[8]->toY = MAP_HEIGHT - 2;
}

// Function to create the map using region information
void createMap(char *map[MAP_HEIGHT][MAP_WIDTH], struct Region *regions[])
{
    // Fill the map with spaces
    for (int i = 0; i < MAP_HEIGHT; i++)
    {
        for (int j = 0; j < MAP_WIDTH; j++)
        {
            *map[i][j] = ' ';
        }
    }

    // Set coordinates and symbols for each region
    for (int i = 0; i < NUM_REGIONS; i++)
    {

        for (int x = regions[i]->fromX; x <= regions[i]->toX; x++)
        {
            for (int y = regions[i]->fromY; y <= regions[i]->toY; y++)
            {
                *map[y][x] = regions[i]->symbol;
            }
        }
    }
}

void craetePaths(char *map[MAP_HEIGHT][MAP_WIDTH])
{
    int exit1 = (rand() % MAP_WIDTH - 2) + 1; // creating the exits locations
    int exit2 = (rand() % MAP_WIDTH - 2) + 1;
    int exit3 = (rand() % MAP_HEIGHT - 2) + 1;
    int exit4 = (rand() % MAP_HEIGHT - 2) + 1;
    
    *map[exit3][exit1] = "#"; // placing the gates on the map
    *map[exit4][exit2] = "#";
    
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    char *map[MAP_HEIGHT][MAP_WIDTH]; // Representing the map

    // Create the border
    createBorder(&map);

    struct Region regions[NUM_REGIONS];

    // Initialize regions
    initializeRegions(&regions);

    // Randomly assign regions to different parts
    assignRegions(&regions);

    // TODO: check if assignRegions() returns true

    // Set coordinates and symbols for each region
    setRegionCoordinates(&regions);

    // Create the map using region information
    createMap(&map, &regions);

    craetePaths(&map);

    // fill out the map

    // Print the map

    return 0;
}
