#include "FundsDistributorManager.h"
#include "Share.h" // Required because Share is used in vector template

FundsDistributorManager::FundsDistributorManager() {
    // Default strategy
    currentDistributor = std::make_unique<EqualFundDistributor>();
}

void FundsDistributorManager::setDistributor(DistributorType type) {
    switch (type) {
        case DistributorType::EQUAL:
            currentDistributor = std::make_unique<EqualFundDistributor>();
            break;
        case DistributorType::MARKOV_CHAIN:
            currentDistributor = std::make_unique<MarkovChainFundDistributor>();
            break;
    }
}

std::vector<double> FundsDistributorManager::distribute(const std::vector<Share*>& shares, double amount) const {
    if (!currentDistributor) return {};
    
    // Pass by const reference explicitly handled in interface
    return currentDistributor->distribute(shares, amount);
}