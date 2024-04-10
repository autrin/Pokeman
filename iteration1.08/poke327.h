
#ifndef POKE327_H
#define POKE327_H
#include <sstream>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <memory>
#include <unordered_map>
#include <string>
#include <fstream>
#include <functional>
#include <iostream>
#include <cstdio>
#include <cstring>
#include "heap.h"
#include "character.h"
#include "pair.h"
#include <climits>

#define malloc(size) ({                  \
    char *_tmp;                          \
    assert(_tmp = (char *)malloc(size)); \
    _tmp;                                \
})

/* Returns true if random float in [0,1] is less than *
 * numerator/denominator.  Uses only integer math.    */
#define rand_under(numerator, denominator) \
    (rand() < ((RAND_MAX / denominator) * numerator))

/* Returns random integer in [min, max]. */
#define rand_range(min, max) ((rand() % (((max) + 1) - (min))) + (min))

#define UNUSED(f) ((void)f)

#define MAP_X 80
#define MAP_Y 21
#define MIN_TREES 10
#define MIN_BOULDERS 10
#define TREE_PROB 95
#define BOULDER_PROB 95
#define WORLD_SIZE 401

#define MIN_TRAINERS 7
#define ADD_TRAINER_PROB 60

#define MOUNTAIN_SYMBOL '%'
#define BOULDER_SYMBOL '0'
#define TREE_SYMBOL '4'
#define FOREST_SYMBOL '^'
#define GATE_SYMBOL '#'
#define PATH_SYMBOL '#'
#define BAILEY_SYMBOL '#'
#define POKEMART_SYMBOL 'M'
#define POKEMON_CENTER_SYMBOL 'C'
#define TALL_GRASS_SYMBOL ':'
#define SHORT_GRASS_SYMBOL '.'
#define WATER_SYMBOL '~'
#define ERROR_SYMBOL '&'

#define PC_SYMBOL '@'
#define HIKER_SYMBOL 'h'
#define RIVAL_SYMBOL 'r'
#define EXPLORER_SYMBOL 'e'
#define SENTRY_SYMBOL 's'
#define PACER_SYMBOL 'p'
#define SWIMMER_SYMBOL 'm'
#define WANDERER_SYMBOL 'w'

#define mappair(pair) (m->map[pair[dim_y]][pair[dim_x]])
#define mapxy(x, y) (m->map[y][x])
#define heightpair(pair) (m->height[pair[dim_y]][pair[dim_x]])
#define heightxy(x, y) (m->height[y][x])

typedef enum __attribute__((__packed__)) terrain_type
{
    ter_boulder,
    ter_tree,
    ter_path,
    ter_mart,
    ter_center,
    ter_grass,
    ter_clearing,
    ter_mountain,
    ter_forest,
    ter_water,
    ter_gate,
    ter_bailey,
    num_terrain_types,
    ter_debug
} terrain_type_t;

extern int32_t move_cost[num_character_types][num_terrain_types];

class map
{
public:
    terrain_type_t map[MAP_Y][MAP_X];
    uint8_t height[MAP_Y][MAP_X];
    character *cmap[MAP_Y][MAP_X];
    heap_t turn;
    int32_t num_trainers;
    int8_t n, s, e, w;
};

class world
{
public:
    map *world[WORLD_SIZE][WORLD_SIZE];
    pair_t cur_idx;
    map *cur_map;
    /* Please distance maps in world, not map, since *
     * we only need one pair at any given time.      */
    int hiker_dist[MAP_Y][MAP_X];
    int rival_dist[MAP_Y][MAP_X];
    class pc pc;
    int quit;
    int add_trainer_prob;
    int char_seq_num;
};

/* Even unallocated, a WORLD_SIZE x WORLD_SIZE array of pointers is a very *
 * large thing to put on the stack.  To avoid that, world is a global.     */
extern class world world;

extern pair_t all_dirs[8];

#define rand_dir(dir)             \
    {                             \
        int _i = rand() & 0x7;    \
        dir[0] = all_dirs[_i][0]; \
        dir[1] = all_dirs[_i][1]; \
    }

typedef struct path
{
    heap_node_t *hn;
    uint8_t pos[2];
    uint8_t from[2];
    int32_t cost;
} path_t;

enum genders{
    female,
    male,
    genderless
};

class CsvFile
{
public:
    static std::vector<std::vector<std::string>> pokemon_data;
    static std::vector<std::vector<std::string>> pokemon_stats_data;
    static std::vector<std::vector<std::string>> type_names_data;
    static std::vector<std::vector<std::string>> experience_data;
    static std::vector<std::vector<std::string>> pokemon_species_data;
    static std::vector<std::vector<std::string>> stats_data;
    static std::vector<std::vector<std::string>> pokemon_types_data;
    static std::vector<std::vector<std::string>> pokemon_moves_data;
    static std::vector<std::vector<std::string>> moves_data;

    virtual std::vector<std::vector<std::string>> parseFile(const std::string &filename) = 0;
    virtual ~CsvFile() {}
    // virtual void print() const = 0;
};

class Pokemon_csv : public CsvFile
{
private:
    int id;
    std::string identifier;
    int species_id;
    int height;
    int weight;
    int base_experience;
    int order;
    int is_default;

public:
    Pokemon_csv() : id(0), species_id(0), height(0), weight(0), base_experience(0), order(0), is_default(0) {}
    Pokemon_csv(int id, std::string identifier, int species_id, int height, int weight, int base_experience, int order, int is_default) : id(id), identifier(identifier), species_id(species_id),
                                                                                                                                          height(height), weight(weight), base_experience(base_experience), order(order), is_default(is_default) {}
    // void setId(int val) { id = val; }
    // void setIdentifier(const std::string& val) { identifier = val; }
    // void setSpeciesId(int val) { species_id = val; }
    // void setHeight(int val) { height = val; }
    // void setWeight(int val) { weight = val; }
    // void setBaseExperience(int val) { base_experience = val; }
    // void setOrder(int val) { order = val; }
    // void setIsDefault(int val) { is_default = val; }

    int getId() const { return id; }
    std::string getIdentifier() const { return identifier; }
    int getSpeciesId() const { return species_id; }
    int getHeight() const { return height; }
    int getWeight() const { return weight; }
    int getBaseExperience() const { return base_experience; }
    int getOrder() const { return order; }
    int getIsDefault() const { return is_default; }

