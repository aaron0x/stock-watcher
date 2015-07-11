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

void PriceWatcher::checkPrice()
{
   const int RETRY_TIME   = 100;
   const int SLEEP_SECOND = 3;

   // Send request.
   for (int i = 0; i < RETRY_TIME; i++) {
      Curl curl;
      string url = composeUrl(&curl);
      
      setCurlOption(&curl, url);

      try {
         curl.perform();
         return;
      } catch (exception &e) {
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
      string response(buffer, total);
      auto   &watchConditions         = self->watchConditions_;
      auto   &outPriceWatchConditions = self->outPriceWatchConditions_;
      int    curPos                   = 0;

      for (auto w = watchConditions.cbegin(); w != watchConditions.cend(); w++) {
         const string pattern("\"LastTradePriceOnly\":\"");
         size_t start = response.find(pattern, curPos) + pattern.size();
         size_t end   = response.find("\"", start) - 1;
         stringstream ss(response.substr(start, end - start + 1)); 
         float price = 0;
         ss >> price;

         if (w->isOutConditon(price)) {
            outPriceWatchConditions.emplace_back(w, price);
         }

         curPos = end + 1;
      }
   } catch (...) {
      return 0;
   }

   return total;
}

// Use YQL:https://developer.yahoo.com/yql
// example:
// https://query.yahooapis.com/v1/public/yql?q=select LastTradePriceOnly from yahoo.finance.quote where symbol in ("2727.TW")&format=json&diagnostics=true&env=store://datatables.org/alltableswithkeys&callback=
// https://query.yahooapis.com/v1/public/yql?q=select%20LastTradePriceOnly%20from%20yahoo.finance.quote%20where%20symbol%20in%20(%222727.TW%22)&format=json&diagnostics=true&env=store%3A%2F%2Fdatatables.org%2Falltableswithkeys&callback=
string PriceWatcher::composeUrl(Curl *curl)
{
    // Concatenate stock IDs.
   string target;
   for (const auto &t : watchConditions_) {
      if (!target.empty()) {
         target += ",";
      }

      target = target + "\"" + t.stockID() + "\"";
   }
   
   string escapedSelect = "select%20LastTradePriceOnly%20from%20yahoo.finance.quote%20where%20symbol%20in%20";
   string escapedTarget = curl->escape(target);
   return string("https://query.yahooapis.com/v1/public/yql?q=") + string(escapedSelect) + string("(") + string(escapedTarget) + string(")") + string("&format=json&env=store%3A%2F%2Fdatatables.org%2Falltableswithkeys&callback=");
}

void PriceWatcher::setCurlOption(Curl *curl, const string &url)
{
   curl->setOption(CURLOPT_FOLLOWLOCATION, 1);
   curl->setOption(CURLOPT_URL,            url.c_str());
   curl->setOption(CURLOPT_WRITEFUNCTION,  handleResponse);
   curl->setOption(CURLOPT_WRITEDATA,      this);
   curl->setOption(CURLOPT_FAILONERROR,    1);
}
