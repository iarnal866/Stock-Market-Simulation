#include "PriceModel.h"
#include "Share.h"

#include <cmath>
#include <random>


// Constructor

PriceModel::PriceModel(double* dt)
    :
    rd{},
    distribution{0.0, 1.0},
    generationEngine{rd()},
    timeStep{dt}{}

//Calculate next price

double PriceModel::calculateStochasticDriftDiffusionPart(Share* share)
{
    double randomNumber = distribution(generationEngine);
    double stochasticDriftDiffusionPart = (share->getDrift() - std::pow(share->getVolatility(), 2) / 2.0) * (*timeStep) + share->getVolatility() * std::sqrt(*timeStep) * randomNumber;
    return stochasticDriftDiffusionPart;
}

double PriceModel::calculateSellBuyImpactPart(Share* share, double & netVolumen)
{
    double sellBuyImpactPart = (share->getLambda())*(netVolumen/(share->getDailyVolumen()));   
    return  sellBuyImpactPart;
}

void PriceModel::updateNextPrice(Share* share, double & netVolumen)
{
    double stochasticDriftDiffusionPart = calculateStochasticDriftDiffusionPart(share);
    double sellBuyImpactPart = calculateSellBuyImpactPart(share, netVolumen);

    double nextPrice = share->getPrice()*std::exp(stochasticDriftDiffusionPart+sellBuyImpactPart);
    share->setPrice(nextPrice);
}


