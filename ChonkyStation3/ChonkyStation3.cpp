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

#ifdef __XBOX_BUILD

static std::string ShowBootWindow(SDL_Window* host_window, SDL_GLContext host_context)
{
   
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  
    io.IniFilename = NULL;
    ImGui_ImplSDL2_InitForOpenGL(host_window, host_context);
    ImGui_ImplOpenGL3_Init("#version 410");

    std::string chosenElf;
    enum class BootMenuState { MAIN, CHOOSE_ELF, ERROR_NO_USB };
    BootMenuState menuState = BootMenuState::MAIN;

    std::vector<std::string> elfFiles;

    bool done = false;
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
            {
                done = true;
                break;
            }

            if (event.type == SDL_CONTROLLERBUTTONDOWN)
            {
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_B ||
                    event.cbutton.button == SDL_CONTROLLER_BUTTON_START)
                {
                    done = true;
                    break;
                }
            }

            if (event.type == SDL_KEYDOWN)
            {
                
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    done = true;
                    break;
                }
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        int w, h;
        SDL_GetWindowSize(host_window, &w, &h);
        ImGui::SetNextWindowPos(ImVec2(float(w) * 0.5f, float(h) * 0.5f), 
                                ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Always);
        ImGui::Begin("Boot Menu", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        float fullWindowHeight = ImGui::GetWindowSize().y;
        ImGui::Dummy(ImVec2(0.0f, fullWindowHeight * 0.15f)); 

        if (menuState == BootMenuState::MAIN)
        {
            const char* titleText = "ChonkyStation3";
            ImVec2 textSize = ImGui::CalcTextSize(titleText);
            float textPosX = (ImGui::GetWindowSize().x - textSize.x) * 0.5f;
            ImGui::SetCursorPosX(textPosX);
            ImGui::SetWindowFontScale(1.5f);
            ImGui::TextUnformatted(titleText);
            ImGui::SetWindowFontScale(1.0f);

            ImGui::Dummy(ImVec2(0.0f, 40.0f)); 

            float windowWidth = ImGui::GetContentRegionAvail().x;
            float buttonWidth = 120.0f;
            float spacing = 20.0f;
            float totalWidth = (buttonWidth * 2) + spacing;
            float offsetX = (windowWidth - totalWidth) * 0.5f;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

            if (ImGui::Button("Start", ImVec2(buttonWidth, 0)))
            {
                done = true; 
            }
            ImGui::SameLine(0, spacing);

            if (ImGui::Button("Choose ELF", ImVec2(buttonWidth, 0)))
            {
                std::filesystem::path usbPath("E:/");
                if (!std::filesystem::exists(usbPath) || !std::filesystem::is_directory(usbPath))
                {
                    menuState = BootMenuState::ERROR_NO_USB;
                }
                else
                {
                    elfFiles.clear();
                    for (auto& entry : std::filesystem::directory_iterator(usbPath))
                    {
                        if (!entry.is_regular_file()) 
                            continue;
                        if (entry.path().extension().string() == ".elf")
                        {
                            elfFiles.push_back(entry.path().string());
                        }
                    }
                    if (elfFiles.empty())
                    {
                        menuState = BootMenuState::ERROR_NO_USB;
                    }
                    else
                    {
                        menuState = BootMenuState::CHOOSE_ELF;
                    }
                }
            }
        }
        else if (menuState == BootMenuState::CHOOSE_ELF)
        {
            ImGui::TextUnformatted("Select an ELF from E:/");
            ImGui::Separator();

            ImGui::BeginChild("elf_list", ImVec2(0, 250), true);
            for (size_t i = 0; i < elfFiles.size(); i++)
            {
                if (ImGui::Selectable(elfFiles[i].c_str()))
                {
                    chosenElf = elfFiles[i];
                    done = true;
                }
            }
            ImGui::EndChild();

            if (ImGui::Button("Back"))
            {
                menuState = BootMenuState::MAIN;
            }
        }
        else if (menuState == BootMenuState::ERROR_NO_USB)
        {
            ImGui::TextWrapped("Raw single ELF loading requires a USB drive in E:/ "
                               "containing at least one .elf file.\n");
            if (ImGui::Button("Back"))
            {
                menuState = BootMenuState::MAIN;
            }
        }

        ImGui::End();

        ImGui::Render();
        SDL_GL_MakeCurrent(host_window, host_context);
        glViewport(0, 0, w, h);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(host_window);
        SDL_Delay(16);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    return chosenElf;
}

