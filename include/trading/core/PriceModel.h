#ifndef PRICEMODEL_H
#define PRICEMODEL_H

#include <random>

class Share; 

class PriceModel{

private:

    std::random_device rd;
    std::normal_distribution<double> distribution;
    std::mt19937 generationEngine;
    double* timeStep;

    double calculateStochasticDriftDiffusionPart(Share* share);
    double calculateSellBuyImpactPart(Share* share, double & netVolume);

public:

PriceModel(double* dt);
void updateNextPrice(Share* share, double & netVolume);
};

#endif

