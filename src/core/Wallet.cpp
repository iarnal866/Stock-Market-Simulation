#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

#include "Wallet.h"
#include "Strategy.h"
#include "Share.h"


// Constructor
Wallet::Wallet(double funds, std::vector<int> strategyID, const std::vector<Share*> p_shares) 
    :       
    strategyEngine{strategyID}, 
    shares{p_shares},               // Raw list of pointers to shares
    walletFundPrice{funds},         // Set initial cash balance
    walletPriceLogbook{},           // Initialize empty history of wallet values
    // cluster shares into groups of different risks using the strategy engine
    sharesClusteredPerRiskGroup{strategyEngine.associateShareToEachRiskGroup(shares)},
    // Initial calculation: Fund -> Money per Share -> Number of Stocks to purchase
    stockQuantityPerShare{calculateStocksQuantityBasedOnTheMoneyToInvestPerShare(calcualeMoneyToInvestPerShareBasedInTheWalletFund(walletFundPrice))}
    {
        //DEBUGING
        /**
        plotSharesOfEachRiskGroup();
        plotTensor(calcualeMoneyToInvestPerShareBasedInTheWalletFund(walletFundPrice));
        plotTensor(stockQuantityPerShare);
        plotTensor(calculateMoneyPerShareGivenTheStockQuantity(stockQuantityPerShare));
        std::cout<<getCurrentTotalWalletPrice()<<std::endl;
        */
    }

// Re-run the clustering logic to group shares according to the strategy's risk groups.
void Wallet::updateSharesClusteredPerRiskGroup()
{
    sharesClusteredPerRiskGroup = strategyEngine.associateShareToEachRiskGroup(shares);
}

// Record snapshot of the total wallet value into the logbook for history tracking.
void Wallet::registerWalletPriceInWalletogbook(double walletPrice)
{
    walletPriceLogbook.push_back(walletPrice);
}
/*
Calculate how much money should be allocated to every single share,
based on the total fund provided and the distribution logic of the StrategyEngine.
*/
std::vector<std::vector<double>> Wallet::calcualeMoneyToInvestPerShareBasedInTheWalletFund(double fund)
{
    std::vector<std::vector<double>> moneyToInvestPerShare=strategyEngine.distributeFundInEachShare(sharesClusteredPerRiskGroup,fund);
    return  moneyToInvestPerShare;
}

/*
Update the quantity of stocks held by adding an increment.
Automatically logs the new total value of the wallet after the update.
*/
void Wallet::updateStocksQuantityBasedOnStockQuantityIncrement(const std::vector<std::vector<double>>& stockQuantityIncrement ) 
{
    for(size_t groupIndex = 0; groupIndex<stockQuantityPerShare.size();groupIndex++)
    {
        auto& group = stockQuantityPerShare[groupIndex];
        const auto& incrementGroup = stockQuantityIncrement[groupIndex];
        
        // The compiler can vectorize this std::transform better than a normal loop
        std::transform(group.begin(), group.end(), incrementGroup.begin(), group.begin(), std::plus<double>());
    }

    registerWalletPriceInWalletogbook(getCurrentTotalWalletPrice());
}

// Tensor substraction; used for calculating differences in allocations or quantities.
std::vector<std::vector<double>> Wallet::substractTwoTensors(const std::vector<std::vector<double>>& tensor1, const std::vector<std::vector<double>>& tensor2) const
{
    if (tensor1.size() != tensor2.size()) {throw std::runtime_error("Dimension mismatch in substractTwoTensors");}

    std::vector<std::vector<double>> resultedTensor(tensor1.size());

   for (size_t i = 0; i < tensor1.size(); ++i) {
        resultedTensor[i].resize(tensor1[i].size());
        std::transform(tensor1[i].begin(), tensor1[i].end(), tensor2[i].begin(), resultedTensor[i].begin(), std::minus<double>());
    }

    return resultedTensor;
}

std::vector<std::vector<double>> Wallet::addTwoTensors(const std::vector<std::vector<double>>& tensor1, const std::vector<std::vector<double>>& tensor2) const
{
    if (tensor1.size() != tensor2.size()) {throw std::runtime_error("Dimension mismatch in addTwoTensors");}

    std::vector<std::vector<double>> resultedTensor(tensor1.size());

    for (size_t i = 0; i < tensor1.size(); ++i) {
        resultedTensor[i].resize(tensor1[i].size());
        // Use std::transform for potential vectorization
        std::transform(tensor1[i].begin(), tensor1[i].end(), tensor2[i].begin(), resultedTensor[i].begin(), std::plus<double>());
    }

    return resultedTensor;
}

