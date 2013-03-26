
#include <string>

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

    if( argc > 1 )
    {
        if( strcmp( argv[1], "--server" ) == 0 )
        {
            ran = 1;
            petescape::core::server::s_main( argc, argv );
        }
        else if( strcmp( argv[1], "--client" ) == 0 )
        {
            ran = 1;
            petescape::core::client::c_main( argc, argv );
        }

        std::cerr << "Invalid argument:\n --client or --server only." << std::endl;
        return 1;
    }

    if( !ran )
        petescape::core::client::c_main( argc, argv );

    return 0;
}
