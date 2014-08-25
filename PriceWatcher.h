/****************************************************
 *** Copyright (c) 2014 Aaron aaron1126@gmail.com ***
 ***                                              ***
 *** See the file LICENSE for copying permission. ***
****************************************************/


#ifndef PRICEWATCHER_H
#define PRICEWATCHER_H

#include <string>
#include <memory>
#include <vector>

class SMTP;
class WatchCondition;

class PriceWatcher
{
public:
   PriceWatcher();

   ~PriceWatcher();

   void addWatch(const std::string &stockID, const std::string &lowerPrice,
                 const std::string &upperPrice);

   void checkPrice(SMTP *smtp);

   void appendOutConditionIDs(const std::string path);

private:
   typedef std::unique_ptr<WatchCondition> UPtrWatchCondition ;

   typedef struct Data {
      PriceWatcher *self;
      SMTP *smtp;
   } Data;

   static size_t handleResponse(char *buffer, size_t size, size_t num, void *userData);

   std::vector<UPtrWatchCondition> watchConditions_;

   std::vector<std::string> outConditionIDs_;
};

#endif /* PRICEWATCHER_H */
