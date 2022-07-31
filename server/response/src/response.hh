#ifndef RESPONSE_HH
#define RESPONSE_HH

namespace restpp
{
    /**
     * @brief The Response class
     * @details The response class is used to send a response to the user
     */
    class Response
    {
    public:
        int socket; // socket fd

        /**
         * @brief Construct a new Response object
         *
         * @param socket socket file descriptor
         */
        Response(int socket);

        /**
         * @brief checks if a response was already sent
         * 
         * @return true if a reaspose was already sent 
         * @return false if a response was not yet sent
         */
        bool is_sent();

    private:
        bool _sent; // whether the response was sent
    };
}

#endif