    std::vector<std::vector<std::string>> parseFile(const std::string &relativePath) override
    {
        std::vector<std::string> basePaths = {
            "/share/cs327/pokedex/pokedex/data/csv/",
            getenv("HOME") + std::string("/.poke327/pokedex/pokedex/data/csv/"),
            "/Users/autrinhakimi/Library/CloudStorage/OneDrive-IowaStateUniversity/Spring2024/CS327/my327/project/Pokeman/data/pokedex/pokedex/data/csv/"};

        std::ifstream file;
        for (const auto &basePath : basePaths)
        {
            std::string fullPath = basePath + relativePath;
            file.open(fullPath);
            if (file.is_open())
            {
                // std::cout << basePath << std::endl;
                break; // Successfully opened the file
            }
        }
        if (!file.is_open())
        {
            std::cerr << "Failed to open file in any known location for: " << relativePath << std::endl;
            return std::vector<std::vector<std::string>>(); // return an empty vector
        }

        std::string line;
        // Skip the header line.
        std::getline(file, line);
        std::vector<std::vector<std::string>> data;

        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string token;
            std::vector<std::string> tokens;

            while (std::getline(iss, token, ','))
            {
                tokens.push_back(token);
            }
            // Check if the last character of the line is a comma
            if (!line.empty() && line.back() == ',')
            {
                tokens.push_back(""); // Add an empty string to represent the empty last field
            }

            if (tokens.size() >= 8)
            { // Make sure there are enough tokens
                try
                {
                    id = !tokens[0].empty() ? std::stoi(tokens[0]) : INT_MAX;
                    identifier = tokens[1];
                    species_id = !tokens[2].empty() ? std::stoi(tokens[2]) : INT_MAX;
                    height = !tokens[3].empty() ? std::stoi(tokens[3]) : INT_MAX;
                    weight = !tokens[4].empty() ? std::stoi(tokens[4]) : INT_MAX;
                    base_experience = !tokens[5].empty() ? std::stoi(tokens[5]) : INT_MAX;
                    order = !tokens[6].empty() ? std::stoi(tokens[6]) : INT_MAX;
                    is_default = tokens.size() > 7 && !tokens[7].empty() ? std::stoi(tokens[7]) : INT_MAX;
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Error parsing line: " << line << "\nException: " << e.what() << std::endl;
                }
                data.push_back(tokens);
            }
        }
        // std::cout << "ID: " << (getId() != INT_MAX ? std::to_string(getId()) : "") << ", "
        //           << "Identifier: " << getIdentifier() << ", "
        //           << "Species ID: " << (getSpeciesId() != INT_MAX ? std::to_string(getSpeciesId()) : "") << ", "
        //           << "Height: " << (getHeight() != INT_MAX ? std::to_string(getHeight()) : "") << ", "
        //           << "Weight: " << (getWeight() != INT_MAX ? std::to_string(getWeight()) : "") << ", "
        //           << "Base Experience: " << (getBaseExperience() != INT_MAX ? std::to_string(getBaseExperience()) : "") << ", "
        //           << "Order: " << (getOrder() != INT_MAX ? std::to_string(getOrder()) : "") << ", "
        //           << "Is Default: " << (getIsDefault() != INT_MAX ? std::to_string(getIsDefault()) : "") << std::endl;
        file.close();
        return data;
    }
};

class stats : public CsvFile
{
private:
    int id;
    int damageClassId;
    std::string identifier;
    int isBattleOnly;
    int gameIndex;

public:
    stats() : id(0), damageClassId(0), isBattleOnly(0), gameIndex(0) {}
    stats(int id,
          int damageClassId,
          std::string identifier,
          int isBattleOnly,
          int gameIndex) : id(id), damageClassId(damageClassId), isBattleOnly(isBattleOnly), gameIndex(gameIndex) {}

    // Getters
    int getId() const { return id; }
    int getDamageClassId() const { return damageClassId; }
    std::string getIdentifier() const { return identifier; }
    int getIsBattleOnly() const { return isBattleOnly; }
    int getGameIndex() const { return gameIndex; }

    // Setters
    // void setId(int newId) { id = newId; }
    // void setDamageClassId(int newDamageClassId) { damageClassId = newDamageClassId; }
    // void setIdentifier(const std::string& newIdentifier) { identifier = newIdentifier; }
    // void setIsBattleOnly(int newIsBattleOnly) { isBattleOnly = newIsBattleOnly; }
    // void setGameIndex(int newGameIndex) { gameIndex = newGameIndex; }

    std::vector<std::vector<std::string>> parseFile(const std::string &relativePath) override
    {
        std::vector<std::string> basePaths = {
            "/share/cs327/pokedex/pokedex/data/csv/",
            getenv("HOME") + std::string("/.poke327/pokedex/pokedex/data/csv/"),
            "/Users/autrinhakimi/Library/CloudStorage/OneDrive-IowaStateUniversity/Spring2024/CS327/my327/project/Pokeman/data/pokedex/pokedex/data/csv/"};

        std::ifstream file;
        for (const auto &basePath : basePaths)
        {
            std::string fullPath = basePath + relativePath;
            file.open(fullPath);
            if (file.is_open())
            {
                // std::cout << basePath << std::endl;
                break; // Successfully opened the file
            }
        }
        if (!file.is_open())
        {
            std::cerr << "Failed to open file in any known location for: " << relativePath << std::endl;
            return std::vector<std::vector<std::string>>(); // return an empty vector
        }

        std::string line;
        // Skip the header line.
        std::getline(file, line);
        std::vector<std::vector<std::string>> data;

        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string token;
            std::vector<std::string> tokens;

            while (std::getline(iss, token, ','))
            {
                tokens.push_back(token);
            }
            if (!line.empty() && line.back() == ',')
            {
                tokens.push_back("");
            }

            // Now `tokens` contains each piece of data from the line.
            if (tokens.size() >= 5)
            { // Make sure there are enough tokens
                try
                {
                    id = !tokens[0].empty() ? std::stoi(tokens[0]) : INT_MAX;
                    damageClassId = !tokens[1].empty() ? std::stoi(tokens[1]) : INT_MAX;
                    identifier = tokens[2];
                    isBattleOnly = !tokens[3].empty() ? std::stoi(tokens[3]) : INT_MAX;
                    gameIndex = tokens.size() > 4 && !tokens[4].empty() ? std::stoi(tokens[4]) : INT_MAX;
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Error parsing line: " << line << "\nException: " << e.what() << std::endl;
                }
                data.push_back(tokens);
            }
        }
        // std::cout << "ID: " << (getId() != INT_MAX ? std::to_string(getId()) : "") << ", "
        //                   << "Damage Class ID: " << (getDamageClassId() != INT_MAX ? std::to_string(getDamageClassId()) : "") << ", "
        //                   << "Identifier: " << getIdentifier() << ", "
        //                   << "Is Battle Only: " << (getIsBattleOnly() != INT_MAX ? std::to_string(getIsBattleOnly()) : "") << ", "
        //                   << "Game Index: " << (getGameIndex() != INT_MAX ? std::to_string(getGameIndex()) : "") << std::endl;
        file.close();
        return data;
    }
};

