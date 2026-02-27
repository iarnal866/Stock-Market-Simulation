#ifndef MARKET_H
#define MARKET_H

#include <memory>
#include <vector>

/*
Forward declarations to speed up compilation: notify the compiler that these classes/structs
exist without needing to #include their full headers yet. This reduces compilation time
and prevents circular dependency issues.
*/ 

class Share;
class Trader;
struct Order;

/*
Market Class: acts as Exchange. It maintains the list of available shares,
manages the list of participating traders and executes the simulation 
logic that determines how prices move over time.
*/

class Market{
    private:
        // Goruping vectors together for better spatial locality
        std::vector<std::unique_ptr<Share>> shares;
        // using unique_ptr ensures the Market owns the memory of these shares and they are
        // automatically deleted when the Market is destroyed.

        std::vector<Trader*> registeredTraders;
        // Pointer to Traders because the Market does not won the traders.

        std::vector<std::vector<double>> sharesBuySellHistory;

        // Primitive types last to avoid padding gaps
        double dt;  // 8 bytes

        void updateNetVolumenPerShareBasedOnTheOrders(std::vector<double>& netVolumenPerShare, const std::vector<Order>& totalOrdersOfThisTimeStep) const;
        
        // Calculate price movements and updates states for the next day:
        void calculateNextStep(int day);
        // Ensure each share's internal ID matches its index in the vector
        void setSharesIDbasedInThePositions();


    public:
        // Constructor
        Market(double timeStep);
        void runSimulation(int steps);

        // Optimized: pass by reference
        void registerShares(std::vector<std::unique_ptr<Share>> p_shares);
        void registerTraders(const std::vector<Trader*>& p_registeredTraders);
        void registerTrader(Trader* trader);

        // Return by const reference to avoid copying the whole history vector
        const std::vector<double>& getPriceHistoryOfAGivenShare(int shareID) const;
        std::vector<Share*> getShares() const;
        double* getTimeStep();

        // Visualization
        void showBuySellHistoryOfShares() const;
        void plotAllShares() const;

};

#endif