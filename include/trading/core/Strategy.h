#ifndef STRATEGY_H
#define STRATEGY_H

#include <vector>
#include <string>
#include "FundsDistributorManager.h" 

// Forward declaration
class Share;

class Strategy {
    public:
        // DATA MEMBERS
        // Grouped by type/size to optimize memory padding
        std::vector<int> id;                 
        std::vector<double> fppsr; 
        FundsDistributorManager distributorManager;

        // CONSTRUCTORS
        Strategy() = delete;
        // Pass vector by const reference
        Strategy(const std::vector<int>& strategyId);
    
        // METHODS
        
        /**
         * @brief Clusters shares into risk groups based on volatility/metrics.
         * @param shares Input vector of shares (const reference).
         * @return Matrix of shares grouped by risk.
         */
        std::vector<std::vector<Share*>> associateShareToEachRiskGroup(const std::vector<Share*>& shares) const;
        
        /**
         * @brief Calculates funds for each share based on risk groups.
         * @param sharesClusteredInEachGroup The grouped shares matrix. 
         * PASSED BY CONST REFERENCE to avoid expensive deep copies.
         * @param funds Total funds available.
         */
        std::vector<std::vector<double>> distributeFundInEachShare(const std::vector<std::vector<Share*>>& sharesClusteredInEachGroup, double funds);
        
        void setDistributionLogic(DistributorType type);

    private: 
        // Helper logic - does not modify object state -> const
        // Takes const Share* as it only reads share data
        int calculateRiskOfShare (const Share* share) const;
};

#endif