class pokemon_stats : public CsvFile
{
private:
    int pokemon_id;
    int stat_id;
    int base_stat;
    int effort;

public:
    pokemon_stats() : pokemon_id(0), stat_id(0), base_stat(0), effort(0) {}
    pokemon_stats(int pokemon_id,
                  int stat_id,
                  int base_stat,
                  int effort) : pokemon_id(pokemon_id), stat_id(stat_id), base_stat(base_stat), effort(effort) {}
    // Getters
    int getPokemonId() const
    {
        return pokemon_id;
    }

    int getStatId() const
    {
        return stat_id;
    }

    int getBaseStat() const
    {
        return base_stat;
    }

    int getEffort() const
    {
        return effort;
    }
    std::vector<std::vector<std::string>> parseFile(const std::string &relativePath) override
    {
        std::vector<std::string> basePaths = {
            "/share/cs327/pokedex/pokedex/data/csv/",
            getenv("HOME") + std::string("/.poke327/pokedex/pokedex/data/csv/"),
            "/Users/autrinhakimi/Library/CloudStorage/OneDrive-IowaStateUniversity/Spring2024/CS327/my327/project/Pokeman/data/pokedex/pokedex/data/csv/"};

        std::ifstream file;
        for (const auto &basePath : basePaths)
        {
            std::string fullPath = basePath + relativePath;
            file.open(fullPath);
            if (file.is_open())
            {
                // std::cout << basePath << std::endl;
                break; // Successfully opened the file
            }
        }
        if (!file.is_open())
        {
            std::cerr << "Failed to open file in any known location for: " << relativePath << std::endl;
            return std::vector<std::vector<std::string>>(); // return an empty vector
        }

        std::string line;
        // Skip the header line.
        std::getline(file, line);
        std::vector<std::vector<std::string>> data;

        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string token;
            std::vector<std::string> tokens;

            while (std::getline(iss, token, ','))
            {
                tokens.push_back(token);
            }
            if (!line.empty() && line.back() == ',')
            {
                tokens.push_back(""); // Add an empty string to represent the empty last field
            }
            // Now `tokens` contains each piece of data from the line.
            if (tokens.size() >= 4)
            { // Make sure there are enough tokens
                try
                {
                    pokemon_id = !tokens[0].empty() ? std::stoi(tokens[0]) : INT_MAX;
                    stat_id = !tokens[1].empty() ? std::stoi(tokens[1]) : INT_MAX;
                    base_stat = !tokens[2].empty() ? std::stoi(tokens[2]) : INT_MAX;
                    effort = tokens.size() > 3 && !tokens[3].empty() ? std::stoi(tokens[3]) : INT_MAX;
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Error parsing line: " << line << "\nException: " << e.what() << std::endl;
                }
                data.push_back(tokens);
            }
        }

        // std::cout << "Pokemon ID: " << (getPokemonId() != INT_MAX ? std::to_string(getPokemonId()) : "") << ", "
        //           << "Stat ID: " << (getStatId() != INT_MAX ? std::to_string(getStatId()) : "") << ", "
        //           << "Base Stat: " << (getBaseStat() != INT_MAX ? std::to_string(getBaseStat()) : "") << ", "
        //           << "Effort: " << (getEffort() != INT_MAX ? std::to_string(getEffort()) : "") << std::endl;
        file.close();
        return data;
    }
};

class type_names : public CsvFile
{
private:
    int type_id;
    int local_language_id;
    std::string name;

public:
    type_names() : type_id(0), local_language_id(0) {}
    type_names(int type_id,
               int local_language_id,
               std::string name) : type_id(type_id), local_language_id(local_language_id), name(name) {}
    // Setters
    // void setTypeId(int val) { type_id = val; }
    // void setLocalLanguageId(int val) { local_language_id = val; }
    // void setName(const std::string& val) { name = val; }

    // Getters
    int getTypeId() const { return type_id; }
    int getLocalLanguageId() const { return local_language_id; }
    std::string getName() const { return name; }

    std::vector<std::vector<std::string>> parseFile(const std::string &relativePath) override
    {
        std::vector<std::string> basePaths = {
            "/share/cs327/pokedex/pokedex/data/csv/",
            getenv("HOME") + std::string("/.poke327/pokedex/pokedex/data/csv/"),
            "/Users/autrinhakimi/Library/CloudStorage/OneDrive-IowaStateUniversity/Spring2024/CS327/my327/project/Pokeman/data/pokedex/pokedex/data/csv/"};

        std::ifstream file;
        for (const auto &basePath : basePaths)
        {
            std::string fullPath = basePath + relativePath;
            file.open(fullPath);
            if (file.is_open())
            {
                // std::cout << basePath << std::endl;
                break; // Successfully opened the file
            }
        }
        if (!file.is_open())
        {
            std::cerr << "Failed to open file in any known location for: " << relativePath << std::endl;
            return std::vector<std::vector<std::string>>(); // return an empty vector
        }

        std::string line;
        // Skip the header line.
        std::getline(file, line);
        std::vector<std::vector<std::string>> data;

        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string token;
            std::vector<std::string> tokens;

            while (std::getline(iss, token, ','))
            {
                tokens.push_back(token);
            }
            if (!line.empty() && line.back() == ',')
            {
                tokens.push_back(""); // Add an empty string to represent the empty last field
            }
            // Now `tokens` contains each piece of data from the line.
            if (tokens.size() >= 3)
            { // Make sure there are enough tokens
                try
                {
                    type_id = !tokens[0].empty() ? std::stoi(tokens[0]) : INT_MAX;
                    local_language_id = !tokens[1].empty() ? std::stoi(tokens[1]) : INT_MAX;
                    name = tokens[2];
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Error parsing line: " << line << "\nException: " << e.what() << std::endl;
                }
                data.push_back(tokens);
            }
        }
        //  std::cout << "Type ID: " << (getTypeId() != INT_MAX ? std::to_string(getTypeId()) : "") << ", "
        //           << "Local Language ID: " << (getLocalLanguageId() != INT_MAX ? std::to_string(getLocalLanguageId()) : "") << ", "
        //           << "Name: " << getName() << std::endl;
        file.close();
        return data;
    }
};

