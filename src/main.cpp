#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <iomanip>

#include <ascii/ascii.h>

#include "Market.h"
#include "Trader.h"
#include "DataLoader.h"

#include <limits>   
#include <algorithm>
#include <iterator>


using namespace ascii;

// Overload the operator '<<' to print vectors in a readable format (e.g., [x,y,z] )
template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v){
    os << "[";
    if (!v.empty()){
        std::copy(v.begin(), v.end() - 1, 
                  std::ostream_iterator<T>(os, ", "));
        os << v.back();
    }
    os << "]";
    return os;
}

// Helper to pause the terminal until the user presses Enter.
void waitForEnter() {
    std::cout << "\nPress ENTER to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

// Exports the price history of all shares in the market to a CSV file.
// Format: step, Company 1, Company 2, ...
void exportMarketToCSV(const Market& market) {
    std::ofstream file("market_history.csv");
    if (!file.is_open()) {
        std::cerr << "Error: Could not open market_history.csv for writing." << std::endl;
        return;
    }

    auto shares = market.getShares();
    if (shares.empty()) return;

    file << "Step";
    for (const auto* s : shares) {
        file << "," << s->getCompany();
    }
    file << "\n";

    size_t steps = shares[0]->getPriceHistory().size();

    for (size_t i = 0; i < steps; ++i) {
        file << i;
        for (const auto* s : shares) {
            file << "," << s->getPriceHistory()[i];
        }
        file << "\n";
    }

    file.close();
    std::cout << "Market history successfully exported to 'market_history.csv'" << std::endl;
}

// Exports the profit/loss history of all traders to a CSV file.
// The values are relative to the initial fund (FinalValue - InitialValue).
void exportTradersToCSV(const std::vector<std::unique_ptr<Trader>>& traders) {
    std::ofstream file("traders_profit_history.csv");
    if (!file.is_open()) {
        std::cerr << "Error: Could not open traders_profit_history.csv for writing." << std::endl;
        return;
    }

    file << "Step";
    for (const auto& t : traders) {
        file << ",Trader_" << t->getTraderID();
    }
    file << "\n";

    if (traders.empty()) return;
    size_t steps = traders[0]->getWalletPriceLogBook().size();

    for (size_t i = 0; i < steps; ++i) {
        file << i;
        for (const auto& t : traders) {
            const auto& history = t->getWalletPriceLogBook();
            double initialVal = history.empty() ? 0.0 : history.front();
            file << "," << (history[i] - initialVal); 
        }
        file << "\n";
    }

    file.close();
    std::cout << "Traders profit history successfully exported to 'traders_profit_history.csv'" << std::endl;
}


int main(int argc, char* argv[]) {
    std::cout << "-------- MARKET SIMULATOR --------\n" << std::endl;

    std::string configFile = "config.json"; 
    if (argc > 1) {
        configFile = argv[1];
    }
    
    double dt = 1.0 / 252.0; // Time step: 1 day in a business year (252 trading days)
    Market stockMarket(dt);
    std::vector<std::unique_ptr<Trader>> traders;

    // 1. DATA LOADING & SIMULATION EXECUTION
    try {
        // Loads simulation parameters, stocks, and traders from JSON
        int days = DataLoader::loadConfig(configFile, stockMarket, traders);
        stockMarket.runSimulation(days);
        std::cout<<"-------- SIMULATION COMPLETED --------\n"<< std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }

    // 2. ANALYSIS MENU (Interactive)
    int choice = -1;
    while (choice != 0) {
        std::cout << "\n" << std::string(40, '=') << std::endl;
        std::cout << Foreground::From(Color::BRIGHT_CYAN) << " ANALYSIS MENU " << Decoration::From(Decoration::RESET) << std::endl;
        std::cout << std::string(40, '=') << std::endl;
        std::cout << "1. Show All Market Shares (Summary Table)" << std::endl;
        std::cout << "2. Show Global Buy/Sell Orders" << std::endl;
        std::cout << "3. Show Trader Specific Orders" << std::endl;
        std::cout << "4. Plot Specific Share Price History" << std::endl;
        std::cout << "5. Plot Trader Wallet History (Single)" << std::endl;
        std::cout << "6. Compare Final Wallet Balances & Profit" << std::endl;
        std::cout << "7. Plot ALL Shares" << std::endl;
        std::cout << "8. Plot ALL Traders Wallets History Profit" << std::endl;
        std::cout << "9. Export Market Prices to CSV" << std::endl;
        std::cout << "10. Export Traders Profit to CSV" << std::endl;
        std::cout << "0. Exit" << std::endl;
        std::cout << std::string(40, '-') << std::endl;
        std::cout << "Select an option: ";
        
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        std::cout << std::endl;

        switch (choice) {
            case 1: // Prints a summary table of shares (Company, final price, volatility, etc.)
                stockMarket.plotAllShares();
                break;

            case 2: // Displays the buy/sell history log for each share
                stockMarket.showBuySellHistoryOfShares();
                break;

            case 3: { // Displays the list of orders executed by a specific trader
                int tID;
                std::cout << "Enter Trader ID: ";
                std::cin >> tID;
                auto it = std::find_if(traders.begin(), traders.end(), 
                    [tID](const std::unique_ptr<Trader>& t) { return t->getTraderID() == tID; });
                if (it != traders.end()) (*it)->showOrders();
                else std::cout << Foreground::From(Color::RED) << "Trader not found." << Decoration::From(Decoration::RESET) << std::endl;
                break;
            }

            case 4: { // Visualizes a line chart of a share's price history in the terminal
                int sID;
                std::cout << "\nEnter Share ID to plot: ";
                std::cin >> sID;
                auto shares = stockMarket.getShares();
                auto it = std::find_if(shares.begin(), shares.end(), [sID](Share* s) { return s->getID() == sID; });
                if (it != shares.end()) {
                    std::unordered_map<std::string, std::vector<double>> graphData;
                    graphData[(*it)->getCompany()] = (*it)->getPriceHistory();
                    Asciichart chart(graphData);
                    std::cout << chart.height(12).show_legend(true).Plot() << std::endl;
                }
                break;
            }

            case 5: { // Visualizes the historical value of a trader's portfolio
                int tID;
                std::cout << "Enter Trader ID: ";
                std::cin >> tID;
                auto it = std::find_if(traders.begin(), traders.end(), [tID](const std::unique_ptr<Trader>& t) { return t->getTraderID() == tID; });
                if (it != traders.end()) {
                    std::unordered_map<std::string, std::vector<double>> walletData;
                    walletData["Trader " + std::to_string(tID)] = (*it)->getWalletPriceLogBook();
                    Asciichart chart(walletData);
                    std::cout << chart.height(10).show_legend(true).Plot() << std::endl;
                }
                break;
            }

            case 6: { // Compares final portfolio values and total profit for all traders
                std::cout << std::left << std::setw(10) << "Trader" << std::setw(20) << "Final Balance" << std::setw(20) << "Profit/Loss" << std::endl;
                for (auto& t : traders) {
                    double finalVal = t->getWallet().getCurrentTotalWalletPrice();
                    double initialVal = t->getWalletPriceLogBook().empty() ? 0.0 : t->getWalletPriceLogBook().front();
                    double profit = finalVal - initialVal;
                    auto color = (profit >= 0) ? Color::BRIGHT_GREEN : Color::BRIGHT_RED;
                    std::cout << "ID " << std::setw(7) << t->getTraderID() << std::fixed << std::setprecision(2) << finalVal << " $         "
                              << Foreground::From(color) << (profit >= 0 ? "+" : "") << profit << " $" << Decoration::From(Decoration::RESET) << std::endl;
                }
                break;
            }

            case 7: {
                std::unordered_map<std::string, std::vector<double>> allStockGraphData;
                for (const auto* share : stockMarket.getShares()) allStockGraphData[share->getCompany()] = share->getPriceHistory();
                if (!allStockGraphData.empty()) std::cout << Asciichart(allStockGraphData).height(15).show_legend(true).Plot() << std::endl;
                break;
            }

            case 8: {
                std::unordered_map<std::string, std::vector<double>> allWalletsGraphData;
                for (auto& t : traders) {
                    std::vector<double> rawHistory = t->getWalletPriceLogBook();
                    if (!rawHistory.empty()) {
                        double initialValue = rawHistory[0];
                        std::vector<double> profitHistory;
                        std::transform(rawHistory.begin(), rawHistory.end(), std::back_inserter(profitHistory), [initialValue](double val){ return val - initialValue; });
                        allWalletsGraphData["Trader " + std::to_string(t->getTraderID())] = profitHistory;
                    }
                }
                if (!allWalletsGraphData.empty()) std::cout << Asciichart(allWalletsGraphData).height(15).show_legend(true).Plot() << std::endl;
                break;
            }

            case 9:
                exportMarketToCSV(stockMarket);
                break;

            case 10:
                exportTradersToCSV(traders);
                break;

            case 0:
                std::cout << "Exiting..." << std::endl;
                break;

            default:
                std::cout << "Invalid option." << std::endl;
        }
        
        if (choice != 0) waitForEnter();
    }
    return 0;
}