static void ShowCriticalAlertAndFreeze(const std::string& message) 
{
    SDL_Window* currentWindow  = SDL_GL_GetCurrentWindow();
    SDL_GLContext currentContext = SDL_GL_GetCurrentContext();

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; 
    io.IniFilename = NULL;

    ImGui_ImplSDL2_InitForOpenGL(currentWindow, currentContext);
    ImGui_ImplOpenGL3_Init("#version 410");

    while (true) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                exit(0);
            }
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        int w, h;
        SDL_GetWindowSize(currentWindow, &w, &h);
        ImGui::SetNextWindowPos(ImVec2(w * 0.5f, h * 0.5f),
                                ImGuiCond_Always,
                                ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_Always);

        ImGui::Begin("Critical Error", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::TextWrapped("%s", message.c_str());
        ImGui::Text("Application will now freeze.");
        ImGui::End();

        ImGui::Render();
        SDL_GL_MakeCurrent(currentWindow, currentContext);
        glViewport(0, 0, w, h);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(currentWindow);
        SDL_Delay(16);
    }
}

static void ShowAlertWithOK(const std::string& message)
{
    SDL_Window* currentWindow  = SDL_GL_GetCurrentWindow();
    SDL_GLContext currentContext = SDL_GL_GetCurrentContext();

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; 
    io.IniFilename = NULL; 

    ImGui_ImplSDL2_InitForOpenGL(currentWindow, currentContext);
    ImGui_ImplOpenGL3_Init("#version 410");

    bool done = false;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                exit(0);
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
                done = true;
            }
            if (event.type == SDL_CONTROLLERBUTTONDOWN &&
                event.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
                done = true;
            }
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        int w, h;
        SDL_GetWindowSize(currentWindow, &w, &h);
        ImGui::SetNextWindowPos(ImVec2(w * 0.5f, h * 0.5f),
                                ImGuiCond_Always,
                                ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_Always);

        ImGui::Begin("Alert", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::TextWrapped("%s", message.c_str());
        ImGui::Text("Press Enter or A to continue.");
        ImGui::End();

        ImGui::Render();
        SDL_GL_MakeCurrent(currentWindow, currentContext);
        glViewport(0, 0, w, h);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(currentWindow);
        SDL_Delay(16);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

static void PreBootChecks() 
{

    const char* localStatePtr = std::getenv("CHONKYSTATION3_LOCAL_STATE_PATH");
    std::filesystem::path externalPath;
    if (localStatePtr && *localStatePtr) {
        externalPath = std::filesystem::path(localStatePtr) / "dev_flash" / "sys" / "external";
        std::cout << "Using localState-based externalPath: " << externalPath.string() << std::endl;
    } else {
        externalPath = "dev_flash/sys/external";
        std::cout << "No localState path set; using default externalPath: " << externalPath.string() << std::endl;
    }

    if (!std::filesystem::exists(externalPath)) {
        std::cout << "No '" << externalPath.string() 
                  << "' found. Assuming first-time setup. Skipping game/EBOOT checks.\n";
        return;
    }

    int prxCount = 0;
    int picCount = 0;
    for (auto& entry : std::filesystem::directory_iterator(externalPath)) {
        if (!entry.is_regular_file()) continue;
        std::string ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext == ".prx") prxCount++;
        if (ext == ".pic") picCount++;
    }
    if (prxCount == 0 || picCount == 0) {
        ShowCriticalAlertAndFreeze(
            "Mismatch in dev_flash/sys/external: Missing either .prx or .pic files.\n"
            "Application will freeze now."
        );
    }

    std::filesystem::path gamesPath;
    if (localStatePtr && *localStatePtr) {
        gamesPath = std::filesystem::path(localStatePtr) / "dev_hdd0" / "game";
        std::cout << "Using localState-based gamesPath: " << gamesPath.string() << std::endl;
    } else {
        gamesPath = "dev_hdd0/game";
        std::cout << "No localState path set; using default gamesPath: " << gamesPath.string() << std::endl;
    }
    std::vector<std::string> missingGames;

    std::cout << "Checking games path: " << gamesPath.string() << std::endl;
    if (std::filesystem::exists(gamesPath) && std::filesystem::is_directory(gamesPath)) {
        for (auto& dir : std::filesystem::directory_iterator(gamesPath)) {
            if (!dir.is_directory()) continue;

            std::cout << "Found game folder: " << dir.path().string() << std::endl;

            std::filesystem::path usrDir = dir.path() / "USRDIR";
            bool foundEBOOT = false;

            if (std::filesystem::exists(usrDir) && std::filesystem::is_directory(usrDir)) {
                std::cout << "  Checking USRDIR: " << usrDir.string() << std::endl;
                for (auto& file : std::filesystem::directory_iterator(usrDir)) {
                    if (!file.is_regular_file()) continue;
                    std::string filename = file.path().filename().string();
                    std::string lowerFilename = filename;
                    std::transform(lowerFilename.begin(), lowerFilename.end(), lowerFilename.begin(), ::tolower);

                    std::cout << "    Checking file: " << filename << std::endl;
                    if (lowerFilename == "eboot.elf") {
                        std::cout << "    Found EBOOT.elf in " << usrDir.string() << std::endl;
                        foundEBOOT = true;
                        break;
                    }
                }
            } else {
                std::cout << "  USRDIR not found at: " << usrDir.string() << std::endl;
            }

            if (!foundEBOOT) {
                missingGames.push_back(dir.path().filename().string());
                std::cout << "  *** EBOOT.elf missing for " << dir.path().filename().string() << std::endl;
            }
        }
    } else {
        std::cout << "gamesPath not found or not a directory: " << gamesPath.string() << std::endl;
    }

    if (!missingGames.empty()) {
        std::string msg = "The following games are missing EBOOT.elf and may not boot:\n";
        for (auto& name : missingGames) {
            msg += "* " + name + "\n";
        }
        ShowAlertWithOK(msg);
    }
}