class Experience : public CsvFile
{
private:
    int growth_rate_id;
    int level;
    int experience;

public:
    Experience() : growth_rate_id(0), level(0), experience(0) {}
    Experience(int growth_rate_id,
               int level,
               int experience) : growth_rate_id(growth_rate_id), level(level), experience(experience) {}

    // Setters
    // void setGrowthRateId(int id) { growth_rate_id = id; }
    // void setLevel(int lvl) { level = lvl; }
    // void setExperience(int exp) { experience = exp; }

    // Getters
    int getGrowthRateId() const { return growth_rate_id; }
    int getLevel() const { return level; }
    int getExperience() const { return experience; }

    std::vector<std::vector<std::string>> parseFile(const std::string &relativePath) override
    {
        std::vector<std::string> basePaths = {
            "/share/cs327/pokedex/pokedex/data/csv/",
            getenv("HOME") + std::string("/.poke327/pokedex/pokedex/data/csv/"),
            "/Users/autrinhakimi/Library/CloudStorage/OneDrive-IowaStateUniversity/Spring2024/CS327/my327/project/Pokeman/data/pokedex/pokedex/data/csv/"};

        std::ifstream file;
        for (const auto &basePath : basePaths)
        {
            std::string fullPath = basePath + relativePath;
            file.open(fullPath);
            if (file.is_open())
            {
                // std::cout << basePath << std::endl;
                break; // Successfully opened the file
            }
        }
        if (!file.is_open())
        {
            std::cerr << "Failed to open file in any known location for: " << relativePath << std::endl;
            return std::vector<std::vector<std::string>>(); // return an empty vector
        }

        std::string line;
        // Skip the header line.
        std::getline(file, line);
        std::vector<std::vector<std::string>> data;

        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string token;
            std::vector<std::string> tokens;

            while (std::getline(iss, token, ','))
            {
                tokens.push_back(token);
            }
            if (!line.empty() && line.back() == ',')
            {
                tokens.push_back(""); // Add an empty string to represent the empty last field
            }
            // Now `tokens` contains each piece of data from the line.
            if (tokens.size() >= 3)
            {
                try
                {
                    growth_rate_id = !tokens[0].empty() ? std::stoi(tokens[0]) : INT_MAX;
                    level = !tokens[1].empty() ? std::stoi(tokens[1]) : INT_MAX;
                    experience = tokens.size() > 2 && !tokens[2].empty() ? std::stoi(tokens[2]) : INT_MAX;
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Error parsing line: " << line << "\nException: " << e.what() << std::endl;
                }
                data.push_back(tokens);
            }
        }
        //  std::cout << "Growth Rate ID: " << (getGrowthRateId() != INT_MAX ? std::to_string(getGrowthRateId()) : "") << ", "
        //           << "Level: " << (getLevel() != INT_MAX ? std::to_string(getLevel()) : "") << ", "
        //           << "Experience: " << (getExperience() != INT_MAX ? std::to_string(getExperience()) : "") << std::endl;
        file.close();
        return data;
    }
};

class pokemon_species : public CsvFile
{
private:
    int id;
    std::string identifier;
    int generation_id;
    int evolves_from_species_id;
    int evolution_chain_id;
    int color_id;
    int shape_id;
    int habitat_id;
    int gender_rate;
    int capture_rate;
    int base_happiness;
    int is_baby;
    int hatch_counter;
    int has_gender_differences;
    int growth_rate_id;
    int forms_switchable;
    int is_legendary;
    int is_mythical;
    int order;
    int conquest_order;

public:
    pokemon_species() : id(0), identifier(""), generation_id(0), evolves_from_species_id(0),
                        evolution_chain_id(0), color_id(0), shape_id(0), habitat_id(0), gender_rate(0),
                        capture_rate(0), base_happiness(0), is_baby(0), hatch_counter(0),
                        has_gender_differences(0), growth_rate_id(0), forms_switchable(0),
                        is_legendary(0), is_mythical(0), order(0), conquest_order(0) {}

    pokemon_species(int id,
                    std::string identifier,
                    int generation_id,
                    int evolves_from_species_id,
                    int evolution_chain_id,
                    int color_id,
                    int shape_id,
                    int habitat_id,
                    int gender_rate,
                    int capture_rate,
                    int base_happiness,
                    int is_baby,
                    int hatch_counter,
                    int has_gender_differences,
                    int growth_rate_id,
                    int forms_switchable,
                    int is_legendary,
                    int is_mythical,
                    int order,
                    int conquest_order) : id(id), generation_id(generation_id), evolves_from_species_id(evolves_from_species_id), evolution_chain_id(evolution_chain_id),
                                          color_id(color_id), shape_id(shape_id), habitat_id(habitat_id), gender_rate(gender_rate), capture_rate(capture_rate), base_happiness(base_happiness),
                                          is_baby(is_baby), hatch_counter(hatch_counter), has_gender_differences(has_gender_differences), growth_rate_id(growth_rate_id), forms_switchable(forms_switchable),
                                          is_legendary(is_legendary), is_mythical(is_mythical), order(order), conquest_order(conquest_order) {}

