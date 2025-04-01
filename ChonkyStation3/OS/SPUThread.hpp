#pragma once

#include <common.hpp>
#include <logger.hpp>

#include <thread>
#include <atomic>
#include <queue>

#include <SPUTypes.hpp>
#include <OS/Syscalls/sys_spu.hpp>
#include <Lv2Objects/Lv2EventQueue.hpp>


using namespace SPUTypes;
using namespace sys_spu;

// Circular dependency
class PlayStation3;

class SPUThread {
public:
    SPUThread(PlayStation3* ps3, std::string name);
    PlayStation3* ps3;

    SPUTypes::State state;
    u8* ls = new u8[256_KB];

    u32 id;
    std::string name;

    enum class ThreadStatus {
        Ready,
        Running,
        Waiting,
        Terminated
    };

    ThreadStatus status = ThreadStatus::Ready;
    bool isRunning();

    void loadImage(sys_spu_image* img);
    
    void reschedule();
    void halt();
    void wait();
    void wakeUp();

    static std::string threadStatusToString(ThreadStatus status) {
        switch (status) {
        case ThreadStatus::Running:    return "Running";
        case ThreadStatus::Waiting:    return "Waiting";
        case ThreadStatus::Terminated: return "Terminated";
        }
    }

    // MFC
    enum SPU_Channel : u32 {
        SPU_RdEventStat     = 0,
        SPU_WrEventMask     = 1,
        SPU_WrEventAck      = 2,
        SPU_RdSigNotify1    = 3,
        SPU_RdSigNotify2    = 4,
        SPU_WrDec           = 7,
        SPU_RdDec           = 8,
        SPU_RdEventMask     = 11,
        SPU_RdMachStat      = 13,
        SPU_WrSRR0          = 14,
        SPU_RdSRR0          = 15,	
        SPU_WrOutMbox       = 28,
        SPU_RdInMbox        = 29,
        SPU_WrOutIntrMbox   = 30,
    };

    enum MFC_Channel: u32 {
        MFC_WrMSSyncReq     = 9,
        MFC_RdTagMask       = 12,
        MFC_LSA             = 16,
        MFC_EAH             = 17, 
        MFC_EAL             = 18,
        MFC_Size            = 19,
        MFC_TagID           = 20,
        MFC_Cmd             = 21,
        MFC_WrTagMask       = 22,
        MFC_WrTagUpdate     = 23,
        MFC_RdTagStat       = 24,
        MFC_RdListStallStat = 25,
        MFC_WrListStallAck  = 26,
        MFC_RdAtomicStat    = 27,
    };

    enum MFC_Command : u32 {
        PUT     = 0x20,
        GET     = 0x40,
        PUTLLC  = 0xb4,
        GETLLAR = 0xd0,
    };

    u32 lsa = 0;
    u32 eah = 0;
    u32 eal = 0;
    u32 size = 0;
    u32 tag_id = 0;
    u32 tag_mask = 0;
    u32 atomic_stat = 0;

    std::queue<u32> in_mbox = {};
    std::queue<u32> out_mbox = {};
    u32 ports[64];

    union {
        u32 raw = 0;

        BitField<0,  1, u32> tg;
        BitField<1,  1, u32> sn;
        BitField<2,  1, u32> reserved;
        BitField<3,  1, u32> qv;
        BitField<4,  1, u32> mb;
        BitField<5,  1, u32> tm;
        BitField<6,  1, u32> me;
        BitField<7,  1, u32> le;
        BitField<8,  1, u32> s2;
        BitField<9,  1, u32> s1;
        BitField<10, 1, u32> lr;
        BitField<11, 1, u32> a;
        BitField<12, 1, u32> ms;
    } event_stat;
    u32 event_mask = 0;

    struct Reservation {
        u32 addr;
        u8 data[128];
    };
    Reservation reservation;

    class LocklineWaiter {
    public:
        LocklineWaiter(PlayStation3* ps3, u32 waiter_id) : ps3(ps3), waiter_id(waiter_id) {}
        PlayStation3* ps3;
        u32 waiter_id;

        void waiter();
        void begin(Reservation reservation);
        void end();

    private:
        std::atomic<bool> is_waiting = false;
        std::atomic<bool> acquired = false;
        Reservation reservation;
        std::thread waiter_thread;
    };
    LocklineWaiter* lockline_waiter;

    void sendLocklineLostEvent(u32 addr);
    void wakeUpIfEvent();
    bool hasPendingEvents();

    std::string channelToString(u32 ch);
    u32  readChannel(u32 ch);
    u32  readChannelCount(u32 ch);
    void writeChannel(u32 ch, u32 val);
    void doCmd(u32 cmd);

private:
    MAKE_LOG_FUNCTION(log, thread_spu);
};