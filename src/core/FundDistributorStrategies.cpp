#include "FundDistributorStrategies.h"
#include "Share.h"
#include <cmath>
#include <numeric>
#include <algorithm>
#include <iterator> 
#include <iostream>

// ==========================================
// EqualFundDistributor
// ==========================================

std::vector<double> EqualFundDistributor::distribute(const std::vector<Share*>& shares, double totalAmount) const {
    if (shares.empty()) return {};
    
    double amountPerShare = totalAmount / shares.size();
    return std::vector<double>(shares.size(), amountPerShare);
}

// ==========================================
// MarkovChainFundDistributor
// ==========================================

int MarkovChainFundDistributor::getState(double currentPrice, double prevPrice) const {
    // 1 = Bull (Price increased), 0 = Bear (Price decreased or same)
    return (currentPrice > prevPrice) ? 1 : 0;
}

void MarkovChainFundDistributor::updateShareHistory(ShareRecord& record, const std::vector<double>& history) const {
    // If not enough data to form a transition (yesterday -> today), exit.
    // This leaves record.lastState at its default value (-1).
    if (history.size() < 3) return;

    // INITIALIZATION (Lazy / First time)
    if (record.processedDataPoints == 0) {
        // Initialize lastState safely using real data from day 2
        record.lastState = getState(history[1], history[0]); 

        // Mark first two points as used
        record.processedDataPoints = 2; 
    }

    // INCREMENTAL UPDATE
    for (size_t i = record.processedDataPoints; i < history.size(); ++i) {
        int currentState = getState(history[i], history[i-1]);
        
        // Flattened Index: Prev * 2 + Curr
        // IMPORTANT: Here we assume lastState is guaranteed to be 0 or 1.
        int transitionIndex = (record.lastState * 2) + currentState;
        
        record.transitionCounts[transitionIndex]++;
        record.lastState = currentState;
    }

    record.processedDataPoints = history.size();
}

void MarkovChainFundDistributor::registerShare(const Share* share) {
    if (!share) return;
    int id = share->getID();

    if (shareRegistry.find(id) != shareRegistry.end()) return;

    ShareRecord& record = shareRegistry[id]; 
    record.companyName = share->getCompany();
    record.sharePtr = share;
    
    updateShareHistory(record, share->getPriceHistory());
}

void MarkovChainFundDistributor::unregisterShare(int shareID) {
    shareRegistry.erase(shareID);
}

std::vector<double> MarkovChainFundDistributor::distribute(const std::vector<Share*>& shares, double totalAmount) const {    
    if (shares.empty()) return {};

    // =========================================================
    // 1. GLOBAL FALLBACK: Are we at the beginning of the simulation?
    // =========================================================
    // If the first share has less than 3 days of history, we assume 
    // the market has just started. We use EqualFundDistributor to avoid 
    // noise or crashes due to lack of data.
    if (shares[0]->getPriceHistory().size() < 3) {
        EqualFundDistributor fallbackStrategy;
        return fallbackStrategy.distribute(shares, totalAmount);
    }
    // =========================================================

    std::vector<double> probs;
    probs.reserve(shares.size());

    for (const Share* share : shares) {
        // 2. LOOKUP & LAZY REGISTRATION
        auto it = shareRegistry.find(share->getID());
        
        if (it == shareRegistry.end()) {
            ShareRecord& newRecord = shareRegistry[share->getID()]; 
            newRecord.companyName = share->getCompany();
            newRecord.sharePtr = share;
            
            updateShareHistory(newRecord, share->getPriceHistory());
            it = shareRegistry.find(share->getID());
        }

        ShareRecord& record = it->second;

        // 3. UPDATE HISTORY
        updateShareHistory(record, share->getPriceHistory());

        // =========================================================
        // 4. INDIVIDUAL SAFETY CHECK (Anti-Crash)
        // =========================================================
        // Even with the global fallback, if a NEW share enters mid-simulation,
        // it will have lastState = -1.
        // If lastState is -1, we cannot calculate array indices.
        if (record.lastState == -1) {
            probs.push_back(0.5); // Neutral probability (total uncertainty)
            continue;             // Skip to the next share
        }
        // =========================================================

        // 5. PROBABILITY CALCULATION (Now guaranteed to be safe)
        int rowStart = record.lastState * 2;
        int countBull = record.transitionCounts[rowStart + 1];
        int totalTransitions = record.transitionCounts[rowStart + 0] + countBull;

        double p = (totalTransitions == 0) ? 0.5 : (static_cast<double>(countBull) / totalTransitions);
        probs.push_back(p);
    }

    // --- Normalization ---
    double sumProbs = std::accumulate(probs.begin(), probs.end(), 0.0);
    std::vector<double> result(shares.size());

    // If all probabilities are 0 (rare with Laplace, but possible), distribute equally
    if (sumProbs == 0.0) {
        EqualFundDistributor fallback;
        return fallback.distribute(shares, totalAmount);
    }

    double inverseSum = totalAmount / sumProbs;
    std::transform(probs.begin(), probs.end(), result.begin(),
        [inverseSum](double p) { return p * inverseSum; });

    return result;
}