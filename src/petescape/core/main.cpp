
#include <string>
#include <boost/asio.hpp>
#include <allegro5/allegro.h>

#include "petescape/core/client/client.h"
#include "petescape/core/server/server.h"

/**
 *  main function for Road Trip Disaster: Pet Escape.
 *  This file should be moved elsewhere, but it's
 *  fine for now.
 */
int main( int argc, char **argv )
{
    bool ran = false;
    int output = 0;

    if( argc > 1 )
    {
        if( strcmp( argv[1], "--server" ) == 0 )
        {
            ran = 1;
            output = petescape::core::server::s_main( argc, argv );
        }
        else if( strcmp(argv[1], "--this") == 0){
            char* clientArgv[] = {argv[0], "--client", "127.0.0.1"};
            int clientArgc = 3;
            ran = 1;
            output = petescape::core::client::c_main( clientArgc, clientArgv );
        }
        else if( (strcmp( argv[1], "--client" ) == 0 && argc == 3)  || strcmp(argv[2], "") == 1)
        {
            ran = 1;
            output = petescape::core::client::c_main( argc, argv );
        }
        else
        {
            std::cerr << "Invalid argument:\n --client, --this, or --server only." << std::endl;
            output = 1;
            ran = true;
        }
    }

    if( !ran )
    {
        char* clientArgv[] = {argv[0], "--client", "127.0.0.1"};
        int clientArgc = 3;
        output = petescape::core::client::c_main( clientArgc, clientArgv );
    }

    return output;
}
