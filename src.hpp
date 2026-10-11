#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <cstring>

class BasicException {
protected:
    const char *message;

public:
    explicit BasicException(const char *_message) : message(_message) {}

    virtual const char *what() const {
        return message;
    }
};

class ArgumentException: public BasicException {
public:
    explicit ArgumentException(const char *_message) : BasicException(_message) {}
};

class IteratorException: public BasicException {
public:
    explicit IteratorException(const char *_message) : BasicException(_message) {}
};

struct Pokemon {
    char name[12];
    int id;
    std::vector<std::string> types;
    
    Pokemon() : id(0) {
        name[0] = '\0';
    }
    
    Pokemon(const char *_name, int _id, const std::vector<std::string>& _types) : id(_id), types(_types) {
        strncpy(name, _name, 11);
        name[11] = '\0';
    }
};

class Pokedex {
private:
    std::map<int, Pokemon> pokemonMap; // Sorted by id
    std::string fileName;
    
    // Type damage relationships
    std::map<std::string, std::map<std::string, float>> typeChart;
    
    void initializeTypeChart() {
        // Fire
        typeChart["fire"]["grass"] = 2.0f;
        typeChart["fire"]["ice"] = 2.0f;
        typeChart["fire"]["fire"] = 0.5f;
        typeChart["fire"]["water"] = 0.5f;
        
        // Water
        typeChart["water"]["fire"] = 2.0f;
        typeChart["water"]["ground"] = 2.0f;
        typeChart["water"]["water"] = 0.5f;
        typeChart["water"]["grass"] = 0.5f;
        
        // Grass
        typeChart["grass"]["water"] = 2.0f;
        typeChart["grass"]["ground"] = 2.0f;
        typeChart["grass"]["fire"] = 0.5f;
        typeChart["grass"]["grass"] = 0.5f;
        typeChart["grass"]["flying"] = 0.5f;
        
        // Electric
        typeChart["electric"]["water"] = 2.0f;
        typeChart["electric"]["flying"] = 2.0f;
        typeChart["electric"]["electric"] = 0.5f;
        typeChart["electric"]["ground"] = 0.0f;
        
        // Ice
        typeChart["ice"]["grass"] = 2.0f;
        typeChart["ice"]["ground"] = 2.0f;
        typeChart["ice"]["flying"] = 2.0f;
        typeChart["ice"]["fire"] = 0.5f;
        typeChart["ice"]["ice"] = 0.5f;
        
        // Ground
        typeChart["ground"]["fire"] = 2.0f;
        typeChart["ground"]["electric"] = 2.0f;
        typeChart["ground"]["grass"] = 0.5f;
        typeChart["ground"]["flying"] = 0.0f;
        
        // Flying
        typeChart["flying"]["grass"] = 2.0f;
        typeChart["flying"]["electric"] = 0.5f;
    }
    
    bool isValidName(const char *name) const {
        if (!name || strlen(name) == 0 || strlen(name) > 10) return false;
        for (int i = 0; name[i]; i++) {
            if (!isalpha(name[i])) return false;
        }
        return true;
    }
    
    bool isValidType(const std::string &type) const {
        const std::set<std::string> validTypes = {"fire", "water", "grass", "electric", "ice", "ground", "flying"};
        return validTypes.find(type) != validTypes.end();
    }
    
    std::vector<std::string> parseTypes(const char *types) const {
        std::vector<std::string> result;
        std::string s(types);
        std::stringstream ss(s);
        std::string token;
        
        while (std::getline(ss, token, '#')) {
            if (!isValidType(token)) {
                throw ArgumentException(("Argument Error: PM Type Invalid (" + token + ")").c_str());
            }
            result.push_back(token);
        }
        
        if (result.empty() || result.size() > 7) {
            throw ArgumentException("Argument Error: PM Type Invalid");
        }
        
        return result;
    }
    
