#include <iostream>
#include <vector>
#include <string>

#include "Share.h"

// Constructor
Share::Share(std::string c, double p, double d, double v, double v2, double p_lambda, double p_dailyVolumen, double* timeStep) 
    : 
    company{c},
    price{p}, 
    drift{d}, 
    volatility{v}, 
    variance{v2}, 
    lambda{p_lambda}, 
    dailyVolumen{p_dailyVolumen},
    priceModel{timeStep}
    {
        price_history.push_back(price);
    }

void Share::setPrice (double newPrice)
{
    price = newPrice;
    price_history.push_back(price);
}
void Share::updatePrice(double netVolumen)
{
    priceModel.updateNextPrice(this,netVolumen);
}

// Getter functions
std::string Share::getCompany() const { return company; }

double Share::getPrice() const { return price; }

double Share::getDrift() const { return drift; }

double Share::getVolatility() const { return volatility; }

double Share::getLambda() const { return lambda; };

double Share::getDailyVolumen() const { return dailyVolumen; };

std::vector<double> Share::getPriceHistory() const { return price_history; }

int Share::getID() const { return shareID; };

void Share::setID(int p_shareID){shareID = p_shareID; };


