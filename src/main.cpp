#include "ui/Application.h"

// ============================================================
// main.cpp — program entry point.
//
// Everything is encapsulated inside Application.
// main() constructs it, calls run(), and returns the exit code.
// run() returns 0 on success, 1 if SDL initialisation failed.
// ============================================================

int main(int argc, char* argv[])
{
    Application app;
    return app.run();
}