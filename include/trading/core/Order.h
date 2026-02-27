#ifndef ORDER_H
#define ORDER_H
#include "Share.h"

struct Order {
    Share* p_share; 
    double quantity;
    int trader_ID;
    int day;
    bool isBuy;
    int shareID;
};

#endif