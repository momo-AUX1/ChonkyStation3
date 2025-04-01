#pragma once

#include <common.hpp>

#include <unordered_map>
#include <format>

#include <Import.hpp>
#include <PRX/PRXExport.hpp>

#include <Modules/SysPrxForUser.hpp>
#include <Modules/SysThread.hpp>
#include <Modules/SysLwMutex.hpp>
#include <Modules/SysLwCond.hpp>
#include <Modules/SysMMapper.hpp>
#include <Modules/CellGcmSys.hpp>
#include <Modules/CellVideoOut.hpp>
#include <Modules/CellSysutil.hpp>
#include <Modules/CellSysmodule.hpp>
#include <Modules/CellResc.hpp>
#include <Modules/CellGame.hpp>
#include <Modules/CellSpurs.hpp>
#include <Modules/CellRtc.hpp>
#include <Modules/CellFs.hpp>
#include <Modules/CellPngDec.hpp>
#include <Modules/SceNpTrophy.hpp>
#include <Modules/SceNp.hpp>
#include <Modules/CellSaveData.hpp>
#include <Modules/CellPad.hpp>
#include <Modules/CellKb.hpp>
#include <Modules/CellSsl.hpp>
#include <Modules/CellSysCache.hpp>
#include <Modules/CellMsgDialog.hpp>
#include <Modules/CellAudio.hpp>
#include <Modules/CellAudioOut.hpp>


// Circular dependency
class PlayStation3;

class ModuleManager {
public:
    ModuleManager(PlayStation3* ps3) :  ps3(ps3), sysPrxForUser(ps3), sysThread(ps3), sysLwMutex(ps3), sysLwCond(ps3), sysMMapper(ps3), cellGcmSys(ps3), cellVideoOut(ps3), cellSysutil(ps3),
                                        cellSysmodule(ps3), cellResc(ps3), cellGame(ps3), cellSpurs(ps3), cellRtc(ps3), cellFs(ps3), cellPngDec(ps3), sceNpTrophy(ps3),
                                        sceNp(ps3), cellSaveData(ps3), cellPad(ps3), cellKb(ps3), cellSsl(ps3), cellSysCache(ps3), cellMsgDialog(ps3), cellAudio(ps3),
                                        cellAudioOut(ps3) {}
    PlayStation3* ps3;

    void call(u32 nid);
    void lle(u32 nid);
    // Map address to import nid
    void registerImport(u32 addr, u32 nid);
    void registerExportTable(const PRXExportTable& exports);
    PRXExportTable getExportTable() { return exports; }
    std::unordered_map<u32, u32> imports = {};
    PRXExportTable exports;

    std::unordered_map<u32, Import> import_map;
    void init();

    std::string getImportName(const u32 nid);
    bool isForcedHLE(const u32 nid);
    std::string last_call;

    SysPrxForUser sysPrxForUser;
    SysThread sysThread;
    SysLwMutex sysLwMutex;
    SysLwCond sysLwCond;
    SysMMapper sysMMapper;
    CellGcmSys cellGcmSys;
    CellVideoOut cellVideoOut;
    CellSysutil cellSysutil;
    CellSysmodule cellSysmodule;
    CellResc cellResc;
    CellGame cellGame;
    CellSpurs cellSpurs;
    CellRtc cellRtc;
    CellFs cellFs;
    CellPngDec cellPngDec;
    SceNpTrophy sceNpTrophy;
    SceNp sceNp;
    CellSaveData cellSaveData;
    CellPad cellPad;
    CellKb cellKb;
    CellSsl cellSsl;
    CellSysCache cellSysCache;
    CellMsgDialog cellMsgDialog;
    CellAudio cellAudio;
    CellAudioOut cellAudioOut;

    u64 stub();

    u32 last_lle_nid = 0;
    void printReturnValue(u64 addr);

private:
    MAKE_LOG_FUNCTION(log, lle_module);
    MAKE_LOG_FUNCTION(unimpl, unimplemented);
};