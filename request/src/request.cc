#include "request.hh"
#include <sstream>

restpp::Request::Request(std::string raw_request)
{
    this->raw_request = raw_request;
    this->parse_request();
}

void restpp::Request::parse_request()
{
    std::stringstream ss(this->raw_request);
    std::string line;

    /* parse first line */

    {
        std::getline(ss, line);
        std::stringstream ss_line(line);
        std::string raw_path;

        std::getline(ss_line, this->method, ' ');
        std::getline(ss_line, raw_path, ' ');
        std::getline(ss_line, this->version, '\r');

        std::string raw_params = raw_path.substr(raw_path.find('?') + 1);
        raw_path = raw_path.substr(0, raw_path.find('?'));

        /* parse params */

        std::stringstream ss_params(raw_params);
        std::string param;

        while (std::getline(ss_params, param, '&'))
        {
            std::stringstream ss_param(param);
            std::string key;
            std::string value;
            this->params[key] = value;
        }

        /* parse path */

        std::stringstream ss_path(raw_path);
        std::string segment;

        while (std::getline(ss_path, segment, '/'))
        {
            if (segment.empty())
            {
                continue;
            }
            this->path.push_back(segment);
        }
    }

    /* parse headers */

    while (std::getline(ss, line))
    {
        if (line.empty())
        {
            break;
        }

        std::stringstream ss_line(line);
        std::string key;
        std::string value;

        std::getline(ss_line, key, ':');
        std::getline(ss_line, value);

        this->headers[key] = value;
    }
}