// Convert monetary allocation into actual stock quantities.
// Quantity = allocated_money / current_market_price
std::vector<std::vector<double>> Wallet::calculateStocksQuantityBasedOnTheMoneyToInvestPerShare(const std::vector<std::vector<double>>& moneyInvestedPerShare) const
{
    std::vector<std::vector<double>> calculatedStockNumberPerShare(sharesClusteredPerRiskGroup.size());
    calculatedStockNumberPerShare.reserve(sharesClusteredPerRiskGroup.size());

    for(size_t groupIndex = 0; groupIndex<sharesClusteredPerRiskGroup.size();groupIndex++)
    {
        calculatedStockNumberPerShare[groupIndex].reserve(sharesClusteredPerRiskGroup[groupIndex].size());

        for(size_t stockIndexInGroup = 0; stockIndexInGroup<sharesClusteredPerRiskGroup[groupIndex].size(); stockIndexInGroup++)
        {
            Share* share = sharesClusteredPerRiskGroup[groupIndex][stockIndexInGroup];
            double shareMarketPrice = share->getPrice();
            // Divide budget by price to get volume
            calculatedStockNumberPerShare[groupIndex].push_back(moneyInvestedPerShare[groupIndex][stockIndexInGroup]/shareMarketPrice);
        }
    }
    
    return calculatedStockNumberPerShare;
}

std::vector<std::vector<double>> Wallet::getCurrentMoneyPerShareGivenTheStockQuantity() const
{
    std::vector<std::vector<double>> moneyInvestedPerShare = calculateMoneyPerShareGivenTheStockQuantity(stockQuantityPerShare) ;
    return  moneyInvestedPerShare;
}

// Convert stock quantities back into monetary value based on current market price
// Value = Quantity * current_market_price
std::vector<std::vector<double>> Wallet::calculateMoneyPerShareGivenTheStockQuantity(const std::vector<std::vector<double>>& stockQuantityPerShare) const
{
    std::vector<std::vector<double>> moneyInvestedPerShare(sharesClusteredPerRiskGroup.size());
    moneyInvestedPerShare.reserve(sharesClusteredPerRiskGroup.size());
    
    for(size_t groupIndex = 0; groupIndex < sharesClusteredPerRiskGroup.size(); groupIndex++)
    {
        const auto& shareGroup = sharesClusteredPerRiskGroup[groupIndex];
        const auto& quantityGroup = stockQuantityPerShare[groupIndex];
        moneyInvestedPerShare[groupIndex].resize(shareGroup.size());

        for(size_t i = 0; i < shareGroup.size(); ++i)
        {
            // Direct access to price to avoid overhead of heavy calls
            moneyInvestedPerShare[groupIndex][i] = quantityGroup[i] * shareGroup[i]->getPrice();
        }
    }
    return moneyInvestedPerShare;
}

double Wallet::calculateTotalWalletPrice(const std::vector<std::vector<double>>& moneyInvestedPerShare) const
{   
    double totalMoney {0};
    // Flattened loop for better cache locality and performance
    for (const auto& group : moneyInvestedPerShare) {
        totalMoney = std::accumulate(group.begin(), group.end(), totalMoney);
    }
    return totalMoney;
}

double Wallet::getCurrentTotalWalletPrice() const
{
    double currentTotalWalletPrice = calculateTotalWalletPrice(getCurrentMoneyPerShareGivenTheStockQuantity());
    return currentTotalWalletPrice;
}
std::vector<double> Wallet::getWalletPriceLogBook() const{return walletPriceLogbook;}



void Wallet::updateHistoryRecordOfStockNumberPerShare()
{
    recordOfStockNumberPerShare.push_back(stockQuantityPerShare);
}

// Getters
const std::vector<std::vector<Share*>>& Wallet::getSharesClusteredPerRiskGroup() const {
    return sharesClusteredPerRiskGroup;
}

const std::vector<std::vector<double>>& Wallet::getStockQuantityPerShare() const {
    return stockQuantityPerShare;
}

const std::vector<std::vector<std::vector<double>>>& Wallet::getRecordOfStockNumberPerShare() const {
    return recordOfStockNumberPerShare;
}

// Visualization: prints a 2D grid (tensor) of values to the terminal.
void Wallet::plotTensor(std::vector<std::vector<double>>& tensor) const
{   std::cout << std::endl<<std::endl<<std::string(30, '-') << std::endl;
    for(size_t groupIndex = 0; groupIndex < sharesClusteredPerRiskGroup.size(); groupIndex++)
    {
        for(size_t stockIndexInGroup = 0; stockIndexInGroup<sharesClusteredPerRiskGroup[groupIndex].size();stockIndexInGroup++)
        {
            std::cout<<tensor[groupIndex][stockIndexInGroup]<<" | ";;
        }
        std::cout<<std::endl;
        std::cout << std::string(30, '-') << std::endl;    
    }
}

// Visualization: prints the names of companies organized by their risk group.
void Wallet::plotSharesOfEachRiskGroup() const
{   
    std::cout << "Wallet shares:" << std::endl << std::endl;
    std::cout << std::string(50, '-') << std::endl; 

    for(size_t groupIndex = 0; groupIndex < sharesClusteredPerRiskGroup.size(); groupIndex++)
    {
        std::cout << "Group " << (groupIndex + 1) << " | ";

        for(size_t stockIndexInGroup = 0; stockIndexInGroup < sharesClusteredPerRiskGroup[groupIndex].size(); stockIndexInGroup++)
        {
            std::string company = sharesClusteredPerRiskGroup[groupIndex][stockIndexInGroup]->getCompany();
            std::cout << company << " | ";
        }
        
        std::cout << std::endl;
        std::cout << std::string(50, '-') << std::endl;
    }
    std::cout << std::endl << std::endl << std::endl;    
}