    // Getters
    int getId() const { return id; }
    std::string getIdentifier() const { return identifier; }
    int getGenerationId() const { return generation_id; }
    int getEvolvesFromSpeciesId() const { return evolves_from_species_id; }
    int getEvolutionChainId() const { return evolution_chain_id; }
    int getColorId() const { return color_id; }
    int getShapeId() const { return shape_id; }
    int getHabitatId() const { return habitat_id; }
    int getGenderRate() const { return gender_rate; }
    int getCaptureRate() const { return capture_rate; }
    int getBaseHappiness() const { return base_happiness; }
    int getIsBaby() const { return is_baby; }
    int getHatchCounter() const { return hatch_counter; }
    int getHasGenderDifferences() const { return has_gender_differences; }
    int getGrowthRateId() const { return growth_rate_id; }
    int getFormsSwitchable() const { return forms_switchable; }
    int getIsLegendary() const { return is_legendary; }
    int getIsMythical() const { return is_mythical; }
    int getOrder() const { return order; }
    int getConquestOrder() const { return conquest_order; }

    // Setters
    // void setId(int value) { id = value; }
    // void setIdentifier(const std::string& value) { identifier = value; }
    // void setGenerationId(int value) { generation_id = value; }
    // void setEvolvesFromSpeciesId(int value) { evolves_from_species_id = value; }
    // void setEvolutionChainId(int value) { evolution_chain_id = value; }
    // void setColorId(int value) { color_id = value; }
    // void setShapeId(int value) { shape_id = value; }
    // void setHabitatId(int value) { habitat_id = value; }
    // void setGenderRate(int value) { gender_rate = value; }
    // void setCaptureRate(int value) { capture_rate = value; }
    // void setBaseHappiness(int value) { base_happiness = value; }
    // void setIsBaby(int value) { is_baby = value; }
    // void setHatchCounter(int value) { hatch_counter = value; }
    // void setHasGenderDifferences(int value) { has_gender_differences = value; }
    // void setGrowthRateId(int value) { growth_rate_id = value; }
    // void setFormsSwitchable(int value) { forms_switchable = value; }
    // void setIsLegendary(int value) { is_legendary = value; }
    // void setIsMythical(int value) { is_mythical = value; }
    // void setOrder(int value) { order = value; }
    // void setConquestOrder(int value) { conquest_order = value; }

    std::vector<std::vector<std::string>> parseFile(const std::string &relativePath) override
    {
        std::vector<std::string> basePaths = {
            "/share/cs327/pokedex/pokedex/data/csv/",
            getenv("HOME") + std::string("/.poke327/pokedex/pokedex/data/csv/"),
            "/Users/autrinhakimi/Library/CloudStorage/OneDrive-IowaStateUniversity/Spring2024/CS327/my327/project/Pokeman/data/pokedex/pokedex/data/csv/"};

        std::ifstream file;
        for (const auto &basePath : basePaths)
        {
            std::string fullPath = basePath + relativePath;
            file.open(fullPath);
            if (file.is_open())
            {
                // std::cout << basePath << std::endl;
                break; // Successfully opened the file
            }
        }
        if (!file.is_open())
        {
            std::cerr << "Failed to open file in any known location for: " << relativePath << std::endl;
            return std::vector<std::vector<std::string>>(); // return an empty vector
        }

        std::string line;
        // Skip the header line.
        std::getline(file, line);
        std::vector<std::vector<std::string>> data;
        while (std::getline(file, line))
        {
            std::vector<std::string> tokens;
            std::string token;
            std::istringstream iss(line);

            while (std::getline(iss, token, ','))
            {
                tokens.push_back(token);
            }
            if (!line.empty() && line.back() == ',')
            {
                tokens.push_back(""); // Add an empty string to represent the empty last field
            }
            // Now `tokens` contains each piece of data from the line.
            if (tokens.size() >= 20)
            { // Make sure there are enough tokens
                try
                {
                    id = !tokens[0].empty() ? std::stoi(tokens[0]) : INT_MAX;
                    identifier = tokens[1];
                    generation_id = !tokens[2].empty() ? std::stoi(tokens[2]) : INT_MAX;
                    evolves_from_species_id = !tokens[3].empty() ? std::stoi(tokens[3]) : INT_MAX;
                    evolution_chain_id = !tokens[4].empty() ? std::stoi(tokens[4]) : INT_MAX;
                    color_id = !tokens[5].empty() ? std::stoi(tokens[5]) : INT_MAX;
                    shape_id = !tokens[6].empty() ? std::stoi(tokens[6]) : INT_MAX;
                    habitat_id = !tokens[7].empty() ? std::stoi(tokens[7]) : INT_MAX;
                    gender_rate = !tokens[8].empty() ? std::stoi(tokens[8]) : INT_MAX;
                    capture_rate = !tokens[9].empty() ? std::stoi(tokens[9]) : INT_MAX;
                    base_happiness = !tokens[10].empty() ? std::stoi(tokens[10]) : INT_MAX;
                    is_baby = !tokens[11].empty() ? std::stoi(tokens[11]) : INT_MAX;
                    hatch_counter = !tokens[12].empty() ? std::stoi(tokens[12]) : INT_MAX;
                    has_gender_differences = !tokens[13].empty() ? std::stoi(tokens[13]) : INT_MAX;
                    growth_rate_id = !tokens[14].empty() ? std::stoi(tokens[14]) : INT_MAX;
                    forms_switchable = !tokens[15].empty() ? std::stoi(tokens[15]) : INT_MAX;
                    is_legendary = !tokens[16].empty() ? std::stoi(tokens[16]) : INT_MAX;
                    is_mythical = !tokens[17].empty() ? std::stoi(tokens[17]) : INT_MAX;
                    order = !tokens[18].empty() ? std::stoi(tokens[18]) : INT_MAX;
                    conquest_order = tokens.size() > 19 && !tokens[19].empty() ? std::stoi(tokens[19]) : INT_MAX;
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Error parsing line: " << line << "\nException: " << e.what() << std::endl;
                }
                data.push_back(tokens);
            }
        }
        //  std::cout << "id: " << (getId() != INT_MAX ? std::to_string(getId()) : "") << ", "
        //           << "identifier: " << getIdentifier() << ", "
        //           << "generation_id: " << (getGenerationId() != INT_MAX ? std::to_string(getGenerationId()) : "") << ", "
        //           << "evolves_from_species_id: " << (getEvolvesFromSpeciesId() != INT_MAX ? std::to_string(getEvolvesFromSpeciesId()) : "") << ", "
        //           << "evolution_chain_id: " << (getEvolutionChainId() != INT_MAX ? std::to_string(getEvolutionChainId()) : "") << ", "
        //           << "color_id: " << (getColorId() != INT_MAX ? std::to_string(getColorId()) : "") << ", "
        //           << "shape_id: " << (getShapeId() != INT_MAX ? std::to_string(getShapeId()) : "") << ", "
        //           << "habitat_id: " << (getHabitatId() != INT_MAX ? std::to_string(getHabitatId()) : "") << ", "
        //           << "gender_rate: " << (getGenderRate() != INT_MAX ? std::to_string(getGenderRate()) : "") << ", "
        //           << "capture_rate: " << (getCaptureRate() != INT_MAX ? std::to_string(getCaptureRate()) : "") << ", "
        //           << "base_happiness: " << (getBaseHappiness() != INT_MAX ? std::to_string(getBaseHappiness()) : "") << ", "
        //           << "is_baby: " << (getIsBaby() != INT_MAX ? std::to_string(getIsBaby()) : "") << ", "
        //           << "hatch_counter: " << (getHatchCounter() != INT_MAX ? std::to_string(getHatchCounter()) : "") << ", "
        //           << "has_gender_differences: " << (getHasGenderDifferences() != INT_MAX ? std::to_string(getHasGenderDifferences()) : "") << ", "
        //           << "growth_rate_id: " << (getGrowthRateId() != INT_MAX ? std::to_string(getGrowthRateId()) : "") << ", "
        //           << "forms_switchable: " << (getFormsSwitchable() != INT_MAX ? std::to_string(getFormsSwitchable()) : "") << ", "
        //           << "is_legendary: " << (getIsLegendary() != INT_MAX ? std::to_string(getIsLegendary()) : "") << ", "
        //           << "is_mythical: " << (getIsMythical() != INT_MAX ? std::to_string(getIsMythical()) : "") << ", "
        //           << "order: " << (getOrder() != INT_MAX ? std::to_string(getOrder()) : "") << ", "
        //           << "conquest_order: " << (getConquestOrder() != INT_MAX ? std::to_string(getConquestOrder()) : "")
        //           << std::endl;
        file.close();
        return data;
    }
};

