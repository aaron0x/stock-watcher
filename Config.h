/****************************************************
 *** Copyright (c) 2014 Aaron aaron1126@gmail.com ***
 ***                                              ***
 *** See the file LICENSE for copying permission. ***
****************************************************/


#ifndef CONFIG_H
#define CONFIG_H

#include "PriceWatcher.h"

#include <set>
#include <map>
#include <memory>
#include <fstream>

class SMTP;

class Config
{
public:
   typedef std::multimap<std::string, std::string> Map;

   Config(const char *path);

   ~Config();

   void checkPrice();

private:
   static const int MAX_NAME_VALUE_LENGTH = 128;

   void load();

   void parseConfig(std::ifstream *configFile, Map *items);

   void parseLine(const char *line, Map *items);

   void setupObjects(const Map &items);

   void setNotifiedStock(const Map &items);

   void openErrorLog(const Map &items);

   void addWatchedStocks(const Map &items);

   void createSMTP(const Map &items);

   void setMailToNotify(const Map &items);

   const char *path_;

   std::ofstream errorLog_;

   std::string notifiedList_;

   std::set<std::string> notifiedStock_;

   PriceWatcher watcher_;

   std::unique_ptr<SMTP> smtp_;

   std::string mailToNotify_;
};

#endif /* CONFIG_H */
