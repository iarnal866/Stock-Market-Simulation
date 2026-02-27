#include "Trader.h"
#include "Order.h"
#include "Share.h"
#include "ascii/color.h"


#include <iostream>
#include <iomanip>
#include <numeric>

Trader::Trader(int trader_ID, Wallet p_Wallet)
    : 
    trader_ID{trader_ID}, 
    wallet{std::move(p_Wallet)}
    {
        std::cout << ascii::Foreground::From(ascii::Color::YELLOW) 
          << "Created Trader ID " << trader_ID 
          << ascii::Decoration::From(ascii::Decoration::RESET) 
          << std::endl;     
        wallet.plotSharesOfEachRiskGroup();
    }

std::vector<std::vector<double>> Trader::calculateIncrementInStockQuantityToComplyRiskStrategy()
{
    double actualWalletPrice = wallet.getCurrentTotalWalletPrice();
    std::vector<std::vector<double>> stockQuantityPerShareThatShouldHave = wallet.calculateStocksQuantityBasedOnTheMoneyToInvestPerShare(wallet.calcualeMoneyToInvestPerShareBasedInTheWalletFund(actualWalletPrice));
    
    std::vector<std::vector<double>> incrementInStockQuantityToComplyRiskStrategy = wallet.substractTwoTensors(stockQuantityPerShareThatShouldHave,wallet.getStockQuantityPerShare());

    return incrementInStockQuantityToComplyRiskStrategy;
}

std::vector<Order> Trader::createOrders(std::vector<std::vector<double>> incrementInStockQuantityToComplyRiskStrategy, int day)
{
    std::vector<std::vector<Share*>> sharesClusteredPerRiskGroup = wallet.getSharesClusteredPerRiskGroup();
    std::vector<Order> orders;

    for(size_t groupIndex = 0; groupIndex<sharesClusteredPerRiskGroup.size();groupIndex++)
    {
        for(size_t stockIndexInGroup = 0; stockIndexInGroup<sharesClusteredPerRiskGroup[groupIndex].size(); stockIndexInGroup++)
        {
            if(incrementInStockQuantityToComplyRiskStrategy[groupIndex][stockIndexInGroup] != 0)
            {
                Order order;
                order.day  = day;
                order.quantity = incrementInStockQuantityToComplyRiskStrategy[groupIndex][stockIndexInGroup];
                order.isBuy = (order.quantity > 0.0) ? true : false;
                order.p_share = sharesClusteredPerRiskGroup[groupIndex][stockIndexInGroup];
                order.trader_ID = trader_ID;
                order.shareID = order.p_share->getID();
                orders.push_back(order);
            }
        }
    }
    return orders;
}

void Trader::updateWallet(std::vector<std::vector<double>> incrementInStockQuantityToComplyRiskStrategy)
{
    wallet.updateStocksQuantityBasedOnStockQuantityIncrement(incrementInStockQuantityToComplyRiskStrategy);
}

std::vector<Order> Trader::executeOrders(int day)
{
    std::vector<std::vector<double>> incrementInStockQuantityToComplyRiskStrategy = calculateIncrementInStockQuantityToComplyRiskStrategy();
    std::vector<Order> orders = createOrders(incrementInStockQuantityToComplyRiskStrategy, day);
    updateWallet(incrementInStockQuantityToComplyRiskStrategy);
    saveOrders(orders);

    return orders;
}

void Trader::saveOrders(std::vector<Order> orders )
{
    allOrders.insert(allOrders.end(),std::make_move_iterator(orders.begin()),std::make_move_iterator(orders.end()));
}

int Trader::getTraderID() const { 
    return trader_ID; 
}
 Wallet& Trader::getWallet() { 
    return wallet; 
}

std::vector<double> Trader::getWalletPriceLogBook() { 
    return wallet.getWalletPriceLogBook(); 
}



void Trader::showOrders() const 
{
    std::cout << std::left 
              << std::setw(8)  << "Day"
              << std::setw(12) << "Trader ID"
              << std::setw(8)  << "Type"
              << std::setw(10) << "Share ID"
              << std::setw(12) << "Quantity"
              << std::endl;

    std::cout << std::string(50, '-') << std::endl;

    for (const auto& order : allOrders)
    {
        std::cout << std::left 
                  << std::setw(8)  << order.day
                  << std::setw(12) << order.trader_ID
                  << std::setw(8)  << (order.isBuy ? "BUY" : "SELL")
                  << std::setw(10) << order.shareID
                  << std::setw(12) << order.quantity
                  << std::endl;
    }
}

