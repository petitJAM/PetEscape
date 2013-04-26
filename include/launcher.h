#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <Python.h>

#define START_FUNCTION "start_server"
#define KILL_FUNCTION  "kill_server"

class Launcher
{
public:
    Launcher();
    ~Launcher();

    void start( const char *args );
    void kill();
    char *getIP();

private:
    PyObject *server_module_name;
    PyObject *server_module;
    PyObject *server_module_contents;
    PyObject *server_start_func;
    PyObject *server_kill_func;

    PyObject *ip_module_name;
    PyObject *ip_module;
    PyObject *ip_module_contents;
    PyObject *ip_module_getip_func;

    PyThreadState *mainThreadState, *myThreadState, *tempState;
    PyInterpreterState *mainInterpreterState;
};

#endif // LAUNCHER_H
