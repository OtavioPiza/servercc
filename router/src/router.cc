#include <functional>
#include <sstream>
#include <stack>

#include "../../request/src/request.hh"
#include "../../response/src/response.hh"
#include "router.hh"

/**
 * @brief adds a handler for a specific verb and path
 *
 * @param method verb
 * @param path path
 * @param handler handler function
 */
void restpp::Router::handle(std::string method, std::string path, handler_t *handler)
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
    if (it == current_handler->handlers.end())
    {
        current_handler->handlers[method] = handler;
    }
    else
    {
        throw std::runtime_error("Already defined handler for " + method + " " + path);
    }
}

/**
 * @brief processses a request and sends a response
 *
 * @param raw_request request string
 * @param socket socket fd
 */
void restpp::Router::process(std::string raw_request, int socket)
{
    Request request(raw_request, socket);
    Response response(socket);

    /* dfs path */

    std::stack<std::pair<unsigned long int, Handler *>> stack;
    stack.push(std::make_pair(0, &this->handler));

    while (!response.is_sent() && !stack.empty())
    {
        auto depth = stack.top().first + 1;
        auto handler = stack.top().second;

        /* check if end of route was reached */

        if (depth == request.path.size())
        {
            auto it = handler->handlers.find(request.method);
            if (it != handler->handlers.end())
            {
                (*it->second)(request, response);
            }
            stack.pop();
            continue;
        }

        /* find explore next path */

        bool matched = false;
        for (int i = handler->next.size() - 1; i >= 0; i--)
        {
            // TODO implment : and *

            if (handler->next[i].first == request.path[depth])
            {
                stack.push(std::make_pair(depth, &handler->next[i].second));
                matched = true;
            }
        }

        if (!matched) // dead end
        {
            stack.pop();
        }
    }
}
