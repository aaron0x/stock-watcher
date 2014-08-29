/****************************************************
 *** Copyright (c) 2014 Aaron aaron1126@gmail.com ***
 ***                                              ***
 *** See the file LICENSE for copying permission. ***
****************************************************/


#include "SMTP.h"
#include "Curl.h"

#include <cstring>
#include <stdexcept>

using namespace std;

const string SMTP::NEW_LINE = "\r\n";

// TODO: Check smtp format.
SMTP::SMTP(const string &smtp, const string &user, const string &password)
   : smtp_("smtp://" + smtp), user_(user), password_(password)
{
   if (smtp.empty()) {
      throw runtime_error("empty smtp address!");
   }
}

SMTP::~SMTP()
{
}

void SMTP::addHeader(const string &name, const string &value)
{
   header_[name] = value;
}

void SMTP::appendBody(const string &content)
{
   body_.append(content);
}

void SMTP::appendBodyWithNewLine(const string &content)
{
   body_ += content + NEW_LINE;
}

string SMTP::header() const
{
   string header;
   for (const auto &h : header_) {
      header += h.first + ": " + h.second + NEW_LINE;
   }

   return header;
}

string SMTP::body() const
{
   return body_;
}

bool SMTP::emptyContent() const
{
   return body_.empty() && header_.empty();
}

// TODO: Check address format.
void SMTP::send(const string &address)
{
   if (address.empty()) {
      throw runtime_error("send smtp to empty address!");
   }


   if (emptyContent()) {
      throw runtime_error("send empty mail!");
   }

   Curl   curl;
   string message               = header() + NEW_LINE + body();
   Data   data                  = {.message = message, .offset = 0 };
   struct curl_slist *recipient = nullptr;
   recipient                    = curl_slist_append(recipient, address.c_str());

   curl.setOption(CURLOPT_USERNAME,     user_.c_str());
   curl.setOption(CURLOPT_PASSWORD,     password_.c_str());
   curl.setOption(CURLOPT_URL,          smtp_.c_str());
   curl.setOption(CURLOPT_USE_SSL,      CURLUSESSL_ALL);
   curl.setOption(CURLOPT_MAIL_RCPT,    recipient);
   curl.setOption(CURLOPT_READFUNCTION, handleRead);
   curl.setOption(CURLOPT_READDATA,     &data);
   curl.setOption(CURLOPT_UPLOAD,       1);

   curl.perform();
}

size_t SMTP::handleRead(char *buffer, size_t size, size_t num, void *userData)
{
   size_t total = size * num;
   Data *data = static_cast<Data *>(userData);
   if (!buffer || total == 0 || !data) {
      return 0;
   }

   const string &message = data->message;
   size_t       &offset  = data->offset;
   if (offset >= message.size()) {
      return 0;
   }

   if (message.size() < total) {
      memcpy(buffer, message.c_str(), message.size());
      offset = message.size();
   } else {
      memcpy(buffer, message.c_str(), total);
      offset = total;
   }

   return offset;
}

