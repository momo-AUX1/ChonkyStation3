#include "SPUThread.hpp"
#include "PlayStation3.hpp"


SPUThread::SPUThread(PlayStation3* ps3, std::string name) : ps3(ps3) {
    id = ps3->handle_manager.request();
    this->name = name;
    log("Created SPU thread %d \"%s\"\n", id, name.c_str());

    lockline_waiter = new LocklineWaiter(ps3, id);
}

bool SPUThread::isRunning() {
    return status == ThreadStatus::Running;
}

void SPUThread::loadImage(sys_spu_image* img) {
    log("Importing SPU image for thread %d \"%s\"\n", id, name.c_str());

    // Copy SPU image to LS
    sys_spu_segment* segs = (sys_spu_segment*)ps3->mem.getPtr(img->segs_ptr);
    for (int i = 0; i < img->n_segs; i++) {
        switch (segs[i].type) {

        case SYS_SPU_SEGMENT_TYPE_COPY: {
            log("*  Loading segment %d type COPY\n", i);
            std::memcpy(&ls[segs[i].ls_addr], ps3->mem.getPtr(segs[i].src.addr), segs[i].size);
            log("*  Copied segment from main[0x%08x - 0x%08x] to ls[0x%08x - 0x%08x]\n", (u32)segs[i].src.addr, segs[i].src.addr + segs[i].size, (u32)segs[i].ls_addr, segs[i].ls_addr + segs[i].size);
            break;
        }

        default: {
            Helpers::panic("Unimplemented segment type %d\n", (u32)segs[i].type);
        }
        }
    }

    log("Entry point: 0x%08x\n", (u32)img->entry);
    state.pc = img->entry;

    /*
    std::string filename = std::format("ls{:d}.bin", id);
    std::ofstream file(filename, std::ios::binary);
    file.write((char*)ls, 16_KB);
    file.close();
    */
}

void SPUThread::reschedule() {
    ps3->scheduler.push(std::bind(&SPUThreadManager::reschedule, &ps3->spu_thread_manager), ps3->curr_block_cycles, "spu thread reschedule");
    ps3->forceSchedulerUpdate();
}

void SPUThread::halt() {
    Helpers::panic("Halting thread %d \"%s\"\n", id, name.c_str());
    status = ThreadStatus::Terminated;
    reschedule();
}

void SPUThread::wait() {
    log("Thread %d \"%s\" is waiting\n", id, name.c_str());
    status = ThreadStatus::Waiting;
    reschedule();
}

void SPUThread::wakeUp() {
    log("Woke up thread %d \"%s\"\n", id, name.c_str());
    status = ThreadStatus::Running;
    reschedule();
}

void SPUThread::LocklineWaiter::waiter() {
    //printf("waiter\n");
    u64 curr = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    u64 elapsed;
    
    while (is_waiting) {
        // Did the data change?
        if (std::memcmp(reservation.data, ps3->mem.getPtr(reservation.addr), 128)) {
            is_waiting = false;
        }

        // Check for timeout
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - curr;
        if (elapsed > 1000) {
            //printf("Timeout\n");
            //exit(0);
            is_waiting = false;
        }
    }
    
    // The lockline reservation was lost
    if (!acquired)  // Set by i.e. PUTLLC
        ps3->spu_thread_manager.getThreadByID(waiter_id)->sendLocklineLostEvent(reservation.addr);
}

void SPUThread::LocklineWaiter::begin(Reservation reservation) {
    //printf("begin\n");
    if (is_waiting) {
        // End the current waiter if it was already active
        acquired = true;
        end();  
    }
    // This happens when the lockline is written, waiter() will set is_waiting to false but the thread will still be waiting to be joined
    if (waiter_thread.joinable()) {
        acquired = true;
        waiter_thread.join();
    }

    this->reservation = reservation;
    is_waiting = true;
    acquired = false;
    waiter_thread = std::thread(&LocklineWaiter::waiter, this);
}

void SPUThread::LocklineWaiter::end() {
    //printf("end\n");
    //if (!is_waiting) Helpers::panic("LocklineWaiter: tried to end waiter, but the waiter was already inactive\n");

    is_waiting = false;
    acquired = true;
    waiter_thread.join();
}

void SPUThread::sendLocklineLostEvent(u32 addr) {
    if (Helpers::inRangeSized<u32>(addr, reservation.addr, 128)) {
        //if (std::memcmp(reservation.data, ps3->mem.getPtr(reservation.addr), 128)) {
            //log("Lockline lost, sending event\n");
            event_stat.lr = true;
            reservation.addr = 0;
            wakeUpIfEvent();
        //}
    }
}

void SPUThread::wakeUpIfEvent() {
    if (hasPendingEvents())
        wakeUp();
}

bool SPUThread::hasPendingEvents() {
    return event_stat.raw & event_mask;
}