class pokemon_moves : public CsvFile
{
private:
    int pokemon_id;
    int version_group_id;
    int move_id;
    int pokemon_move_method_id;
    int level;
    int order;

public:
    pokemon_moves() : pokemon_id(0), version_group_id(0), move_id(0),
                      pokemon_move_method_id(0), level(0), order(0) {}
    pokemon_moves(int pokemon_id, int version_group_id, int move_id,
                  int pokemon_move_method_id, int level, int order)
        : pokemon_id(pokemon_id), version_group_id(version_group_id),
          move_id(move_id), pokemon_move_method_id(pokemon_move_method_id),
          level(level), order(order) {}

    // Getters
    int getPokemonId() const { return pokemon_id; }
    int getVersionGroupId() const { return version_group_id; }
    int getMoveId() const { return move_id; }
    int getPokemonMoveMethodId() const { return pokemon_move_method_id; }
    int getLevel() const { return level; }
    int getOrder() const { return order; }

    std::vector<std::vector<std::string>> parseFile(const std::string &relativePath) override
    {
        std::vector<std::string> basePaths = {
            "/share/cs327/pokedex/pokedex/data/csv/",
            getenv("HOME") + std::string("/.poke327/pokedex/pokedex/data/csv/"),
            "/Users/autrinhakimi/Library/CloudStorage/OneDrive-IowaStateUniversity/Spring2024/CS327/my327/project/Pokeman/data/pokedex/pokedex/data/csv/"};

        std::ifstream file;
        for (const auto &basePath : basePaths)
        {
            std::string fullPath = basePath + relativePath;
            file.open(fullPath);
            if (file.is_open())
            {
                // std::cout << basePath << std::endl;
                break; // Successfully opened the file
            }
        }
        if (!file.is_open())
        {
            std::cerr << "Failed to open file in any known location for: " << relativePath << std::endl;
            return std::vector<std::vector<std::string>>(); // return an empty vector
        }

        std::string line;
        // Skip the header line.
        std::getline(file, line);
        std::vector<std::vector<std::string>> data;

        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string token;
            std::vector<std::string> tokens;

            while (std::getline(iss, token, ','))
            {
                tokens.push_back(token);
            }
            // Check if the last character of the line is a comma
            if (!line.empty() && line.back() == ',')
            {
                tokens.push_back(""); // Add an empty string to represent the empty last field
            }

            if (tokens.size() >= 6)
            {
                try
                {
                    pokemon_id = !tokens[0].empty() ? std::stoi(tokens[0]) : INT_MAX;
                    version_group_id = !tokens[1].empty() ? std::stoi(tokens[1]) : INT_MAX;
                    move_id = !tokens[2].empty() ? std::stoi(tokens[2]) : INT_MAX;
                    pokemon_move_method_id = !tokens[3].empty() ? std::stoi(tokens[3]) : INT_MAX;
                    level = !tokens[4].empty() ? std::stoi(tokens[4]) : INT_MAX;
                    order = tokens.size() > 5 && !tokens[5].empty() ? std::stoi(tokens[5]) : INT_MAX;
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Error parsing line: " << line << "\nException: " << e.what() << std::endl;
                }
                data.push_back(tokens);
            }
        }
        // std::cout << "pokemon Id: " << (getPokemonId() != INT_MAX ? std::to_string(getPokemonId()) : "") << ", "
        //   << "Version Group Id: " << (getVersionGroupId() != INT_MAX ? std::to_string(getVersionGroupId()) : "") << ", "
        //   << "Move Id: " << (getMoveId() != INT_MAX ? std::to_string(getMoveId()) : "") << ", "
        //   << "pokemon_move_method_id: " << (getPokemonMoveMethodId() != INT_MAX ? std::to_string(getPokemonMoveMethodId()) : "") << ", "
        //   << "level: " << (getLevel() != INT_MAX ? std::to_string(getLevel()) : "") << ", "
        //   << "order: " << (getOrder() != INT_MAX ? std::to_string(getOrder()) : "")
        //   << std::endl;
        file.close();
        return data;
    }
};

