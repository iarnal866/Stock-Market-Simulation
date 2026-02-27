#ifndef TRADER_H
#define TRADER_H

#include "Wallet.h"
#include "Share.h"
#include "Order.h"

#include <string>
#include <vector>
#include <random>

class Trader{
    private:
        int trader_ID;
        Wallet wallet;        
        std::vector<Order> allOrders; 

        std::vector<std::vector<double>> calculateIncrementInStockQuantityToComplyRiskStrategy();
        std::vector<Order> createOrders(std::vector<std::vector<double>> incrementInStockQuantityToComplyRiskStrategy, int day);
        void updateWallet(std::vector<std::vector<double>> incrementInStockQuantityToComplyRiskStrategy);
        void saveOrders(std::vector<Order> orders );

        
    public:
        Trader(int trader_ID, Wallet p_Wallet);

        std::vector<Order> executeOrders(int day);

        int getTraderID() const;
        Wallet& getWallet();
        void showOrders() const ;
        std::vector<double> getWalletPriceLogBook();

};

#endif