#ifndef POKE327_H
# define POKE327_H

# include <cstdlib>
# include <cassert>
#include <memory>
#include <unordered_map>
#include <string>
#include <functional>
#include <iostream>

# include "heap.h"
# include "character.h"
# include "pair.h"

#define malloc(size) ({                 \
  char *_tmp;                           \
  assert(_tmp = (char *) malloc(size)); \
  _tmp;                                 \
})

/* Returns true if random float in [0,1] is less than *
 * numerator/denominator.  Uses only integer math.    */
# define rand_under(numerator, denominator) \
  (rand() < ((RAND_MAX / denominator) * numerator))

 /* Returns random integer in [min, max]. */
# define rand_range(min, max) ((rand() % (((max) + 1) - (min))) + (min))

# define UNUSED(f) ((void) f)

#define MAP_X              80
#define MAP_Y              21
#define MIN_TREES          10
#define MIN_BOULDERS       10
#define TREE_PROB          95
#define BOULDER_PROB       95
#define WORLD_SIZE         401

#define MIN_TRAINERS     7
#define ADD_TRAINER_PROB 60

#define MOUNTAIN_SYMBOL       '%'
#define BOULDER_SYMBOL        '0'
#define TREE_SYMBOL           '4'
#define FOREST_SYMBOL         '^'
#define GATE_SYMBOL           '#'
#define PATH_SYMBOL           '#'
#define BAILEY_SYMBOL         '#'
#define POKEMART_SYMBOL       'M'
#define POKEMON_CENTER_SYMBOL 'C'
#define TALL_GRASS_SYMBOL     ':'
#define SHORT_GRASS_SYMBOL    '.'
#define WATER_SYMBOL          '~'
#define ERROR_SYMBOL          '&'

#define PC_SYMBOL       '@'
#define HIKER_SYMBOL    'h'
#define RIVAL_SYMBOL    'r'
#define EXPLORER_SYMBOL 'e'
#define SENTRY_SYMBOL   's'
#define PACER_SYMBOL    'p'
#define SWIMMER_SYMBOL  'm'
#define WANDERER_SYMBOL 'w'

#define mappair(pair) (m->map[pair[dim_y]][pair[dim_x]])
#define mapxy(x, y) (m->map[y][x])
#define heightpair(pair) (m->height[pair[dim_y]][pair[dim_x]])
#define heightxy(x, y) (m->height[y][x])