class moves : public CsvFile
{
private:
    int id;
    std::string identifier;
    int generation_id;
    int type_id;
    int power;
    int pp;
    int accuracy;
    int priority;
    int target_id;
    int damage_class_id;
    int effect_id;
    int effect_chance;
    int contest_type_id;
    int contest_effect_id;
    int super_contest_effect_id;

public:
    moves() : id(0), generation_id(0), type_id(0), power(0),
              pp(0), accuracy(0), priority(0), target_id(0), damage_class_id(0),
              effect_id(0), effect_chance(0), contest_type_id(0), contest_effect_id(0),
              super_contest_effect_id(0) {}
    moves(int id,
          std::string identifier,
          int generation_id,
          int type_id,
          int power,
          int pp,
          int accuracy,
          int priority,
          int target_id,
          int damage_class_id,
          int effect_id,
          int effect_chance,
          int contest_type_id,
          int contest_effect_id,
          int super_contest_effect_id) : id(id), identifier(identifier), generation_id(generation_id), type_id(type_id),
                                         power(power), pp(pp), accuracy(accuracy), priority(priority), target_id(target_id), damage_class_id(damage_class_id),
                                         effect_id(effect_id), effect_chance(effect_chance), contest_type_id(contest_type_id), contest_effect_id(contest_effect_id),
                                         super_contest_effect_id(super_contest_effect_id) {}
    // Getters and Setters
    // void setId(int idVal) { id = idVal; }
    // void setIdentifier(const std::string& identifierVal) { identifier = identifierVal; }
    // void setGeneration_id(int genId) { generation_id = genId; }
    // void setType_id(int typeId) { type_id = type_id; }

    int getId() const { return id; }
    std::string getIdentifier() const { return identifier; }
    int getGeneration_id() const { return generation_id; }
    int getType_id() const { return type_id; }
    int getPower() const { return power; }
    int getPp() const { return pp; }
    int getAccuracy() const { return accuracy; }
    int getPriority() const { return priority; }
    int getTarget_id() const { return target_id; }
    int getDamage_class_id() const { return damage_class_id; }
    int getEffect_id() const { return effect_id; }
    int getEffect_chance() const { return effect_chance; }
    int getContest_type_id() const { return contest_type_id; }
    int getContest_effect_id() const { return contest_effect_id; }
    int getSuper_contest_effect_id() const { return super_contest_effect_id; }

    std::vector<std::vector<std::string>> parseFile(const std::string &relativePath) override
    {
        std::vector<std::string> basePaths = {
            "/share/cs327/pokedex/pokedex/data/csv/",
            getenv("HOME") + std::string("/.poke327/pokedex/pokedex/data/csv/"),
            "/Users/autrinhakimi/Library/CloudStorage/OneDrive-IowaStateUniversity/Spring2024/CS327/my327/project/Pokeman/data/pokedex/pokedex/data/csv/"};

        std::ifstream file;
        for (const auto &basePath : basePaths)
        {
            std::string fullPath = basePath + relativePath;
            file.open(fullPath);
            if (file.is_open())
            {
                break; // Successfully opened the file
            }
        }

        if (!file.is_open())
        {
            std::cerr << "Failed to open file in any known location for: " << relativePath << std::endl;
            return std::vector<std::vector<std::string>>(); // return an empty vector
        }

        std::string line;
        // Skip the header line.
        std::getline(file, line);
        std::vector<std::vector<std::string>> data;

        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string token;
            std::vector<std::string> tokens;
            while (std::getline(iss, token, ','))
            {
                tokens.push_back(token);
            }
            if (!line.empty() && line.back() == ',')
            {
                tokens.push_back(""); // Add an empty string to represent the empty last field
            }
            if (tokens.size() >= 15)
            {
                try
                {
                    id = tokens[0].empty() ? INT_MAX : std::stoi(tokens[0]);
                    identifier = tokens[1];
                    generation_id = tokens[2].empty() ? INT_MAX : std::stoi(tokens[2]);
                    type_id = tokens[3].empty() ? INT_MAX : std::stoi(tokens[3]);
                    power = tokens[4].empty() ? INT_MAX : std::stoi(tokens[4]);
                    pp = tokens[5].empty() ? INT_MAX : std::stoi(tokens[5]);
                    accuracy = tokens[6].empty() ? INT_MAX : std::stoi(tokens[6]);
                    priority = tokens[7].empty() ? INT_MAX : std::stoi(tokens[7]);
                    target_id = tokens[8].empty() ? INT_MAX : std::stoi(tokens[8]);
                    damage_class_id = tokens[9].empty() ? INT_MAX : std::stoi(tokens[9]);
                    effect_id = tokens[10].empty() ? INT_MAX : std::stoi(tokens[10]);
                    effect_chance = tokens[11].empty() ? INT_MAX : std::stoi(tokens[11]);
                    contest_type_id = tokens[12].empty() ? INT_MAX : std::stoi(tokens[12]);
                    contest_effect_id = tokens[13].empty() ? INT_MAX : std::stoi(tokens[13]);
                    super_contest_effect_id = tokens.size() > 14 && !tokens[14].empty() ? std::stoi(tokens[14]) : INT_MAX;
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Error parsing line: " << line << "\nException: " << e.what() << std::endl;
                }
                data.push_back(tokens);
            }
        }
        // std::cout << "ID: " << (id != INT_MAX ? std::to_string(id) : "") << ", "
        //   << "Identifier: " << identifier << ", "
        //   << "Generation ID: " << (generation_id != INT_MAX ? std::to_string(generation_id) : "") << ", "
        //   << "Type ID: " << (type_id != INT_MAX ? std::to_string(type_id) : "") << ", "
        //   << "Power: " << (power != INT_MAX ? std::to_string(power) : "") << ", "
        //   << "PP: " << (pp != INT_MAX ? std::to_string(pp) : "") << ", "
        //   << "Accuracy: " << (accuracy != INT_MAX ? std::to_string(accuracy) : "") << ", "
        //   << "Priority: " << (priority != INT_MAX ? std::to_string(priority) : "") << ", "
        //   << "Target ID: " << (target_id != INT_MAX ? std::to_string(target_id) : "") << ", "
        //   << "Damage Class ID: " << (damage_class_id != INT_MAX ? std::to_string(damage_class_id) : "") << ", "
        //   << "Effect ID: " << (effect_id != INT_MAX ? std::to_string(effect_id) : "") << ", "
        //   << "Effect Chance: " << (effect_chance != INT_MAX ? std::to_string(effect_chance) : "") << ", "
        //   << "Contest Type ID: " << (contest_type_id != INT_MAX ? std::to_string(contest_type_id) : "") << ", "
        //   << "Contest Effect ID: " << (contest_effect_id != INT_MAX ? std::to_string(contest_effect_id) : "") << ", "
        //   << "Super Contest Effect ID: " << (super_contest_effect_id != INT_MAX ? std::to_string(super_contest_effect_id) : "")
        //   << std::endl;
        file.close();
        return data;
    }
};
class pokemon_types : public CsvFile
{
private:
    int pokemon_id;
    int type_id;
    int slot;

public:
    pokemon_types() : pokemon_id(0), type_id(0), slot(0) {}
    pokemon_types(int pokemon_id,
                  int type_id,
                  int slot) : pokemon_id(pokemon_id), type_id(type_id), slot(slot) {}
    // Setters
    // void setPokemonId(int value) { pokemon_id = value; }
    // void setTypeId(int value) { type_id = value; }
    // void setSlot(int value) { slot = value; }

