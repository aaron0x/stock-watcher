/****************************************************
 *** Copyright (c) 2014 Aaron aaron1126@gmail.com ***
 ***                                              ***
 *** See the file LICENSE for copying permission. ***
****************************************************/


#include "Config.h"

#include <iostream>
#include <exception>
#include <unistd.h>

using namespace std;

void usage()
{
   cerr << "Use: program -c cofig_file\n";
}

int main(int argc, char **argv)
{
   int option = getopt(argc, argv, "c:");
   if (option == -1 || option == '?') {
      usage();
      return EXIT_FAILURE;
   }

   try {
      Config config(optarg);
      config.load();
      config.checkPrice();
   } catch (const exception &e) {
      cerr << "caught exception: " << e.what() << endl;
      return EXIT_FAILURE;
   }

   return EXIT_SUCCESS;
}
