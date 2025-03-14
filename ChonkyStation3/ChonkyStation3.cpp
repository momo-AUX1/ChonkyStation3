#include <iostream>
#include <filesystem>
#include <SDL.h>
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __XBOX_BUILD
#include <cstdio>
#include <fstream>
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

#ifdef __XBOX_BUILD

void redirect_output(const char* filename) {
    static std::ofstream log_file(filename, std::ios::trunc);

    if (!log_file) {
        std::cerr << "Failed to open log file!" << std::endl;
        return;
    }

    FILE* fp = freopen(filename, "w", stdout);
    if (!fp) {
        std::cerr << "Failed to redirect stdout!" << std::endl;
    }

    std::cout.rdbuf(log_file.rdbuf());
}

extern "C" EXPORT int external_main(SDL_Window* host_window, SDL_GLContext host_context, int argc, char** argv) {
    try {
        redirect_output("E:/PS3_LOGS.txt");
        printf("ChonkyStation3 external_main: Started\n");
        
        // Make the GL context current on this thread.
        if (SDL_GL_MakeCurrent(host_window, host_context) != 0) {
            printf("external_main: SDL_GL_MakeCurrent failed: %s\n", SDL_GetError());
            return -1;
        }
        
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
            Helpers::panic("OpenGL init failed");
        }
        printf("external_main: GL context is now current\n");

        // Process file path if provided will probably never be used again in UWP.
        std::filesystem::path file = "";
        if (argc >= 2) {
            file = argv[1];
            printf("external_main: File provided: %s\n", file.generic_string().c_str());
        } else {
            printf("external_main: No file provided, calling gameSelector\n");
        }

        PlayStation3* ps3 = new PlayStation3(file);
        if (file.empty()) {
            printf("external_main: Calling gameSelector()\n");
            ps3->gameSelector();
        }
        printf("external_main: Initializing PS3\n");
        ps3->init();

        // Validate the host window.
        if (!host_window) {
            printf("external_main: Error: Provided SDL window pointer is null.\n");
            return -1;
        }
        printf("external_main: Using provided SDL window: %p\n", host_window);

        GameWindow game_window(host_window);
        printf("external_main: GameWindow constructed\n");

        printf("external_main: Calling game_window.run()\n");
        game_window.run(ps3);
        printf("external_main: Finished run()\n");

        return 0;
    } catch (const std::exception& e) {
        fprintf(stderr, "Exception caught in external_main: %s\n", e.what());
        return -1;
    } catch (...) {
        fprintf(stderr, "Unknown exception caught in external_main.\n");
        return -1;
    }
}

#else

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