    // Getters
    int getPokemonId() const { return pokemon_id; }
    int getTypeId() const { return type_id; }
    int getSlot() const { return slot; }

    std::vector<std::vector<std::string>> parseFile(const std::string &relativePath) override
    {
        std::vector<std::string> basePaths = {
            "/share/cs327/pokedex/pokedex/data/csv/",
            getenv("HOME") + std::string("/.poke327/pokedex/pokedex/data/csv/"),
            "/Users/autrinhakimi/Library/CloudStorage/OneDrive-IowaStateUniversity/Spring2024/CS327/my327/project/Pokeman/data/pokedex/pokedex/data/csv/"};

        std::ifstream file;
        for (const auto &basePath : basePaths)
        {
            std::string fullPath = basePath + relativePath;
            file.open(fullPath);
            if (file.is_open())
            {
                break; // Successfully opened the file
            }
        }

        if (!file.is_open())
        {
            std::cerr << "Failed to open file in any known location for: " << relativePath << std::endl;
            return std::vector<std::vector<std::string>>(); // return an empty vector
        }

        std::string line;
        // Skip the header line.
        std::getline(file, line);
        std::vector<std::vector<std::string>> data;

        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string token;
            std::vector<std::string> tokens;
            while (std::getline(iss, token, ','))
            {
                tokens.push_back(token);
            }
            if (!line.empty() && line.back() == ',')
            {
                tokens.push_back(""); // Add an empty string to represent the empty last field
            }
            if (tokens.size() >= 3)
            { // Ensure there are enough tokens for all fields
                try
                {
                    pokemon_id = tokens[0].empty() ? INT_MAX : std::stoi(tokens[0]);
                    type_id = tokens[1].empty() ? INT_MAX : std::stoi(tokens[1]);
                    slot = tokens.size() > 2 && !tokens[2].empty() ? std::stoi(tokens[2]) : INT_MAX;
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Error parsing line: " << line << "\nException: " << e.what() << std::endl;
                }
                data.push_back(tokens);
            }
        }
        // std::cout << "pokemon ID: " << (pokemon_id != INT_MAX ? std::to_string(pokemon_id) : "") << ", "
        //   << "Type ID: " << (type_id != INT_MAX ? std::to_string(type_id) : "") << ", "
        //   << "Slot: " << (slot != INT_MAX ? std::to_string(slot) : "")
        //   << std::endl;
        file.close();
        return data;
    }
};

// Factory
class CsvFileFactory
{
public:
    static std::unique_ptr<CsvFile> createFromFile(const std::string &typeName, const std::string &filename)
    {
        // Determine type and parse accordingly
        if (typeName == "pokemon")
        {
            auto pokemon = std::make_unique<Pokemon_csv>();
            CsvFile::pokemon_data = pokemon->parseFile(filename);
            return pokemon;
        }
        else if (typeName == "stats")
        {
            auto s = std::make_unique<stats>();
            std::vector<std::vector<std::string>> stats_data = s->parseFile(filename);
            return s;
        }
        else if (typeName == "pokemon_stats")
        {
            auto pokemon_st = std::make_unique<pokemon_stats>();
            CsvFile::pokemon_stats_data = pokemon_st->parseFile(filename);
            return pokemon_st;
        }
        else if (typeName == "type_names")
        {
            auto type_n = std::make_unique<type_names>();
            CsvFile::type_names_data = type_n->parseFile(filename);
            return type_n;
        }
        else if (typeName == "experience")
        {
            auto experience = std::make_unique<Experience>();
            CsvFile::experience_data = experience->parseFile(filename);
            return experience;
        }
        else if (typeName == "pokemon_species")
        {
            auto pokemon_sp = std::make_unique<pokemon_species>();
            CsvFile::pokemon_species_data = pokemon_sp->parseFile(filename);
            return pokemon_sp;
        }
        else if (typeName == "pokemon_moves")
        {
            auto pokemon_m = std::make_unique<pokemon_moves>();
            CsvFile::pokemon_moves_data = pokemon_m->parseFile(filename);
            return pokemon_m;
        }
        else if (typeName == "moves")
        {
            auto m = std::make_unique<moves>();
            CsvFile::moves_data = m->parseFile(filename);
            return m;
        }
        else if (typeName == "pokemon_types")
        {
            auto pokemon_t = std::make_unique<pokemon_types>();
            CsvFile::pokemon_types_data = pokemon_t->parseFile(filename);
            return pokemon_t;
        }
        return nullptr;
    }
};

int new_map(int teleport);
void pathfind(map *m);

#endif
