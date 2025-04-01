#pragma once

#include <common.hpp>

#include <functional>
#include <unordered_map>

#include <Settings.hpp>
#include <PPU.hpp>
#include <PPU/Backends/PPUInterpreter.hpp>
#include <SPU.hpp>
#include <SPU/Backends/SPUInterpreter.hpp>
#include <RSX.hpp>
#include <Memory.hpp>
#include <ELF/ELFLoader.hpp>
#include <SFO/SFOLoader.hpp>
#include <Game/GameLoader.hpp>
#include <ElfSymbolParser.hpp>
#include <ModuleManager.hpp>
#include <ThreadManager.hpp>
#include <SPUThreadManager.hpp>
#include <HandleManager.hpp>
#include <PRXManager.hpp>
#include <Lv2ObjectManager.hpp>
#include <Syscall.hpp>
#include <Scheduler.hpp>
#include <Filesystem.hpp>
#include <CrashAnalyzer.hpp>


class PlayStation3 {
public:
    PlayStation3(const fs::path& executable = "");
    ~PlayStation3();
    Memory mem = Memory();
    PPU* ppu;
    PPUInterpreter interpreter;
    SPU* spu;
    SPUInterpreter spu_interpreter;
    RSX rsx;
    ModuleManager module_manager;
    ThreadManager thread_manager;
    SPUThreadManager spu_thread_manager;
    HandleManager handle_manager;
    PRXManager prx_manager;
    Lv2ObjectManager lv2_obj;
    Syscall syscall;
    ElfSymbolParser elf_parser;
    Scheduler scheduler;
    Filesystem fs;
    
    Settings settings;
    CrashAnalyzer crash_analyzer;
    GameLoader::InstalledGame curr_game;
    fs::path elf_path;
    std::string elf_path_encrypted;

    std::function<void(void)> flip_handler;
    u64 cycle_count = 0;
    u64 curr_block_cycles = 0;
    u64 curr_block = 0;
    u64 skipped_cycles = 0;
    bool force_scheduler_update = false;

    void gameSelector();
    void loadGame(const GameLoader::InstalledGame& game);
    void setFlipHandler(std::function<void(void)> const& handler);
    void init();
    void run();
    void step();
    void printCrashInfo(std::runtime_error err);
    void flip();
    void vblank();
    void skipToNextEvent();
    void forceSchedulerUpdate();

    void pressButton(u32 button);
    void resetButtons();

    // Debugging
    void enableSPUOnPC(u32 unused);
    u32 enable_spu_on_pc = 0;
    std::string spu_thread_to_enable = "";
};
