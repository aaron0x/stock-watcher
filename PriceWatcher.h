/****************************************************
 *** Copyright (c) 2014 Aaron aaron1126@gmail.com ***
 ***                                              ***
 *** See the file LICENSE for copying permission. ***
****************************************************/


#ifndef PRICEWATCHER_H
#define PRICEWATCHER_H

#include <string>
#include <vector>

class WatchCondition;

class PriceWatcher
{
public:
   PriceWatcher();

   ~PriceWatcher();

   void addWatch(const std::string &stockID, const std::string &lowerPrice,
                 const std::string &upperPrice);

   void checkPrice();

   bool hasOutPriceCondition() const;

   void appendOutConditionIDs(const std::string path) const;

   std::string composeOutConditionMessage() const;

private:
   static size_t handleResponse(char *buffer, size_t size, size_t num, void *userData);

   typedef std::vector<WatchCondition> WatchConditions;
   typedef float CurrnetPrice;

   WatchConditions watchConditions_;

   std::vector<std::pair<WatchConditions::const_iterator, CurrnetPrice>> outPriceWatchConditions_;
};

#endif /* PRICEWATCHER_H */
