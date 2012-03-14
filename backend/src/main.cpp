#include <iostream>
#include <vector>
#include <string>
#include "vector.h"
#include "dummyscene.h"
#include "renderer.h"
#include <csignal>

#include "backendapplication.h"

int main(int argc, char * argv[])
{
    // Parse command line parameters to a vector of strings
    std::vector<std::string> args(argv, argv+argc);
    
    signal(SIGPIPE, SIG_IGN);

    if (args.size() > 1) {
        dmcr::BackendApplication app;

        try {
            app.connect(args[1], 9393);
            app.start();
        } catch (const std::runtime_error& e) {
            std::cout << "Exception: " << e.what() << std::endl;
        }
    } else {
        std::cout << "Usage: " << args.at(0) << " <address>" << std::endl;
        std::cout << "Connects to DMCR frontend located at <address> " <<
                     "port 9393" << std::endl;
        return 1;
    }

    return 0;
}

