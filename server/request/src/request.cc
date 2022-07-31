#include <sstream>

#include "request.hh"

restpp::Request::Request(std::string raw_request, int socket)
{
    this->raw_request = raw_request;
    this->socket = socket;
    this->parse_request();
}

void restpp::Request::parse_request()
{
    // auto err = llhttp_execute(&this->parser, this->raw_request.c_str(), this->raw_request.size());
    // if (err != HPE_OK)
    // {
    //     this->error = std::string("errror");
    // }
}
