#ifndef RESTPP_SERVER_HH
#define RESTPP_SERVER_HH

namespace restpp
{
    class Server
    {
    private:
        int port; // Port to listen on

    public:
        Server(int port);
    };
}

#endif