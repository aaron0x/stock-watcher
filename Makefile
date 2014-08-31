CXXFLAGS  ?= -O2 -Wall -Werror -std=c++11
LOADLIBES := -lcurl

stock_watcher: stock_watcher.cpp Config.o PriceWatcher.o WatchCondition.o SMTP.o Curl.o

Config.o: Config.h PriceWatcher.h SMTP.h Curl.h

PriceWatcher.o: PriceWatcher.h WatchCondition.h Curl.h SMTP.h

WatchCondition.o: WatchCondition.h

SMTP.o: SMTP.h Curl.h

Curl.o:	Curl.h

clean:
	rm -f *.o stock_watcher
