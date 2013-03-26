
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

    std::cout << argv[0] << std::endl;
    std::cout << argv[1] << std::endl;

    if( argc > 1 )
    {
        if( strcmp( argv[1], "--server" ) == 0 )
        {
            ran = 1;
            petescape::core::server::s_main( argc, argv );
        }
    }

    if( !ran )
        petescape::core::client::c_main( argc, argv );

    return 0;
}
