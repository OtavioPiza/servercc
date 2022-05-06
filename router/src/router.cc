#include <sstream>

#include "router.hh"

/**
 * @brief adds a handler for a specific verb and path
 * 
 * @param method verb
 * @param path path
 * @param handler handler function
 */
void restpp::Router::handle(std::string method, std::string path, std::function<void(Request &, Response &)> handler)
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

    /* create or get handler for path */

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

    auto it = current_handler->handlers.find(method);
    if (it != current_handler->handlers.end())
    {
        it->second = handler;
    }
    else
    {
        throw std::runtime_error("Already defined handler for " + method + " " + path);
    }
}