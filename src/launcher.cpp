#include "launcher.h"
#include <Python.h>
#include <stdio.h>

Launcher::Launcher()
{
    // Set up Python Environment
    server_module_name = NULL;
    server_module = NULL;
    server_module_contents = NULL;
    server_start_func = NULL;
    server_kill_func  = NULL;
    ip_module_name = NULL;
    ip_module = NULL;
    ip_module_contents = NULL;
    ip_module_getip_func = NULL;

    Py_Initialize();
    char **argv = (char**)malloc( sizeof( char * ) * 2 );
    argv[0] = (char*)malloc( sizeof( char ) * 7 );
    strcpy( argv[0], "Python\0" );

    PySys_SetArgv( 1, argv );

    // Initialize thread support
    PyEval_InitThreads();

    server_module_name     = PyString_FromString( "server_mod" );
    server_module          = PyImport_Import( server_module_name );
    server_module_contents = PyModule_GetDict( server_module );

    ip_module_name     = PyString_FromString( "get_ip" );
    ip_module          = PyImport_Import( ip_module_name );
    ip_module_contents = PyModule_GetDict( ip_module );

    server_start_func    = PyDict_GetItemString( server_module_contents, "start_server" );
    server_kill_func     = PyDict_GetItemString( server_module_contents, "kill_server" );
    ip_module_getip_func = PyDict_GetItemString( ip_module_contents,     "get_ip_address" );
}

void Launcher::start( const char *args )
{
    if( PyCallable_Check( server_start_func ) )
    {
        PyObject *py_args = PyTuple_New( 1 );
        PyTuple_SetItem( py_args, 0, PyString_FromString( args ) );

        PyObject *result = PyObject_CallObject( server_start_func, py_args );
        if( result == nullptr )
        {
            PyErr_Print();
            return;
        }
        Py_DECREF( py_args );
        Py_DECREF( result );
    }
}

void Launcher::kill()
{
    PyObject *result = PyObject_CallObject( server_kill_func, NULL );
    if( result == nullptr )
    {
        PyErr_Print();
        return;
    }
    Py_DECREF( result );
}

char *Launcher::getIP()
{
    char *ret_value;

//    PyGILState_STATE state = PyGILState_Ensure();
    PyObject *result = PyObject_CallObject( ip_module_getip_func, NULL );
    if( result == nullptr )
    {
        PyErr_Print();
        return nullptr;
    }

    if( result == Py_None )
    {
        ret_value = nullptr;
    }
    else
    {
        ret_value = strdup( PyString_AsString( result ) );
    }

    Py_DECREF( result );

//    PyGILState_Release( state );
    return ret_value;
}

Launcher::~Launcher()
{
//     Swap out the current thread
//    PyThreadState_Swap(tempState);

    // Release global lock
//    PyEval_ReleaseLock();

//    // Clean up thread state
//    PyThreadState_Clear(myThreadState);
//    PyThreadState_Delete(myThreadState);

    printf( "CLEANUP\n" );

    Py_DECREF( server_start_func );
    Py_DECREF( server_kill_func );
    Py_DECREF( ip_module_getip_func );
    Py_DECREF( server_module_name );
    Py_DECREF( server_module );
    Py_DECREF( server_module_contents );
    Py_DECREF( ip_module_name );
    Py_DECREF( ip_module );
    Py_DECREF( ip_module_contents );

    Py_Finalize();
}
