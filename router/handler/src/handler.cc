#include "handler.hh"

restpp::Handler::Handler()
{
    this->handle_get = nullptr;
    this->handle_post = nullptr;
    this->handle_put = nullptr;
    this->handle_patch = nullptr;
    this->handle_delete = nullptr;
    this->handle_options = nullptr;
};
