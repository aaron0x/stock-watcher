#ifndef SMTP_H
#define SMTP_H

#include <map>
#include <string>

class SMTP
{
public:
   SMTP(const std::string &smtp, const std::string &user, const std::string &password);

   ~SMTP();

   void addHeader(const std::string &name, const std::string &value);

   void appendBody(const std::string &content);

   void appendBodyWithNewLine(const std::string &content);

   std::string header() const;

   std::string body() const;

   bool emptyContent() const;

   void send(const std::string &address);

private:
   static const std::string NEW_LINE;

   typedef struct Data {
      const std::string &message;
      size_t offset;
   } Data;

   static size_t handleRead(char *buffer, size_t size, size_t num, void *userData);

   std::string smtp_;

   std::string user_;

   std::string password_;

   std::map<std::string, std::string> header_;

   std::string body_;
};

#endif /* SMTP_H */