#include <string>
#include <sstream>

#include "router.hh"

void restpp::Router::handle(std::string method, std::string path, bool (*handler)(int, int))
{
    /* parse path */

    std::vector<std::string> path_parts;
    std::stringstream ss(path);
    std::string part;

    while (std::getline(ss, part, '/'))
    {
        if (part.size() > 0)
        {
            path_parts.push_back(part);
        }
    }

    Handler *current_handler = &this->handler;

    for (auto &part : path_parts)
    {
        bool found = false;

        /* try to find existing path */

        for (auto &next : current_handler->next)
        {
            if (next.first == part)
            {
                current_handler = &next.second;
                found = true;
                break;
            }
        }

        /* create new path */

        if (!found)
        {
            Handler new_handler;
            current_handler->next.push_back(std::make_pair(part, new_handler));
            current_handler = &current_handler->next.back().second;
        }
    }

    /* set handler */

    if (method == "GET")
    {
        if (current_handler->handle_get)
        {
            throw std::runtime_error("Handler for GET already set for path " + path);
        }
        current_handler->handle_get = handler;
    }

    if (method == "POST")
    {
        if (current_handler->handle_post)
        {
            throw std::runtime_error("Handler for POST already set for path " + path);
        }
        current_handler->handle_post = handler;
    }

    if (method == "PUT")
    {
        if (current_handler->handle_put)
        {
            throw std::runtime_error("Handler for PUT already set for path " + path);
        }
        current_handler->handle_put = handler;
    }

    if (method == "PATCH")
    {
        if (current_handler->handle_patch)
        {
            throw std::runtime_error("Handler for PATCH already set for path " + path);
        }
        current_handler->handle_patch = handler;
    }

    if (method == "DELETE")
    {
        if (current_handler->handle_delete)
        {
            throw std::runtime_error("Handler for DELETE already set for path " + path);
        }
        current_handler->handle_delete = handler;
    }

    if (method == "OPTIONS")
    {
        if (current_handler->handle_options)
        {
            throw std::runtime_error("Handler for OPTIONS already set for path " + path);
        }
        current_handler->handle_options = handler;
    }
}