std::string SPUThread::channelToString(u32 ch) {
    switch (ch) {
    
    case SPU_RdEventStat:       return "SPU_RdEventStat";
    case SPU_WrEventMask:       return "SPU_WrEventMask";
    case SPU_WrEventAck:        return "SPU_WrEventAck";
    case SPU_RdSigNotify1:      return "SPU_RdSigNotify1";
    case SPU_RdSigNotify2:      return "SPU_RdSigNotify2";
    case SPU_WrDec:             return "SPU_WrDec";
    case SPU_RdDec:             return "SPU_RdDec";
    case SPU_RdEventMask:       return "SPU_RdEventMask";
    case SPU_RdMachStat:        return "SPU_RdMachStat";
    case SPU_WrSRR0:            return "SPU_WrSRR0";
    case SPU_RdSRR0:            return "SPU_RdSRR0";
    case SPU_WrOutMbox:         return "SPU_WrOutMbox";
    case SPU_RdInMbox:          return "SPU_RdInMbox";
    case SPU_WrOutIntrMbox:     return "SPU_WrOutIntrMbox";

    case MFC_WrMSSyncReq:       return "MFC_WrMSSyncReq";
    case MFC_RdTagMask:         return "MFC_RdTagMask";
    case MFC_LSA:               return "MFC_LSA";
    case MFC_EAH:               return "MFC_EAH";
    case MFC_EAL:               return "MFC_EAL";
    case MFC_Size:              return "MFC_Size";
    case MFC_TagID:             return "MFC_TagID";
    case MFC_Cmd:               return "MFC_Cmd";
    case MFC_WrTagMask:         return "MFC_WrTagMask";
    case MFC_WrTagUpdate:       return "MFC_WrTagUpdate";
    case MFC_RdTagStat:         return "MFC_RdTagStat";
    case MFC_RdListStallStat:   return "MFC_RdListStallStat";
    case MFC_WrListStallAck:    return "MFC_WrListStallAck";
    case MFC_RdAtomicStat:      return "MFC_RdAtomicStat";

    default:
        Helpers::panic("Tried to get name of bad MFC channel %d\n", ch);
    }
}

u32 SPUThread::readChannel(u32 ch) {
    log("Read %s @ 0x%08x\n", channelToString(ch).c_str(), ps3->spu->state.pc);

    switch (ch) {
    
    case SPU_RdEventStat: {
        if (!hasPendingEvents()) {
            wait();
            return 0;
        }

        return event_stat.raw & event_mask;
    }

    case MFC_RdTagStat:     return 0xffffffff & tag_mask;   // TODO
    case MFC_RdAtomicStat:  return atomic_stat;             

    default:
        Helpers::panic("Unimplemented MFC channel read 0x%02x\n", ch);
    }
}

void SPUThread::writeChannel(u32 ch, u32 val) {
    log("%s = 0x%08x\n", channelToString(ch).c_str(), val);

    switch (ch) {
     
    case SPU_WrEventMask:   event_mask      = val;      break;
    case SPU_WrEventAck:    event_stat.raw &= ~val;     break;

    case MFC_LSA:           lsa         = val;  break;
    case MFC_EAH:           eah         = val;  break;
    case MFC_EAL:           eal         = val;  break;
    case MFC_Size:          size        = val;  break;
    case MFC_TagID:         tag_id      = val;  break;
    case MFC_Cmd:           doCmd(val);         break;
    case MFC_WrTagMask:     tag_mask    = val;  break;
    case MFC_WrTagUpdate:   /* TODO */          break;

    default:
        Helpers::panic("Unimplemented MFC channel write 0x%02x\n", ch);
    }
}

void SPUThread::doCmd(u32 cmd) {
    switch (cmd) {
       
    case GET: {
        log("GET\n");
        std::memcpy(&ls[lsa & 0x3ffff], ps3->mem.getPtr(eal), size);
        break;
    }

    case PUTLLC: {
        log("PUTLLC ");
        lockline_waiter->end();

        bool success = true;
        if (eal != reservation.addr) success = false;
        else if (std::memcmp(reservation.data, ps3->mem.getPtr(eal), 128)) {    // Has the lockline data changed?
            success = false;
        }

        // Conditionally write
        if (success) {
            std::memcpy(ps3->mem.getPtr(eal), &ls[lsa & 0x3ffff], 128);
            reservation.addr = 0;
        }

        // Update atomic stat
        atomic_stat = 0;
        atomic_stat |= !success;    // Bit 0 is set if the reservation was lost 

        if (success) logNoPrefix("Success\n");
        else logNoPrefix("Failure\n");
        break;
    }

    case GETLLAR: {
        log("GETLLAR 0x%08x\n", ps3->spu->state.pc);

        // Get reservation data
        reservation.addr = eal;
        std::memcpy(reservation.data, ps3->mem.getPtr(eal), 128);
        
        // Copy it to local storage
        std::memcpy(&ls[lsa & 0x3ffff], ps3->mem.getPtr(eal), 128);

        /*for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 4; j++)
                printf("0x%08x ", Helpers::bswap<u32>(*(u32*)&ls[lsa + (i * 0x10) + (j * 4)]));
            printf("\n");
        }*/

        // Update atomic stat
        atomic_stat = 0;            // It gets overwritten on every command
        atomic_stat |= (1 << 2);    // getllar command completed
        
        // Setup lockline waiter
        lockline_waiter->begin(reservation);
        break;
    }

    default:
        Helpers::panic("Unimplemented MFC command 0x%02x\n", cmd);
    }
}