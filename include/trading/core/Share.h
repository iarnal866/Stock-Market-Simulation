#ifndef SHARE_H
#define SHARE_H

#include "PriceModel.h" 

#include <iostream>
#include <vector>
#include <string>


class Share{
    private:
        std::string company;
        double price;
        double drift;
        double volatility;
		double variance;
		double lambda;
		double dailyVolumen;
		int shareID;

        std::vector<double> price_history; // vector that stores the price at every time step
		PriceModel priceModel;

		friend class PriceModel;

		// Set functions for the priceModel class
		void setPrice(double newPrice);

    public:
	// Constructor
	
	Share(std::string c, double p, double d, double v, double v2, double p_lambda, double p_dailyVolumen, double* p_timeStep);      
	// Methods
	void updatePrice(double netVolumen);




	// Get functions
	std::string getCompany() const;
	double getPrice() const;
	double getDrift() const;
	double getVolatility() const;
	double getLambda() const;
	double getDailyVolumen() const;
	int getID() const;
	std::vector<double> getPriceHistory() const;

	void setID(int p_shareID);

};
#endif 
