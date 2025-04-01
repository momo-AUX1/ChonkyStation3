#include <iostream>
#include <filesystem>
#include <SDL.h>
#ifdef _WIN32
#include <windows.h>
#endif



#ifdef __XBOX_BUILD
#include <cstdio>
#include <fstream>
#include <algorithm>
#include <vector>
#include <string>
#include <imgui.h>
#include <cstdlib>
#include <stdexcept>
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#endif

#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT __attribute__((visibility("default")))
#endif

#include <Frontend/GameWindow.hpp>
#ifdef CHONKYSTATION3_QT_BUILD
#include <Frontend/MainWindow.hpp>
#else
#include "PlayStation3.hpp" // Not needed in Qt builds because it's included in MainWindow.hpp
#endif

#ifdef _WIN32
#include <windows.h>
// Gently ask to use the discrete Nvidia/AMD GPU if possible instead of
// integrated graphics
extern "C" {
__declspec(dllexport) DWORD NvOptimusEnablement = 1;
__declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 1;
}
#endif


int main(int argc, char** argv) {
    printf("ChonkyStation3\n\n");

#ifdef CHONKYSTATION3_QT_BUILD
    QApplication::setStyle("fusion");
    QCoreApplication::addLibraryPath("./Qt6");
    QApplication app(argc, argv);
    MainWindow main_window = MainWindow();
    int ret = app.exec();
    main_window.onExit();
    return ret;
#else
    std::filesystem::path file = "";
    if (argc >= 2)
        file = argv[1];

    PlayStation3* ps3 = new PlayStation3(file);
    if (argc == 1)
        ps3->gameSelector(); // Launch game selector if no file path is provided.
    ps3->init();

    GameWindow game_window;
    game_window.run(ps3);
    return 0;
#endif
}
#endif