    void saveToFile() {
        std::ofstream file(fileName);
        if (!file.is_open()) return;
        
        for (const auto& pair : pokemonMap) {
            const Pokemon& pm = pair.second;
            file << pm.name << " " << pm.id;
            for (size_t i = 0; i < pm.types.size(); i++) {
                if (i == 0) file << " ";
                else file << "#";
                file << pm.types[i];
            }
            file << std::endl;
        }
        
        file.close();
    }
    
    void loadFromFile() {
        std::ifstream file(fileName);
        if (!file.is_open()) return;
        
        pokemonMap.clear();
        
        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string name, typesStr;
            int id;
            
            ss >> name >> id;
            std::getline(ss, typesStr);
            if (!typesStr.empty() && typesStr[0] == ' ') {
                typesStr = typesStr.substr(1);
            }
            
            try {
                std::vector<std::string> types = parseTypes(typesStr.c_str());
                Pokemon pm(name.c_str(), id, types);
                pokemonMap[id] = pm;
            } catch (...) {
                // Skip invalid entries
            }
        }
        
        file.close();
    }

public:
    explicit Pokedex(const char *_fileName) : fileName(_fileName) {
        initializeTypeChart();
        loadFromFile();
    }

    ~Pokedex() {
        saveToFile();
    }

    bool pokeAdd(const char *name, int id, const char *types) {
        if (!isValidName(name)) {
            throw ArgumentException(("Argument Error: PM Name Invalid (" + std::string(name) + ")").c_str());
        }
        
        if (id <= 0) {
            throw ArgumentException(("Argument Error: PM ID Invalid (" + std::to_string(id) + ")").c_str());
        }
        
        std::vector<std::string> parsedTypes;
        try {
            parsedTypes = parseTypes(types);
        } catch (ArgumentException &e) {
            throw;
        }
        
        // Check if name already exists
        for (const auto& pair : pokemonMap) {
            if (strcmp(pair.second.name, name) == 0) {
                return false;
            }
        }
        
        // Check if id already exists
        if (pokemonMap.find(id) != pokemonMap.end()) {
            return false;
        }
        
        Pokemon pm(name, id, parsedTypes);
        pokemonMap[id] = pm;
        return true;
    }

    bool pokeDel(int id) {
        if (id <= 0) {
            throw ArgumentException(("Argument Error: PM ID Invalid (" + std::to_string(id) + ")").c_str());
        }
        
        auto it = pokemonMap.find(id);
        if (it != pokemonMap.end()) {
            pokemonMap.erase(it);
            return true;
        }
        return false;
    }

    std::string pokeFind(int id) const {
        if (id <= 0) {
            throw ArgumentException(("Argument Error: PM ID Invalid (" + std::to_string(id) + ")").c_str());
        }
        
        auto it = pokemonMap.find(id);
        if (it != pokemonMap.end()) {
            return std::string(it->second.name);
        }
        return "None";
    }

    std::string typeFind(const char *types) const {
        std::vector<std::string> searchTypes;
        try {
            std::string s(types);
            if (s.empty()) {
                throw ArgumentException("Argument Error: PM Type Invalid");
            }
            searchTypes = parseTypes(types);
        } catch (ArgumentException &e) {
            throw;
        }
        
        std::vector<Pokemon> results;
        
        for (const auto& pair : pokemonMap) {
            const Pokemon& pm = pair.second;
            bool hasAllTypes = true;
            
            for (const std::string& searchType : searchTypes) {
                bool found = false;
                for (const std::string& pmType : pm.types) {
                    if (pmType == searchType) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    hasAllTypes = false;
                    break;
                }
            }
            
            if (hasAllTypes) {
                results.push_back(pm);
            }
        }
        
        if (results.empty()) {
            return "None";
        }
        
        std::sort(results.begin(), results.end(), [](const Pokemon& a, const Pokemon& b) {
            return a.id < b.id;
        });
        
        std::string result = std::to_string(results.size());
        for (const Pokemon& pm : results) {
            result += "\n" + std::string(pm.name);
        }
        
        return result;
    }

    float attack(const char *type, int id) const {
        if (!isValidType(type)) {
            throw ArgumentException(("Argument Error: PM Type Invalid (" + std::string(type) + ")").c_str());
        }
        
        if (id <= 0) {
            throw ArgumentException(("Argument Error: PM ID Invalid (" + std::to_string(id) + ")").c_str());
        }
        
        auto it = pokemonMap.find(id);
        if (it == pokemonMap.end()) {
            return -1.0f;
        }
        
        const Pokemon& defender = it->second;
        float totalDamage = 1.0f;
        
        for (const std::string& defType : defender.types) {
            auto attackIt = typeChart.find(type);
            if (attackIt != typeChart.end()) {
                auto defIt = attackIt->second.find(defType);
                if (defIt != attackIt->second.end()) {
                    totalDamage *= defIt->second;
                }
            }
        }
        
        return totalDamage;
    }

    int catchTry() const {
        if (pokemonMap.empty()) {
            return 0;
        }
        
        std::set<int> owned;
        std::set<int> canCatch;
        
        // Start with the smallest ID
        owned.insert(pokemonMap.begin()->first);
        
        bool newCatch = true;
        while (newCatch) {
            newCatch = false;
            
            for (const auto& pair : pokemonMap) {
                int targetId = pair.first;
                if (owned.find(targetId) != owned.end() || canCatch.find(targetId) != canCatch.end()) {
                    continue;
                }
                
                const Pokemon& target = pair.second;
                
                // Check if any owned Pokemon can catch this one
                for (int ownedId : owned) {
                    const Pokemon& catcher = pokemonMap.at(ownedId);
                    
                    // Try each type of the catcher
                    for (const std::string& attackType : catcher.types) {
                        float damage = attack(attackType.c_str(), targetId);
                        if (damage >= 2.0f) {
                            canCatch.insert(targetId);
                            newCatch = true;
                            break;
                        }
                    }
                    if (canCatch.find(targetId) != canCatch.end()) {
                        break;
                    }
                }
            }
            
            // Add newly caught Pokemon to owned
            for (int newId : canCatch) {
                owned.insert(newId);
            }
        }
        
        return owned.size();
    }

    struct iterator {
    private:
        std::map<int, Pokemon>::iterator it;
        std::map<int, Pokemon>* container;
        
    public:
        iterator(std::map<int, Pokemon>::iterator _it, std::map<int, Pokemon>* _container) 
            : it(_it), container(_container) {}
        
        iterator &operator++() {
            if (container && it != container->end()) {
                ++it;
            } else {
                throw IteratorException("Iterator Error: Invalid operation");
            }
            return *this;
        }
        
        iterator &operator--() {
            if (container && it != container->begin()) {
                --it;
            } else {
                throw IteratorException("Iterator Error: Invalid operation");
            }
            return *this;
        }
        
        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }
        
        iterator operator--(int) {
            iterator temp = *this;
            --(*this);
            return temp;
        }
        
        iterator & operator = (const iterator &rhs) {
            it = rhs.it;
            container = rhs.container;
            return *this;
        }
        
        bool operator == (const iterator &rhs) const {
            return it == rhs.it;
        }
        
        bool operator != (const iterator &rhs) const {
            return it != rhs.it;
        }
        
        Pokemon & operator*() const {
            if (container && it != container->end()) {
                return it->second;
            }
            throw IteratorException("Iterator Error: Invalid operation");
        }
        
        Pokemon *operator->() const {
            if (container && it != container->end()) {
                return &(it->second);
            }
            throw IteratorException("Iterator Error: Invalid operation");
        }
        
        friend class Pokedex;
    };

    iterator begin() {
        return iterator(pokemonMap.begin(), &pokemonMap);
    }

    iterator end() {
        return iterator(pokemonMap.end(), &pokemonMap);
    }
};