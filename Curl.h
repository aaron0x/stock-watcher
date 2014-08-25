/****************************************************
 *** Copyright (c) 2014 Aaron aaron1126@gmail.com ***
 ***                                              ***
 *** See the file LICENSE for copying permission. ***
****************************************************/


#ifndef CURL_H
#define CURL_H

#include <curl/curl.h>

#include <stdexcept>
#include <memory>

class Curl
{
public:
   Curl();

   ~Curl();

   template<typename T>
   void setOption(CURLoption option, T parameter)
   {
      CURLcode ret = curl_easy_setopt(handle_.get(), option, parameter);
      if (ret != CURLE_OK) {
         throw std::runtime_error(curl_easy_strerror(ret));
      }
   }

   void perform();

private:
   std::unique_ptr<CURL, void(*)(CURL *)> handle_ = { NULL, curl_easy_cleanup };
};

#endif /* CURL_H */
