/****************************************************
 *** Copyright (c) 2014 Aaron aaron1126@gmail.com ***
 ***                                              ***
 *** See the file LICENSE for copying permission. ***
****************************************************/


#include "Config.h"
#include "SMTP.h"

#include <unistd.h>
#include <sstream>
#include <cstring>
#include <stdexcept>

using namespace std;

Config::Config(const char *path) : path_(path)
{
   if (!path || path[0] == '\0') {
      throw invalid_argument("Configuration file path is wrong!");
   }

   load();
}

Config::~Config()
{

}

void Config::load()
{
   ifstream configFile(path_);
   if (!configFile) {
      throw runtime_error(string("Can't open configuration file:") + path_);
   }

   Map items;
   parseConfig(&configFile, &items);
   setupObjects(items);
}

void Config::checkPrice()
{
   try {
      watcher_.checkPrice();

      if (watcher_.hasOutPriceCondition()) {
         string message = watcher_.composeOutConditionMessage();

         smtp_->addHeader("Subject", "Stock Watcher");
         smtp_->appendBody(message);
         smtp_->send(mailToNotify_);

         // Add notified stock into notified list.
         if (!notifiedList_.empty()) {
            watcher_.appendOutConditionIDs(notifiedList_);
         }
      }
   } catch (const exception &e) {
      if (errorLog_) {
         errorLog_ << e.what() << endl;
      }

      smtp_->appendBodyWithNewLine(string("checkPrice failed, reason: ") + e.what());
      smtp_->send(mailToNotify_);

      throw;
   }
}

void Config::parseConfig(ifstream *configFile, Map *items)
{
   char oneLine[MAX_NAME_VALUE_LENGTH + 1] = "";

   while (configFile->getline(oneLine, MAX_NAME_VALUE_LENGTH + 1)) {
      parseLine(oneLine, items);
   }
}

void Config::parseLine(const char *line, Map *items)
{
   // Skip empty line.
   if (strlen(line) == 0) {
      return;
   }

   // Find first Non-whitespace character.
   const char *nonSpace = line;
   while (isspace(*nonSpace)) {
      nonSpace++;
   }

   // Skip comment line.
   if (*nonSpace == '#') {
      return;
   }

   // Get Key-Value pare.
   char key[MAX_NAME_VALUE_LENGTH + 1]   = "";
   char value[MAX_NAME_VALUE_LENGTH + 1] = "";
   if (sscanf(nonSpace, "%[^=]=%[^\n]", key, value) != 2) {
      throw invalid_argument(string("Format of configuration file error: ") + line);
   }

   items->emplace(key, value);
}

void Config::setupObjects(const Map &items)
{
   setNotifiedStock(items);
   openErrorLog(items);
   addWatchedStocks(items);
   createSMTP(items);
   setMailToNotify(items);
}

// Read these stocks have been notified from file.
void Config::setNotifiedStock(const Map &items)
{
   const auto it = items.find("notified_list");
   if (it == items.end()) {
      return;
   }

   notifiedList_ = it->second;
   if (access(notifiedList_.c_str(), R_OK) == -1) {
      return;
   }

   ifstream notified(notifiedList_);
   if (!notified) {
      throw runtime_error("Can't open notified list:" + it->second);
   }

   while (!notified.eof()) {
      string stock;
      notified >> stock;
      notifiedStock_.insert(stock);
   }
}

void Config::openErrorLog(const Map &items)
{
   const auto it = items.find("error_log");
   if (it == items.end()) {
      return;
   }

   errorLog_.open(it->second, ofstream::out | ofstream::app);
   if (!errorLog_) {
      throw runtime_error("Can't open error log:" + it->second);
   }
}

void Config::addWatchedStocks(const Map &items)
{
   const auto ret = items.equal_range("stock");
   for (auto it = ret.first; it != ret.second; it++) {
      string stockID, lower, upper;
      stringstream input(it->second);

      input >> stockID;
      input >> lower;
      input >> upper;

      // Skip notified stock.
      if (notifiedStock_.find(stockID) != notifiedStock_.end()) {
         continue;
      }

      watcher_.addWatch(stockID, lower, upper);
   }
}

void Config::createSMTP(const Map &items)
{
   const auto smtpIt = items.find("smtp");
   if (smtpIt == items.end()) {
      throw invalid_argument("Missing SMTP setting");
   }
   const auto   userIt   = items.find("user");
   const auto   passwdIt = items.find("password");

   const string &smtp    = smtpIt->second;
   const string &user    = (userIt   == items.end()) ? string("") : userIt->second;
   const string &passwd  = (passwdIt == items.end()) ? string("") : passwdIt->second;

   smtp_.reset(new SMTP(smtp, user, passwd));
}

void Config::setMailToNotify(const Map &items)
{
   const auto addressIt = items.find("notify_mail");
   if (addressIt == items.end()) {
      throw invalid_argument("Missing Notify mail");
   }

   mailToNotify_ = addressIt->second;
}
