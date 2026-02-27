#include "Strategy.h"
#include "Share.h" // Full definition required here
#include <iostream>
#include <stdexcept>
#include <string>
#include <algorithm> // For std::min

// Constructor: Accepts vector by const reference
Strategy::Strategy(const std::vector<int>& strategyParams) 
{
    if (strategyParams.size() < 2) {
        throw std::invalid_argument("Configuration error: Strategy vector must have [RiskID, DistributionID].");
    }

    // Initialize ID vector
    id = {strategyParams[0]}; 

    int idGroupRisk = strategyParams[0];
    int idInsideGroupRisk = strategyParams[1];

    // Initialize Risk Profile (FPPSR: Funds Percentage Per Share Risk)
    switch (idGroupRisk) {
        case 1: // Conservative
            fppsr = {0.6, 0.3, 0.1}; 
            break;
        case 2: // Balanced
            fppsr = {0.3, 0.5, 0.2}; 
            break;
        case 3: // Risky
            fppsr = {0.1, 0.4, 0.5}; 
            break;
        case 4: // Equal
            fppsr = {1.0/3.0, 1.0/3.0, 1.0/3.0}; 
            break;
        default:
            throw std::invalid_argument("Invalid Risk Strategy ID: " + std::to_string(idGroupRisk));            
    }

    // Initialize Distribution Logic
    switch (idInsideGroupRisk) {
        case 1: 
            setDistributionLogic(DistributorType::EQUAL);
            break;
        case 2: 
            setDistributionLogic(DistributorType::MARKOV_CHAIN);
            break;
        default: 
            throw std::invalid_argument("Invalid Distribution Logic ID: " + std::to_string(idInsideGroupRisk));            
    }
}

void Strategy::setDistributionLogic(DistributorType type) {
    distributorManager.setDistributor(type);
}

// CRITICAL OPTIMIZATION: Accepts 'sharesClusteredInEachGroup' by const reference.
// Prevents copying the nested vector structure.
std::vector<std::vector<double>> Strategy::distributeFundInEachShare(const std::vector<std::vector<Share*>>& sharesClusteredInEachGroup, double funds)
{
    std::vector<std::vector<double>> moneyInvestedInEachShare(sharesClusteredInEachGroup.size());
    
    // 1. Calculate Active Weight (Normalization factor)
    double totalActiveWeight = 0.0;
    size_t loopLimit = std::min(sharesClusteredInEachGroup.size(), fppsr.size());

    for (size_t i = 0; i < loopLimit; i++) {
        // Access via reference to avoid copy overhead
        if (!sharesClusteredInEachGroup[i].empty()) {
            totalActiveWeight += fppsr[i];
        }
    }

    if (totalActiveWeight == 0.0) return moneyInvestedInEachShare;

    // 2. Distribute funds per group
    for (size_t i = 0; i < loopLimit; i++)
    {
        // Use const reference to access the inner vector
        const auto& groupShares = sharesClusteredInEachGroup[i];
        
        if (groupShares.empty()) {
            moneyInvestedInEachShare[i] = {};
            continue;
        }

        // Calculate funds allocated to this specific risk group
        double normalizedPercentage = fppsr[i] / totalActiveWeight;
        double moneyForThisGroup = funds * normalizedPercentage;

        // Delegate specific distribution to the Manager
        moneyInvestedInEachShare[i] = distributorManager.distribute(groupShares, moneyForThisGroup);
    }

    return moneyInvestedInEachShare;
}

// Optimization: Accepts shares by const reference
std::vector<std::vector<Share*>> Strategy::associateShareToEachRiskGroup(const std::vector<Share*>& shares) const
{
    // Create buckets for 3 risk levels (Low, Medium, High)
    std::vector<std::vector<Share*>> sharesClusteredInEachGroup(3); // Hardcoded 3 based on Risk Logic
    
    for (const auto& share : shares) {
        int riskLevel = calculateRiskOfShare(share);
        if(riskLevel >= 0 && riskLevel < 3) {
            sharesClusteredInEachGroup[riskLevel].push_back(share);
        }
    }
    return sharesClusteredInEachGroup;
}

// Helper marked as const
int Strategy::calculateRiskOfShare(const Share* share) const
{
    double volatility = share->getVolatility();
    if (volatility < 0.20) return 0; // Low Risk
    if (volatility < 0.45) return 1; // Medium Risk
    return 2;                        // High Risk
}