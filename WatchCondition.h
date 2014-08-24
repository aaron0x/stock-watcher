#ifndef WATCHCONDITOIN_H
#define WATCHCONDITOIN_H

#include <string>

class WatchCondition
{
public:
   WatchCondition(const std::string &stockID, const std::string &lowerPrice,
                  const std::string &upperPrice);

   ~WatchCondition();

   const std::string &stockID() const;

   bool isOutConditon(float price) const;

   std::string outConditionMessage(float price) const;

private:
   std::string stockID_;

   float lowerPrice_;

   float upperPrice_;
};

#endif /* WATCHCONDITOIN_H */