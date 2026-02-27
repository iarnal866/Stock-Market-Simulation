#include "DataLoader.h"
#include "Market.h"  // Required for full definitions
#include "Trader.h"  // Required for full definitions
#include <fstream>
#include <iostream>
#include <json.hpp>

using json = nlohmann::json;

int DataLoader::loadConfig(const std::string& filename, Market& market, std::vector<std::unique_ptr<Trader>>& traders) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("File can not be opened: " + filename);
    }

    json j;
    file >> j;

    int simulationSteps = 100; // Valor por defecto
    if (j.contains("simulation") && j["simulation"].contains("days")) {
        simulationSteps = j["simulation"]["days"].get<int>();
    }

    // 1. Load Shares
    std::vector<std::unique_ptr<Share>> shares;
    // Map for O(1) lookup during trader creation
    std::unordered_map<std::string, Share*> shareMap; 
    
    // Optimistic reservation to prevent reallocations
    if (j.contains("shares")) shares.reserve(j["shares"].size());

    double* timeStep = market.getTimeStep();

    for (const auto& item : j["shares"]) {
        auto share = std::make_unique<Share>(
            item["name"], 
            item["price"], 
            item["drift"], 
            item["vol"], 
            item["var"], 
            item["lambda"], 
            item["volume"], 
            timeStep
        );
        shareMap[share->getCompany()] = share.get(); 
        shares.push_back(std::move(share));
    }
    
    // Transfer ownership to Market
    market.registerShares(std::move(shares));    

    // 2. Load Traders
    if (j.contains("traders")) traders.reserve(j["traders"].size());

    for (const auto& t_item : j["traders"]) {
        std::vector<Share*> walletShares;
        
        // Link shares to trader wallet
        for (const auto& shareName : t_item["wallet_shares"]) {
            if (shareMap.find(shareName) != shareMap.end()) {
                walletShares.push_back(shareMap[shareName]);
            } else {
                std::cerr << "Warning: Share " << shareName << " not found for trader " << t_item["id"] << std::endl;
            }
        }

        int traderID = t_item["id"].get<int>(); 
        std::vector<int> strategyParams = t_item["strategy"].get<std::vector<int>>();

        // Create Wallet
        Wallet wallet(t_item["capital"], strategyParams, walletShares);

        // Create Trader
        std::unique_ptr<Trader> newTrader = std::make_unique<Trader>(traderID, std::move(wallet));
        traders.push_back(std::move(newTrader));
    }

    // Register raw pointers with Market for simulation loop
    market.registerTraders([&]() 
    {
        std::vector<Trader*> ptrs;
        ptrs.reserve(traders.size()); // Optimization: Reserve memory
        for (const auto& t : traders) {
            ptrs.push_back(t.get());
        }
        return ptrs;
    }());

    std::cout << "Successfully loaded " << simulationSteps << " days from: " << filename << std::endl;
    return simulationSteps;
}