extern "C" EXPORT int external_main(SDL_Window* host_window, SDL_GLContext host_context, int argc, char** argv) {
    try {
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
        printf("external_main: Calling Prebootchecks\n");
        

        // Process file path if provided will probably never be used again in UWP (nvm!).
        std::filesystem::path file = "";
        std::filesystem::path localStatePath = "";
        if (argc >= 2) {
            file = argv[1];
            printf("external_main: File provided: %s\n", file.generic_string().c_str());
        } else {
            printf("external_main: No file provided, calling gameSelector\n");
        }

        if (argc >= 3) {
            localStatePath = argv[2];
            printf("external_main: Local state path provided: %s\n", localStatePath.generic_string().c_str());
        
            std::string envVarName = "CHONKYSTATION3_LOCAL_STATE_PATH";
            std::string envVarValue = localStatePath.generic_string();
        
        #ifdef _WIN32
            if (_putenv_s(envVarName.c_str(), envVarValue.c_str()) != 0) {
                printf("external_main: Failed to set environment variable %s\n", envVarName.c_str());
            } else {
                printf("external_main: Environment variable %s set to %s\n", envVarName.c_str(), envVarValue.c_str());
            }
        #else
            if (setenv(envVarName.c_str(), envVarValue.c_str(), 1) != 0) {
                printf("external_main: Failed to set environment variable %s\n", envVarName.c_str());
            } else {
                printf("external_main: Environment variable %s set to %s\n", envVarName.c_str(), envVarValue.c_str());
            }
        #endif
        
        } else {
            printf("external_main: Local state path not provided as argument.\n");
        }
        std::string result = ShowBootWindow(host_window, host_context);
        file = result;
        PreBootChecks();

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
        ShowCriticalAlertAndFreeze(std::string("Exception: ") + e.what());
        return -1;
    } catch (...) {
        fprintf(stderr, "Unknown exception caught in external_main.\n");
        ShowCriticalAlertAndFreeze("Unknown exception caught.\n");
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