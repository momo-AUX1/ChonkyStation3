#include <Syscall.hpp>
#include "PlayStation3.hpp"


MAKE_LOG_FUNCTION(log_sys_fs, sys_fs);

u64 Syscall::sys_fs_test() {
    log_sys_fs("sys_fs_test(...)\n");

    return Result::CELL_OK;
}

u64 Syscall::sys_fs_fcntl() {
    const u32 fd = ARG0;
    const u32 op = ARG1;
    const u32 arg_ptr = ARG2;
    const u32 size = ARG3;
    log_sys_fs("sys_fs_fcntl(fd: %d, op: 0x%08x, arg_ptr: 0x%08x, size: %d) UNIMPLEMENTED\n", fd, op, arg_ptr, size);

    return Result::CELL_OK;
}