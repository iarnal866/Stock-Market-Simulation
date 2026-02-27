#include "Market.h"
#include "Share.h"
#include "Trader.h"
#include "Order.h"

#include <vector>
#include <algorithm> 
#include <iterator> 
#include <iomanip>
#include <iostream>

// Constructor
Market::Market(double timeStep)
    : 
    shares{}, 
    dt{timeStep},
    registeredTraders{},
    sharesBuySellHistory{}
{
}

// Transfer ownership of Share objects into the Market.
// Uses move iterators to transfer unique_ptrs from the input vector
// to the Market's internal storage without copying.
void Market::registerShares(std::vector<std::unique_ptr<Share>> p_shares){

    shares.insert(shares.end(), std::make_move_iterator(p_shares.begin()), std::make_move_iterator(p_shares.end()));

    setSharesIDbasedInThePositions();
}

// Adds multiple traders to the Market's registry.
// optimizes by reserving memory upfront to prevent multiple 
// reallocations as the internal vector grows.
void Market::registerTraders(const std::vector<Trader*>& p_registeredTraders)
{
    // Reserve space to avoid multiple reallocations
    registeredTraders.reserve(registeredTraders.size() + p_registeredTraders.size());
    registeredTraders.insert(registeredTraders.end(), p_registeredTraders.begin(), p_registeredTraders.end());
}

void Market::runSimulation(int steps)
{
    for(int day = 0; day<steps; day++)
    {
        calculateNextStep(day);
    }
}

/*
Core Simulation Logic for a single time increment
    1. Collects all orders from all registered traders.
    2. Aggregates the total buy/sell volume for every share.
    3. Updates each share's price based on the net volume (supply vs demand)
    4. Records the volume data into the history log.
*/
void Market::calculateNextStep(int day)
{
    std::vector<double> netVolumenPerShare(shares.size(),0.0);
    std::vector<Order> totalOrdersOfThisTimeStep;

    // Optimization: estimate order count to prevent reallocations
    // Assume each trader makes at least 1 order
    totalOrdersOfThisTimeStep.reserve(registeredTraders.size());
    
    for (Trader* t : registeredTraders)
    {
        std::vector<Order> orders = t->executeOrders(day);
        totalOrdersOfThisTimeStep.insert(totalOrdersOfThisTimeStep.end(),std::make_move_iterator(orders.begin()),std::make_move_iterator(orders.end()));
    }
    // Convert the list of individual orders into a summarized volume per share
    updateNetVolumenPerShareBasedOnTheOrders(netVolumenPerShare, totalOrdersOfThisTimeStep);
    // Apply the market impact: high net volume usually drives prices up
    for (size_t shareIndex = 0; shareIndex < shares.size(); shareIndex++)
    {
        shares[shareIndex]->updatePrice(netVolumenPerShare[shareIndex]);
    }

    sharesBuySellHistory.push_back(std::move(netVolumenPerShare));
}


// Uses the Share's ID to find the correct index in the 'netVolumePerShare' vector
void Market::updateNetVolumenPerShareBasedOnTheOrders(std::vector<double>& netVolumenPerShare, const std::vector<Order>& totalOrdersOfThisTimeStep) const
{
    for (const Order & o : totalOrdersOfThisTimeStep)
    {
        double quantity = o.quantity;
        Share* sharePtr = o.p_share;
        int shareIDposition = sharePtr->getID();
        int trader_ID = o.trader_ID ;


        try 
        {
            netVolumenPerShare.at(shareIDposition) += quantity;
        } 
        catch (const std::out_of_range& e) 
        {
            std::cerr << "[WARNING] Market: Share ID (" << shareIDposition 
              << ") is out of bounds for 'netVolumenPerShare' vector (size: " 
              << netVolumenPerShare.size() << ")." << std::endl;
        }
    }
}

// Add a single trader to the market participant list.
void Market::registerTrader(Trader* trader)
{
    registeredTraders.push_back(trader);
}

// Synchronizes the internal IDs of Share objects with their vector index
void Market::setSharesIDbasedInThePositions()
{
    for (int shareIndex = 0; shareIndex<shares.size() ; shareIndex++)
    {
        shares[shareIndex]->setID(shareIndex);
    }
}

// Returns a pointer to the simulation's time step.
double* Market::getTimeStep() {return &dt; }

// Retrieves the full price history for a specific share ID.
const std::vector<double>& Market::getPriceHistoryOfAGivenShare(int shareID) const
{
    std::vector<double> sharePriceHistory = shares[shareID]->getPriceHistory();
    return sharePriceHistory;

}

// Returns a vector of raw pointers to shares
std::vector<Share*> Market::getShares() const {
    std::vector<Share*> ptr_shares;
    ptr_shares.reserve(shares.size());
   // Optimization: Using std::transform instead of manual for loop
    std::transform(shares.begin(), shares.end(), std::back_inserter(ptr_shares),
                   [](const std::unique_ptr<Share>& s) { return s.get(); });

    return ptr_shares;
};

// Prints a formatted table showing the volume activity of every share 
// for every day of the simulation.
void Market::showBuySellHistoryOfShares() const
{
    if (sharesBuySellHistory.empty()) {
        std::cout << "No trading history available." << std::endl;
        return;
    }

    const int colWidth = 6; 
    const int dayWidth = 6; 
    size_t numShares = sharesBuySellHistory[0].size(); 

    std::cout << "\nBUY/SELL OF EVERY SHARE IN EACH DAY" << std::endl;
    std::cout << std::string(dayWidth + (colWidth + 2) * numShares + 1, '=') << std::endl;

    std::cout << std::left << std::setw(dayWidth) << "Day" << "|";
    
    for (size_t i = 0; i < numShares; ++i) {
        std::string header = std::to_string(i);
        std::cout << std::right << std::setw(colWidth) << header << " |";
    }
    std::cout << std::endl;

    std::cout << std::string(dayWidth + (colWidth + 2) * numShares + 1, '-') << std::endl;

    for (size_t day = 0; day < sharesBuySellHistory.size(); ++day) 
    {
        std::cout << std::left << std::setw(dayWidth) << day << "|";

        const auto& dayData = sharesBuySellHistory[day];
        
        for (size_t shareID = 0; shareID < numShares; ++shareID) 
        {
            if (shareID < dayData.size()) {
                std::cout << std::right 
                          << std::setw(colWidth) 
                          << std::fixed << std::setprecision(2) << dayData[shareID] 
                          << " |";
            } else {
                std::cout << std::right << std::setw(colWidth) << "N/A" << " |";
            }
        }
        std::cout << std::endl;
    }
}

// Print a summary of all shares currently in the market,
// including their current price, volatility, and drift.
void Market::plotAllShares() const {
    std::cout << std::endl;
    std::cout << "==================== MARKET SHARES OVERVIEW ====================" << std::endl;


    std::cout << std::left 
              << std::setw(5)  << "ID" 
              << std::setw(20) << "Company" 
              << std::setw(12) << "Price" 
              << std::setw(12) << "Vol (%)" 
              << std::setw(12) << "Drift"
              << std::endl;

    std::cout << std::string(65, '-') << std::endl; 
    for (const auto& share : shares) {
        std::cout << std::left 
                  << std::setw(5)  << share->getID()
                  << std::setw(20) << share->getCompany()
                  << std::fixed << std::setprecision(2) << std::setw(12) << share->getPrice()
                  << std::setprecision(4) << std::setw(12) << share->getVolatility()
                  << std::setprecision(4) << std::setw(12) << share->getDrift()
                  << std::endl;
    }

    std::cout << "================================================================" << std::endl;
    std::cout << std::endl;
}