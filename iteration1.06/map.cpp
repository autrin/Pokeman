
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
class World world;
void world_init() {
    int i, j;
    world.x = WORLD_WIDTH / 2;
    world.y = WORLD_HEIGHT / 2;
    for (i = 0; i < WORLD_HEIGHT; i++) {
        for (j = 0; j < WORLD_WIDTH; j++) {
            world.w[i][j] = NULL;
        }
    }
    world.global_sequence_number = 0;
    // world.w[world.y][world.x]->npc_count = 0;
}


// Function to create the border of the map
void createBorder(map* m)
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

void generateTerrainWithNoise(map* m) {
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
void createMap(map* m, struct Region regions[NUM_REGIONS])
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

void createSingleCenterOrMart(map* m, char building)
{
    if (validPositionsCount == 0)
        return; // No valid positions available
    int idx;
    Position pos;
    while (true) {
        idx = rand() % validPositionsCount;
        pos = validPositionsForBuildings[idx];

        // Example logic to place building next to selected path_tposition
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

void createCC(map* m)
{
    createSingleCenterOrMart(m, 'C');
}

void createPokemart(map* m)
{
    createSingleCenterOrMart(m, 'M');
}

void printMap(map* m)
{
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            if (world.w[world.y][world.x]->npcs[y][x]) {
                printf("%c", world.w[world.y][world.x]->npcs[y][x]->symbol);
            }
            else {
                printf("%c", m->m[y][x]);
            }
        }
        printf("\n");
    }
}

void sprinkle(map* m)
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

void createPaths(map* m)
//! The issue with north (either first map or all of them) is that I think the gate does not exist on the first map
// ! But I think they exist because of the if statements. Anyways fix the bottom gates.
{
    int topExit = -1, leftExit = -1, bottomExit = -1, rightExit = -1;

    // Correctly align top and bottom exits with adjacent maps if they exist
    if (world.y > 0 && world.w[world.y - 1][world.x])
    {
        // Align top exit with the bottom exit of the map above
        topExit = world.w[world.y - 1][world.x]->bottomExit;
    }
    if (world.y < WORLD_HEIGHT - 1 && world.w[world.y + 1][world.x])
    {
        // Preemptively align bottom exit with the top exit of the map below
        bottomExit = world.w[world.y + 1][world.x]->topExit;
    }
    // Left neighbor
    if (world.x > 0 && world.w[world.y][world.x - 1])
    {

        leftExit = world.w[world.y][world.x - 1]->rightExit;
    }
    // Right neighbor
    if (world.x < WORLD_WIDTH - 1 && world.w[world.y][world.x + 1])
    {

        rightExit = world.w[world.y][world.x + 1]->leftExit;
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


void freeAllMaps()
{
    // free_npcs();
    for (int y = 0; y < WORLD_HEIGHT; y++)
    {
        for (int x = 0; x < WORLD_WIDTH; x++)
        {
            if (world.w[y][x])
            {
                heap_delete(&world.w[y][x]->event_heap);
                delete world.w[y][x];
                // free(world.w[y][x]);
                world.w[y][x] = NULL;
            }
        }
    }
}
/* calls all of the functions neccessary to create a single map */
void newMapCaller(int moveMap)
{
    if (!world.w[world.y][world.x])
    {
        struct Region regions[NUM_REGIONS];
        world.w[world.y][world.x] = (map*)malloc(sizeof(map)); //!
        // world.w[world.y][world.x] = new map;
        initializeRegions(regions);
        assignRegions(regions);
        setRegionCoordinates(regions);
        createMap(world.w[world.y][world.x], regions); // add gates parameters
        generateTerrainWithNoise(world.w[world.y][world.x]);
        createPaths(world.w[world.y][world.x]);
        // collectValidPositions(world.w[world.y][world.x]); // Done in place to make it faster
        createBorder(world.w[world.y][world.x]);

        int d = abs(world.x - (WORLD_WIDTH / 2)) + abs(world.y - (WORLD_HEIGHT / 2)); // Manhattan distance from the center

        // Calculate the probability of placing buildings based on the distance
        double probOfBuildings = d > 200 ? 5.0 : (50.0 - (45.0 * d) / 200.0);

        // Generate a Pokémon Center if a random number is below the calculated probability or if we're at the center of the world
        if ((rand() % 100) < probOfBuildings || !d)
        {                                              // Using d == 0 to explicitly check for the center
            createCC(world.w[world.y][world.x]); // Place a Pokémon Center
        }

        // Similarly, generate a Pokémart under the same conditions
        if ((rand() % 100) < probOfBuildings || !d)
        {
            createPokemart(world.w[world.y][world.x]); // Place a Pokémart
        }
        sprinkle(world.w[world.y][world.x]);
        // After generating the map, store the pointer in the world
        // *world.w[world.y][world.x] = world.w[world.y][world.x]->m;
        // heap_init(&world.w[world.y][world.x]->event_heap, characters_turn_comp, cleanup_characters);
        collectValidPositions(world.w[world.y][world.x]);
        // placePlayer(world.w[world.y][world.x]); // place '@' on road, called once bc there is only one player in the world
        world.w[world.y][world.x]->npc_count = 0;
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            for (int x = 0; x < MAP_WIDTH; x++)
            {
                world.w[world.y][world.x]->npcs[y][x] = NULL;
            }
        }
        heap_init(&world.w[world.y][world.x]->event_heap, characters_turn_comp, NULL);
        dijkstra(world.w[world.y][world.x]);
        if (world.y == WORLD_HEIGHT / 2 && world.x == WORLD_WIDTH / 2) {
            character* pc = create_pc(world.w[world.y][world.x]);
            world.pc = *pc;
            world.pc.heap_node = heap_insert(&world.w[world.y][world.x]->event_heap, &world.pc);
        }
        else {
            // fix the position of the pc moving through the gates
            if (world.pc.y == 1) {
                world.pc.y = MAP_HEIGHT - 2;
            }
            else if (world.pc.y == MAP_HEIGHT - 2) {
                world.pc.y = 1;
            }
            else if (world.pc.x == 1) {
                world.pc.x = MAP_WIDTH - 2;
            }
            else if (world.pc.x == MAP_WIDTH - 2) {
                world.pc.x = 1;
            }
            world.w[world.y][world.x]->npcs[world.pc.y][world.pc.x] = &world.pc;
            character* cur_character;
            if ((cur_character = (character*)heap_peek_min(&world.w[world.y][world.x]->event_heap))) {
                world.pc.next_turn = cur_character->next_turn;
            }
            else {
                world.pc.next_turn = 0;
            }
        }
        if (moveMap) {
            do {
                world.w[world.y][world.x]->npcs[world.pc.y][world.pc.x] = NULL;
                // Position pos = find_valid_position_for_npc(PC);
                world.pc.x = rand() % ((MAP_WIDTH - 2)) + 1;
                world.pc.y = rand() % ((MAP_HEIGHT - 2)) + 1;
            } while (world.w[world.y][world.x]->npcs[world.pc.y][world.pc.x] ||
                get_cost(world.w[world.y][world.x]->m[world.pc.y][world.pc.x], world.pc.x, world.pc.y, PC) == SHRT_MAX ||
                world.rivalDist[world.pc.y][world.pc.x] < 0);
            world.w[world.y][world.x]->npcs[world.pc.y][world.pc.x] = &world.pc;
        }
        generate_npcs(numtrainers, world.w[world.y][world.x]); // Place after map is created
    }
    else {
        collectValidPositions(world.w[world.y][world.x]);

        // fix the position of the pc moving through the gates
        if (world.pc.y == 1) {
            world.pc.y = MAP_HEIGHT - 2;
        }
        else if (world.pc.y == MAP_HEIGHT - 2) {
            world.pc.y = 1;
        }
        else if (world.pc.x == 1) {
            world.pc.x = MAP_WIDTH - 2;
        }
        else if (world.pc.x == MAP_WIDTH - 2) {
            world.pc.x = 1;
        }
        world.w[world.y][world.x]->npcs[world.pc.y][world.pc.x] = &world.pc;
        character* cur_character;
        if ((cur_character = (character*)heap_peek_min(&world.w[world.y][world.x]->event_heap))) {
            world.pc.next_turn = cur_character->next_turn;
        }
        else {
            world.pc.next_turn = 0;
        }
    }
}