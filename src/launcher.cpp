#include "launcher.h"
#include <Python.h>

Launcher::Launcher()
{
    // Set up Python Environment
    module_name   = NULL;
    server_module = NULL;
    server_module_contents = NULL;
    server_start_func = NULL;
    server_kill_func  = NULL;

    Py_Initialize();

    module_name = PyString_FromString( "server_mod" );
    server_module = PyImport_Import( module_name );
    server_module_contents = PyModule_GetDict( server_module );

    server_start_func = PyDict_GetItemString( server_module_contents, "start_server" );
    server_kill_func = PyDict_GetItemString( server_module_contents, "kill_server" );
}

void Launcher::start( const char *args )
{
    if( PyCallable_Check( server_start_func ) )
    {
        PyObject *py_args = PyTuple_New( 1 );
        PyTuple_SetItem( py_args, 0, PyString_FromString( args ) );

        PyObject_CallObject( server_start_func, py_args );
    }
}

void Launcher::kill()
{
    PyObject_CallObject( server_kill_func, NULL );
}

Launcher::~Launcher()
{
    Py_Finalize();
}
