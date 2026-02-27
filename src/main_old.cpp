#include <iostream>
#include <vector>
#include <memory>
#include <string>

#include <ascii/ascii.h>

#include "Market.h"
#include "Trader.h"
#include "DataLoader.h"

#include <limits>   
#include <algorithm>
#include <iterator>


using namespace ascii;
// using <algorithm>
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

void waitForEnter() {
    std::cout << "\nPress ENTER to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}


int main(int argc, char* argv[]) {
    std::cout << "-------- MARKET SIMULATOR --------\n" << std::endl;

    std::string configFile = "config.json"; 
    if (argc > 1) {
        configFile = argv[1];
    }
    
    double dt = 1.0 / 252.0; 
    Market stockMarket(dt);
    std::vector<std::unique_ptr<Trader>> traders;

    try {

        //stockMarket.runSimulation(10000); // for gprof statistics
        int days = DataLoader::loadConfig(configFile, stockMarket, traders);
        stockMarket.runSimulation(days);

        std::cout<<"-------- SIMULATION COMPLETED --------\n"<< std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }

    std::unordered_map<std::string, std::vector<double>> allStockGraphData;
    for (const auto* share : stockMarket.getShares()) {
        allStockGraphData[share->getCompany()] = share->getPriceHistory();
    }

    if (!allStockGraphData.empty()) {
        Asciichart chart(allStockGraphData);
        std::cout << "\nMarket Evolution Graph:\n";
        std::cout << chart.height(15).show_legend(true).Plot() << std::endl;
    }
    // ---------------------------------------------------------
    // INTERACTIVE MENU
    // ---------------------------------------------------------
    
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
        std::cout << "6. Compare Final Wallet Balances & Profit" << std::endl; // Actualizado
        std::cout << "7. Plot ALL Shares" << std::endl;
        std::cout << "8. Plot ALL Traders Wallets History Profit" << std::endl; // Nuevo
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
            case 1:
                stockMarket.plotAllShares();
                break;

            case 2:
                stockMarket.showBuySellHistoryOfShares();
                break;

            case 3: { 
                int tID;
                std::cout << "Enter Trader ID: ";
                std::cin >> tID;
                
                auto it = std::find_if(traders.begin(), traders.end(), 
                    [tID](const std::unique_ptr<Trader>& t) { return t->getTraderID() == tID; });

                if (it != traders.end()) {
                    (*it)->showOrders();
                } else {
                    std::cout << Foreground::From(Color::RED) << "Trader not found." << Decoration::From(Decoration::RESET) << std::endl;
                }
                break;
            }

            case 4: {
                std::cout << "Available Shares IDs:" << std::endl;
                for(auto* s : stockMarket.getShares()) std::cout << s->getID() << ": " << s->getCompany() << " | ";
                std::cout << "\nEnter Share ID to plot: ";
                
                int sID;
                std::cin >> sID;
                
                auto shares = stockMarket.getShares();
                auto it = std::find_if(shares.begin(), shares.end(), 
                    [sID](Share* s) { return s->getID() == sID; });

                if (it != shares.end()) {
                    std::unordered_map<std::string, std::vector<double>> graphData;
                    graphData[(*it)->getCompany()] = (*it)->getPriceHistory();
                    
                    Asciichart chart(graphData);
                    std::cout << "\nPrice Evolution: " << (*it)->getCompany() << "\n";
                    std::cout << chart.height(12).show_legend(true).Plot() << std::endl;
                } else {
                    std::cout << Foreground::From(Color::RED) << "Share ID not found." << Decoration::From(Decoration::RESET) << std::endl;
                }
                break;
            }

            case 5: {
                int tID;
                std::cout << "Enter Trader ID: ";
                std::cin >> tID;

                auto it = std::find_if(traders.begin(), traders.end(), 
                    [tID](const std::unique_ptr<Trader>& t) { return t->getTraderID() == tID; });

                if (it != traders.end()) {
                    std::unordered_map<std::string, std::vector<double>> walletData;
                    walletData["Trader " + std::to_string(tID)] = (*it)->getWalletPriceLogBook();

                    Asciichart chart(walletData);
                    std::cout << "\nWallet Value History for Trader " << tID << "\n";
                    std::cout << chart.height(10).show_legend(true).Plot() << std::endl;
                } else {
                    std::cout << Foreground::From(Color::RED) << "Trader not found." << Decoration::From(Decoration::RESET) << std::endl;
                }
                break;
            }

            case 6: {
                std::cout << "--- Final Wallet Positions & Profit ---" << std::endl;
                std::cout << std::left << std::setw(10) << "Trader" 
                          << std::setw(20) << "Final Balance" 
                          << std::setw(20) << "Profit/Loss" << std::endl;
                std::cout << std::string(50, '-') << std::endl;
                
                std::for_each(traders.begin(), traders.end(), [](auto& t){
                    double finalVal = t->getWallet().getCurrentTotalWalletPrice();

                    const auto& history = t->getWalletPriceLogBook();
                    double initialVal = history.empty() ? 0.0 : history.front(); // Use front() instead
                    double profit = finalVal - initialVal;

                    auto profitColor = (profit >= 0) ? Color::BRIGHT_GREEN : Color::BRIGHT_RED;
                    std::string sign = (profit >= 0) ? "+" : "";

                    std::cout << "ID " << std::setw(7) << t->getTraderID() 
                              << std::fixed << std::setprecision(2) 
                              << finalVal << " $         "
                              << Foreground::From(profitColor) << sign << profit << " $" 
                              << Decoration::From(Decoration::RESET) << std::endl;
                });
                
                break;
            }

            case 7: {
                std::unordered_map<std::string, std::vector<double>> allStockGraphData;
                for (const auto* share : stockMarket.getShares()) {
                    allStockGraphData[share->getCompany()] = share->getPriceHistory();
                }
                if (!allStockGraphData.empty()) {
                    Asciichart chart(allStockGraphData);
                    std::cout << "\nMarket Evolution Graph:\n";
                    std::cout << chart.height(15).show_legend(true).Plot() << std::endl;
                }
                break;
            }

            case 8: {
                std::unordered_map<std::string, std::vector<double>> allWalletsGraphData;
                allWalletsGraphData.reserve(traders.size()); // Reserve space

                for (auto& t : traders) {
                    std::string label = "Trader " + std::to_string(t->getTraderID());
                    
                    std::vector<double> rawHistory = t->getWalletPriceLogBook();
                    
                    if (!rawHistory.empty()) {
                        double initialValue = rawHistory[0];
                        std::vector<double> profitHistory;
                        profitHistory.reserve(rawHistory.size()); 
                        // Use std::transform instead
                                             
                        std::transform(rawHistory.begin(), rawHistory.end(), std::back_inserter(profitHistory), 
                                        [initialValue](double val){
                                            return val - initialValue;
                        });
                        
                        allWalletsGraphData[label] = profitHistory;
                    }
                }

                if (!allWalletsGraphData.empty()) {
                    Asciichart chart(allWalletsGraphData);
                    std::cout << "\nAll Traders Profit Evolution (Relative to Initial Fund):\n";
                    std::cout << chart.height(15).show_legend(true).Plot() << std::endl;
                } else {
                    std::cout << "No wallet data available." << std::endl;
                }
                break;
            }

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