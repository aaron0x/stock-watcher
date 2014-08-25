/****************************************************
 *** Copyright (c) 2014 Aaron aaron1126@gmail.com ***
 ***                                              ***
 *** See the file LICENSE for copying permission. ***
****************************************************/


#include "WatchCondition.h"

#include <cstdlib>
#include <stdexcept>
#include <sstream>

using namespace std;

// TODO: strtof will convet '1.2c' to '1.2', the price string should be checked.
// std::regex is good but have not been implement in most gcc...
WatchCondition::WatchCondition(const string &stockID, const string &lowerPrice,
                               const string &upperPrice) : stockID_(stockID)
{
   lowerPrice_ = strtof(lowerPrice.c_str(), nullptr);
   upperPrice_ = strtof(upperPrice.c_str(), nullptr);

   if (stockID_.empty() || lowerPrice_ < 0 || lowerPrice_  > upperPrice_) {
      throw invalid_argument("WatchCondition create error: " + stockID +
                             " " + lowerPrice + " " + upperPrice);
   }
}

WatchCondition::~WatchCondition()
{
}

const string &WatchCondition::stockID() const
{
   return stockID_;
}

bool WatchCondition::isOutConditon(float price) const
{
   return price > upperPrice_ || price < lowerPrice_;
}

string WatchCondition::outConditionMessage(float price) const
{
   if (!isOutConditon(price)) {
      stringstream ss;
      ss << stockID_ << " price [" << price << "] is NOT out of range "
         << "(" << lowerPrice_ << ", " << upperPrice_ << ")";
      throw runtime_error(ss.str());
   }

   stringstream ss;
   ss << stockID_ << " price [" << price << "] is out of range "
      << "(" << lowerPrice_ << ", " << upperPrice_ << ")";
   return ss.str();
}
