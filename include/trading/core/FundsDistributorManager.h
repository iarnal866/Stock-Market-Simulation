#ifndef FUNDS_DISTRIBUTOR_MANAGER_H
#define FUNDS_DISTRIBUTOR_MANAGER_H

#include <memory>
#include <vector>
#include "FundDistributorStrategies.h" 

enum class DistributorType {
    EQUAL,
    MARKOV_CHAIN
};

class FundsDistributorManager {
private:
    // Unique pointer to the strategy interface (8 bytes on 64-bit systems)
    std::unique_ptr<IFundDistributor> currentDistributor;

public:
    FundsDistributorManager(); 

    void setDistributor(DistributorType type);
    
    /**
     * @brief Executes the current distribution strategy.
     * @param shares Input shares (const reference).
     * @param amount Total funds.
     */
    std::vector<double> distribute(const std::vector<Share*>& shares, double amount) const;
};

#endif