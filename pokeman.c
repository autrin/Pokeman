#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define MAP_WIDTH 80  // width of the map
#define MAP_HEIGHT 21 // height of the map
#define NUM_REGIONS 5 // Number of regions

char symbols[] = {'%', '^', ':', '.', '~'}; // Simplified symbols array
// Define a structure to represent a region
struct Region
{
    int32_t fromX, fromY, toX, toY;
    char symbol;
};

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

// Function to randomly assign regions to different parts
// bool assignRegions(struct Region regions[NUM_REGIONS]) {
//     bool usedSymbols[NUM_REGIONS] = {false}; // Track used symbols

//     for (int i = 0; i < NUM_REGIONS; i++) {
//         int part = rand() % NUM_REGIONS;
//         regions[i].symbol = symbols[part];
//         usedSymbols[part] = true; // Mark this symbol as used

//         // Ensure 'TALL_GRASS' and 'CLEARING' appear at least twice
//         if (symbols[part] == ':' && i < NUM_REGIONS - 1) {
//             regions[i + 1].symbol = symbols[part];
//             i++; // Increment i to avoid overriding next region
//         }
//     }

//     // Check if all symbols were used
//     for (int i = 0; i < NUM_REGIONS; i++) {
//         if (!usedSymbols[i]) {
//             printf("ERROR: Not all region types were used!\n");
//             return false;
//         }
//     }

//     return true;
// }
// void assignRegions(struct Region regions[NUM_REGIONS])
// {
    // Pre-assign required symbols
    // int assignedCount = 0;
    // regions[assignedCount++].symbol = '%'; // BOULDER
    // regions[assignedCount++].symbol = '^'; // TREE
    // regions[assignedCount++].symbol = ':'; // TALL_GRASS
    // regions[assignedCount++].symbol = ':'; // Additional TALL_GRASS
    // regions[assignedCount++].symbol = '.'; // CLEARING
    // regions[assignedCount++].symbol = '.'; // Additional CLEARING
    // regions[assignedCount++].symbol = '~'; // TERRAIN_WATER

    // // Randomly assign remaining regions
    // for (int i = assignedCount; i < NUM_REGIONS; i++)
    // {
    //     int part = rand() % NUM_REGIONS;
    //     regions[i].symbol = symbols[part];
    // }
// }

void assignRegions(struct Region regions[NUM_REGIONS]) {
    // Ensuring that each region type is used
    int usedSymbols[NUM_REGIONS] = {0};

    // Initially assign a unique symbol to each region to ensure coverage
    for (int i = 0; i < NUM_REGIONS; i++) {
        regions[i].symbol = symbols[i % 5];
        // usedSymbols[regions[i].symbol - '%']++;
        switch (regions[i].symbol) {
            case '%': usedSymbols[0]++; break;
            case '^': usedSymbols[1]++; break;
            case ':': usedSymbols[2]++; break;
            case '.': usedSymbols[3]++; break;
            case '~': usedSymbols[4]++; break;
        }
    }
    int firstWater = rand() % NUM_REGIONS;
    regions[firstWater].symbol = '~';
    usedSymbols[4]++;

    int secondWater = rand() % NUM_REGIONS;
    regions[secondWater].symbol = '~';
    usedSymbols[4]++;

    int firstTallGrass = rand() % NUM_REGIONS;
    regions[firstTallGrass].symbol = ':';
    usedSymbols[2]++;

    int secondTallGrass = rand() % NUM_REGIONS;
    regions[secondTallGrass].symbol = ':';
    usedSymbols[2]++;


    // Additional assignments to meet specific terrain requirements
    for (int i = 0; i < NUM_REGIONS; i++) {
        int p = rand();
        // here you can add conditions to adjust the distribution
        // e.g.: Increase the probability of water or grass
        if (p % 2 == 0 && regions[i].symbol == ' ') {  // Example condition, adjust as needed
            regions[i].symbol = '%'; // Assigning more tall grass
            usedSymbols[0]++; 
            /*This line increments the count for the terrain symbol ':' in the usedSymbols array. 
            It effectively says, "Find the index corresponding to the symbol ':', and increase the count at that index by 1.*/
        } else if (p % 2 != 0 && regions[i].symbol == ' ') {  // Example condition
            regions[i].symbol = '%'; // Assigning more water
            usedSymbols[0]++;
        }
        // similar conditions for other terrains as needed
    }

    // Print terrain distribution for debugging
    printf("Terrain Distribution:\n");
    for (int i = 0; i < 5; i++) {
        printf("%c: %d\n", symbols[i], usedSymbols[i]);
    }
}


