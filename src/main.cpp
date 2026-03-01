// =============================================================================
// main.cpp — Entry point for the Project Planning & Costing application
//
// Build:  mkdir build && cd build && cmake .. && make
// Run:    ./project_planner --docroot ../resources --http-address 0.0.0.0 --http-port 8080
// Open:   http://localhost:8080
// =============================================================================

#include <Wt/WApplication.h>
#include "App.h"

int main(int argc, char** argv)
{
    return Wt::WRun(argc, argv, [](const Wt::WEnvironment& env) {
        return std::make_unique<App>(env);
    });
}
