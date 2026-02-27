#ifndef DATALOADER_H
#define DATALOADER_H

#include <string>
#include <vector>
#include <memory>

// Forward declarations to reduce compilation dependencies
class Market;
class Trader;

class DataLoader {
public:
    /**
     * @brief Loads the initial configuration from a file.
     * @param filename Path to the configuration file (passed by const reference).
     * @param market Reference to the Market object to be populated.
     * @param traders Reference to the vector of Trader pointers to be populated.
     */
    static int loadConfig(const std::string& filename, Market& market, std::vector<std::unique_ptr<Trader>>& traders);
};

#endif