typedef enum __attribute__((__packed__)) terrain_type {
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

class map {
public:
  terrain_type_t map[MAP_Y][MAP_X];
  uint8_t height[MAP_Y][MAP_X];
  character* cmap[MAP_Y][MAP_X];
  heap_t turn;
  int32_t num_trainers;
  int8_t n, s, e, w;
};

class world {
public:
  map* world[WORLD_SIZE][WORLD_SIZE];
  pair_t cur_idx;
  map* cur_map;
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

#define rand_dir(dir) {     \
  int _i = rand() & 0x7;    \
  dir[0] = all_dirs[_i][0]; \
  dir[1] = all_dirs[_i][1]; \
}

#define REGISTER_CSV_TYPE(TYPE) \
    CsvFactory::instance().registerType(#TYPE, []() -> std::unique_ptr<CsvFile> { return std::make_unique<TYPE>(); })


typedef struct path {
  heap_node_t* hn;
  uint8_t pos[2];
  uint8_t from[2];
  int32_t cost;
} path_t;


class CsvFile {
public:
  virtual void parseFile(const std::string& filename) = 0;
  virtual ~CsvFile() {}
};

class Pokemon : public CsvFile {
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
  Pokemon(int id, std::string identifier, int species_id, int height, int weight, int base_experience, int order, int is_default) : id(id), identifier(identifier), species_id(species_id),
    height(height), weight(weight), base_experience(base_experience), order(order), is_default(is_default) {}

  // // Getters and Setters
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

  void parseFile(const std::string& filename) override {
    // Parsing logic here
  }
};

class stats : public CsvFile {
private:
  int id;
  int damageClassId;
  std::string identifier;
  bool isBattleOnly;
  int gameIndex;
public:
  // Constructor, if needed
  stats(int id,
    int damageClassId,
    std::string identifier,
    bool isBattleOnly,
    int gameIndex) : id(id), damageClassId(damageClassId), isBattleOnly(isBattleOnly), gameIndex(gameIndex) {}

  // Getters
  int getId() const { return id; }
  int getDamageClassId() const { return damageClassId; }
  std::string getIdentifier() const { return identifier; }
  bool getIsBattleOnly() const { return isBattleOnly; }
  int getGameIndex() const { return gameIndex; }

  // Setters
  // void setId(int newId) { id = newId; }
  // void setDamageClassId(int newDamageClassId) { damageClassId = newDamageClassId; }
  // void setIdentifier(const std::string& newIdentifier) { identifier = newIdentifier; }
  // void setIsBattleOnly(bool newIsBattleOnly) { isBattleOnly = newIsBattleOnly; }
  // void setGameIndex(int newGameIndex) { gameIndex = newGameIndex; }

  void parseFile(const std::string& filename) override {
    // Parse the stats.csv file here
  }
};

class pokemon_stats : public CsvFile {
private:
  int pokemon_id;
  int stat_id;
  int base_stat;
  int effort;
public:
  pokemon_stats(int pokemon_id,
    int stat_id,
    int base_stat,
    int effort) : pokemon_id(pokemon_id), stat_id(stat_id), base_stat(base_stat), effort(effort) {}
  // Getters
  int getPokemonId() const {
    return pokemon_id;
  }

  int getStatId() const {
    return stat_id;
  }

  int getBaseStat() const {
    return base_stat;
  }

  int getEffort() const {
    return effort;
  }
  void parseFile(const std::string& filename) override {
    // Parse the pokemon_stats.csv file here
  }
};

class type_names : public CsvFile {
private:
  int type_id;
  int local_language_id;
  std::string name;

public:
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

  void parseFile(const std::string& filename) override {
    // Parsing logic here
  }
};


class Experience : public CsvFile {
private:
  int growth_rate_id;
  int level;
  int experience;

public:
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

  void parseFile(const std::string& filename) override {
    // Parsing logic here
  }
};


class pokemon_species : public CsvFile {
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
  // Constructor, if needed
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

  void parseFile(const std::string& filename) override {
    // Parse the pokemon_species.csv file here
  }
};

class pokeman_moves : public CsvFile {
private:
  int pokemon_id;
  int version_group_id;
  int move_id;
  int pokemon_move_method_id;
  int level;
  int order;

public:
  // Constructor
  pokeman_moves(int pokemon_id, int version_group_id, int move_id,
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

  void parseFile(const std::string& filename) override {
    // Parse the pokeman_moves.csv file here
  }
};


class moves : public CsvFile {
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

  void parseFile(const std::string& filename) override {
    // Parse the moves.csv file here
  }
};
class pokemon_types : public CsvFile {
private:
  int pokemon_id;
  int type_id;
  int slot;
public:
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
  
  void parseFile(const std::string& filename) override {
    // Parse the pokemon_types.csv file here
  }
};
// Factory
class CsvFactory {
public:
  using Creator = std::function<std::unique_ptr<CsvFile>()>;

  static CsvFactory& instance() {
    static CsvFactory instance;
    return instance;
  }

  void registerType(const std::string& typeName, Creator creator) {
    creators[typeName] = creator;
  }

  std::unique_ptr<CsvFile> create(const std::string& typeName) {
    if (creators.find(typeName) != creators.end()) {
      return creators[typeName]();
    }
    // Handle error or return nullptr
    std::cerr << "Unknown CSV type: " << typeName << std::endl;
    return nullptr;
  }

private:
  std::unordered_map<std::string, Creator> creators;
  CsvFactory() {} // Constructor made private for singleton
};
int new_map(int teleport);
void pathfind(map* m);

#endif