// Function to set coordinates for each region
void setRegionCoordinates(struct Region regions[NUM_REGIONS])
{
    // Define the coordinates for each region
    // regions[0].fromX = 1;
    // regions[0].fromY = 1;
    // regions[0].toX = MAP_WIDTH / 3;
    // regions[0].toY = MAP_HEIGHT / 3;

    // regions[1].fromX = MAP_WIDTH / 3;
    // regions[1].fromY = 1;
    // regions[1].toX = 2 * MAP_WIDTH / 3;
    // regions[1].toY = MAP_HEIGHT / 3;

    // regions[2].fromX = 2 * MAP_WIDTH / 3;
    // regions[2].fromY = 1;
    // regions[2].toX = MAP_WIDTH - 2;
    // regions[2].toY = MAP_HEIGHT / 3;

    // regions[3].fromX = 1;
    // regions[3].fromY = MAP_HEIGHT / 3;
    // regions[3].toX = MAP_WIDTH / 3;
    // regions[3].toY = 2 * MAP_HEIGHT / 3;

    // regions[4].fromX = MAP_WIDTH / 3;
    // regions[4].fromY = MAP_HEIGHT / 3;
    // regions[4].toX = 2 * MAP_WIDTH / 3;
    // regions[4].toY = 2 * MAP_HEIGHT / 3;

    // regions[5].fromX = 2 * MAP_WIDTH / 3;
    // regions[5].fromY = MAP_HEIGHT / 3;
    // regions[5].toX = MAP_WIDTH - 2;
    // regions[5].toY = 2 * MAP_HEIGHT / 3;

    // regions[6].fromX = 1;
    // regions[6].fromY = 2 * MAP_HEIGHT / 3;
    // regions[6].toX = MAP_WIDTH / 3;
    // regions[6].toY = MAP_HEIGHT - 2;

    // regions[7].fromX = MAP_WIDTH / 3;
    // regions[7].fromY = 2 * MAP_HEIGHT / 3;
    // regions[7].toX = 2 * MAP_WIDTH / 3;
    // regions[7].toY = MAP_HEIGHT - 2;

    // regions[8].fromX = 2 * MAP_WIDTH / 3;
    // regions[8].fromY = 2 * MAP_HEIGHT / 3;
    // regions[8].toX = MAP_WIDTH - 2;
    // regions[8].toY = MAP_HEIGHT - 2;

    for (int i = 0; i < NUM_REGIONS; i++) {
        regions[i].fromX = (i % 3) * (MAP_WIDTH / 3);
        regions[i].toX = ((i % 3) + 1) * (MAP_WIDTH / 3) - 1;
        regions[i].fromY = (i / 3) * (MAP_HEIGHT / NUM_REGIONS);
        regions[i].toY = ((i / 3) + 1) * (MAP_HEIGHT / NUM_REGIONS) - 1;
    }
}

// Function to create the map using region information
void createMap(char map[MAP_HEIGHT][MAP_WIDTH], struct Region regions[NUM_REGIONS]) {
    // Initially fill the map with a default terrain to avoid empty spaces
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            map[y][x] = '.'; // Use '.' or another symbol as the default terrain
        }
    }

    // Then assign specific terrains based on regions
    for (int i = 0; i < NUM_REGIONS; i++) {
        for (int x = regions[i].fromX; x <= regions[i].toX; x++) {
            for (int y = regions[i].fromY; y <= regions[i].toY; y++) {
                map[y][x] = regions[i].symbol;
            }
        }
    }
}


void createPaths(char map[MAP_HEIGHT][MAP_WIDTH])
{
    // Correctly position exits within map borders
    int topExit = (rand() % (MAP_WIDTH - 4)) + 2;  // Top exit
    int leftExit = (rand() % (MAP_HEIGHT - 4)) + 2; // Left exit

    // Set exits
    map[0][topExit] = '#'; // top exit
    map[leftExit][0] = '#'; // left exit

    // Create North-South path
    for (int y = 1; y < MAP_HEIGHT; y++)
    {
        map[y][topExit] = '#';
        if(y == MAP_HEIGHT - 1){
            break; // to prevent having more than 1 gate
        }
        // Random deviation for the path
        if (rand() % 5 == 0 && topExit > 1 && topExit < MAP_WIDTH - 5)
        {
            topExit--; // Move path left
            map[y][topExit] = '#'; // to prevent diagonal paths and moves
                                 //You can comment it out to allow diagonal paths
        }
        else if (rand() % 3 == 0 && topExit < MAP_WIDTH - 5)
        {
            topExit++; // Move path right
            map[y][topExit] = '#';// to prevent diagonal paths and moves
        }
    }

    // Create East-West path
    for (int x = 1; x < MAP_WIDTH; x++)
    {
        map[leftExit][x] = '#';
        if(x == MAP_WIDTH - 1){ 
            break; // to prevent having more than 1 gate
        }
        // Random deviation for the path
        if (rand() % 5 == 0 && leftExit > 1 && leftExit < MAP_HEIGHT - 5)
        {
            leftExit--; // Move path up
            map[leftExit][x] = '#';// to prevent diagonal paths and moves. You can comment it to allow diagonal paths
        }
        else if (rand() % 3 == 0 && leftExit < MAP_HEIGHT - 5)
        {
            leftExit++; // Move path down
            map[leftExit][x] = '#';// to prevent diagonal paths and moves
        }
    }
}

// Function to create a Pokeman Center
void createCC(char map[MAP_HEIGHT][MAP_WIDTH]){
    // find the gate first to find the paths faster
    for(int i = 0; i < MAP_WIDTH; i++){
        for(int j = 0; j < MAP_HEIGHT; j ++){
            if(map[j][i] == '#'){
                
            }
        }
    }

}

// Function to create a Pokemart



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

int main(int argc, char *argv[])
{
    srand(time(NULL));
    char map[MAP_HEIGHT][MAP_WIDTH];
    struct Region regions[NUM_REGIONS];

    initializeRegions(regions);
    assignRegions(regions);
    //     return 1; // Error handling
    // }
    setRegionCoordinates(regions);
    createMap(map, regions);
    createPaths(map);
    createBorder(map); // Ensure borders are created last
    createCC(map);
    printMap(map);

    return 0;
}
