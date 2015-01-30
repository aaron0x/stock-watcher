/****************************************************
 *** Copyright (c) 2014 Aaron aaron1126@gmail.com ***
 ***                                              ***
 *** See the file LICENSE for copying permission. ***
****************************************************/


#include "PriceWatcher.h"
#include "Curl.h"
#include "WatchCondition.h"

#include <stdexcept>
#include <sstream>
#include <fstream>
#include <thread>
#include <chrono>

using namespace std;

PriceWatcher::PriceWatcher()
{
}

PriceWatcher::~PriceWatcher()
{

}

void PriceWatcher::addWatch(const string &stockID, const string &lowerPrice,
                            const string &upperPrice)
{
   if (stockID.empty() || lowerPrice.empty() || upperPrice.empty()) {
      throw invalid_argument("error watch target!");
   }

   watchConditions_.push_back(WatchCondition(stockID, lowerPrice, upperPrice));

   outPriceWatchConditions_.clear();
}

// For unknown reason, YAHOO API failed from time to time.
// Using a new session to retry.
// Yahoo finance API: https://code.google.com/p/yahoo-finance-managed/wiki/YahooFinanceAPIs
// b3:Last Trade Price Only
// http://finance.yahoo.com/d/quotes.csv?s=1565.TWO&f=l1
// http://finance.yahoo.com/d/quotes.csv?s=2727.TW+1565.TWO&f=l1
void PriceWatcher::checkPrice()
{
   const int RETRY_TIME   = 100;
   const int SLEEP_SECOND = 3;

   // Concatenate stock IDs.
   string target;
   for (const auto &t : watchConditions_) {
      if (!target.empty()) {
         target += "+";
      }

      target += t.stockID();
   }

   // Send request.
   for (int i = 0; i < RETRY_TIME; i++) {
      Curl curl;
      string url    = "http://finance.yahoo.com/d/quotes.csv?s=" + target + "&f=l1";
      
      curl.setOption(CURLOPT_FOLLOWLOCATION, 1);
      curl.setOption(CURLOPT_URL,            url.c_str());
      curl.setOption(CURLOPT_WRITEFUNCTION,  handleResponse);
      curl.setOption(CURLOPT_WRITEDATA,      this);
      curl.setOption(CURLOPT_FAILONERROR,    1);

      try {
         curl.perform();
         return;
      } catch (...) {
         if (i == RETRY_TIME) {
            throw;
         }
      }

      this_thread::sleep_for(chrono::seconds(SLEEP_SECOND));
   }
}

bool PriceWatcher::hasOutPriceCondition() const {
   return !outPriceWatchConditions_.empty();
}

void PriceWatcher::appendOutConditionIDs(const string path) const
{
   ofstream notifiedList(path, ios_base::out | ios_base::app);
   if (!notifiedList) {
      throw runtime_error("Can't open notified list:" + path);
   }

   for (auto o : outPriceWatchConditions_) {
      notifiedList << o.first->stockID() << endl;
   }
}

string PriceWatcher::composeOutConditionMessage() const {
   stringstream message;

   for (auto w : outPriceWatchConditions_) {
      message << w.first->outConditionMessage(w.second) << endl;
   }

   return message.str();
}

size_t PriceWatcher::handleResponse(char *buffer, size_t size, size_t num, void *userData)
{
   size_t total = size * num;
   PriceWatcher *self   = static_cast<PriceWatcher*>(userData);
   if (!buffer || total == 0 || !self) {
      return 0;
   }

   try {
      stringstream response(string(buffer, total));
      auto         &watchConditions         = self->watchConditions_;
      auto         &outPriceWatchConditions = self->outPriceWatchConditions_;

      for (auto w = watchConditions.cbegin(); w != watchConditions.cend(); w++) {
         float price = 0;
         response >> price;

         if (w->isOutConditon(price)) {
            outPriceWatchConditions.emplace_back(w, price);
         }
      }
   } catch (...) {
      return 0;
   }

   return total;
}
