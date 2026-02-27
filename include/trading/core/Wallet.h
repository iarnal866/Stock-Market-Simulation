#ifndef WALLET_H
#define WALLET_H

#include <vector>
#include "Share.h"
#include "Strategy.h"
/*
Wallet class handles the distribution of funds, tracks stock quantities,
and maintains a historical log of the portfolio's total value. It uses a 
"tensor" style data structure to map values to specific shares within
specific risk clusters.
*/

class Wallet{
    private:
        // Member variables grouped by size (padding optimization)
        // The logic engine that decides how to cluster shares and distribute money.
        Strategy strategyEngine;        // Large object
        // Initial or current cash available in the wallet
        double walletFundPrice;         // 8 bytes

        // Grouping pointers and vectors (usually 24-32 bytes each)
        std::vector<Share*> shares;
        std::vector<double> walletPriceLogbook; // record of the total wallet value
        std::vector<std::vector<Share*>> sharesClusteredPerRiskGroup;
        std::vector<std::vector<double>> stockQuantityPerShare;
        std::vector<std::vector<std::vector<double>>> recordOfStockNumberPerShare;

        // Private helper methods
        void updateSharesClusteredPerRiskGroup();
        // takes a "snapshot" of current stock quantities 
        void updateHistoryRecordOfStockNumberPerShare();

        void registerWalletPriceInWalletogbook(double walletPrice);

    public:
        // Constructor
        Wallet(const double funds, std::vector<int> strategyID,const std::vector<Share*> p_shares);
        // Prevent instantiation of an empty wallet without funds or strategy.
        Wallet() = delete;


        // Getters
        const std::vector<std::vector<Share*>>& getSharesClusteredPerRiskGroup() const;
        const std::vector<std::vector<double>>& getStockQuantityPerShare() const;
        const std::vector<std::vector<std::vector<double>>>& getRecordOfStockNumberPerShare() const;

        std::vector<std::vector<double>> getCurrentMoneyPerShareGivenTheStockQuantity() const;
        double getCurrentTotalWalletPrice() const;
        std::vector<double> getWalletPriceLogBook() const;

        // Public Methods
        std::vector<std::vector<double>> calcualeMoneyToInvestPerShareBasedInTheWalletFund(double fund);
        std::vector<std::vector<double>> calculateStocksQuantityBasedOnTheMoneyToInvestPerShare(const std::vector<std::vector<double>>& moneyInvestedPerShare) const;
        std::vector<std::vector<double>> calculateMoneyPerShareGivenTheStockQuantity(const std::vector<std::vector<double>>& stockQuantityPerShare)const;

        double calculateTotalWalletPrice(const std::vector<std::vector<double>>& moneyInvestedPerShare) const;

        void updateStocksQuantityBasedOnStockQuantityIncrement(const std::vector<std::vector<double>>& stockQuantityIncrement );

        std::vector<std::vector<double>> substractTwoTensors(const std::vector<std::vector<double>>& tensor1, const std::vector<std::vector<double>>& tensor2) const;
        std::vector<std::vector<double>> addTwoTensors(const std::vector<std::vector<double>>& tensor1, const std::vector<std::vector<double>>& tensor2) const;
        
        // Visualization Methods
        void plotTensor(std::vector<std::vector<double>>& tensor) const;
        void plotSharesOfEachRiskGroup() const;


};
#endif