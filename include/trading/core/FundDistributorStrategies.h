#ifndef FUND_DISTRIBUTOR_STRATEGIES_H
#define FUND_DISTRIBUTOR_STRATEGIES_H

#include <vector>
#include <map>
#include <string>
#include <array>

// Forward declaration
class Share;

class IFundDistributor {
public:
    virtual ~IFundDistributor() = default;

    /**
     * @brief Calculates fund distribution logic.
     * @param shares Vector of Share pointers (passed by const reference).
     * @param totalAmount Total capital available for distribution.
     * @return Vector of allocated amounts per share.
     */
    virtual std::vector<double> distribute(const std::vector<Share*>& shares, double totalAmount) const = 0;
};


class EqualFundDistributor : public IFundDistributor {
public:
    std::vector<double> distribute(const std::vector<Share*>& shares, double totalAmount) const override;
};


class MarkovChainFundDistributor : public IFundDistributor {
private:
    // Internal struct to hold the incremental state of a registered Share
    struct ShareRecord {
        std::string companyName;       
        const Share* sharePtr;         // Non-owning pointer for reference
        
        // Flattened 2x2 Transition Matrix [Bear->Bear, Bear->Bull, Bull->Bear, Bull->Bull]
        std::array<int, 4> transitionCounts = {1, 1, 1, 1};
        int lastState = -1;             // 0: Bear, 1: Bull
        size_t processedDataPoints = 0; // Tracks how much history has been processed
    };

    // The Registry: Maps Share ID -> ShareRecord.
    // Marked 'mutable' to allow state updates during the 'const' distribute call.
    mutable std::map<int, ShareRecord> shareRegistry; 

    // Helper methods
    int getState(double currentPrice, double prevPrice) const;
    
    /**
     * @brief Incrementally updates the transition matrix for a specific record.
     * @param record Reference to the share's record in the registry.
     * @param history The full price history of the share.
     */
    void updateShareHistory(ShareRecord& record, const std::vector<double>& history) const;

public:
    // --- REGISTRY MANAGEMENT ---
    
    /**
     * @brief Registers a share in the strategy and pre-calculates its existing history.
     * This should be called before the simulation loop to avoid performance spikes.
     */
    void registerShare(const Share* share);

    /**
     * @brief Removes a share from the registry to free memory.
     */
    void unregisterShare(int shareID);


    // --- DISTRIBUTION LOGIC ---
    std::vector<double> distribute(const std::vector<Share*>& shares, double totalAmount) const override;
};

#endif