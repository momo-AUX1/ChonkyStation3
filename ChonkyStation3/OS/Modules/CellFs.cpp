#include "CellFs.hpp"
#include "PlayStation3.hpp"


u64 CellFs::cellFsClose() {
    const u32 file_id = ARG0;
    log("cellFsClose(file_id: %d)\n", file_id);

    if ((s32)file_id <= 0) return Result::CELL_BADF;
    ps3->fs.close(file_id);
    return Result::CELL_OK;
}

u64 CellFs::cellFsOpendir() {
    const u32 path_ptr = ARG0;
    const u32 file_id_ptr = ARG1;
    const std::string path = Helpers::readString(ps3->mem.getPtr(path_ptr));
    log("cellFsOpendir(path_ptr: 0x%08x, file_id_ptr: 0x%08x) [path: %s]\n", path_ptr, file_id_ptr, path.c_str());

    if (path == ".") return CELL_ENOENT;    // TODO: why?
    if (path == "") return CELL_ENOENT; // TLOU does this, it's meant to happen
    if (path == "/http-cache/") return CELL_ENOTMOUNTED;

    const u32 file_id = ps3->fs.opendir(path);
    ps3->mem.write<u32>(file_id_ptr, file_id);

    if (file_id == 0) {
        return Result::CELL_ENOENT;
    }
    return Result::CELL_OK;
}

u64 CellFs::cellFsRead() {
    const u32 file_id = ARG0;
    const u32 buf = ARG1;
    const u64 size = ARG2;
    const u32 bytes_read_ptr = ARG3;    // bytes_read is u64
    log("cellFsRead(file_id: %d, buf: 0x%08x, size: %lld, bytes_read_ptr: 0x%08x)\n", file_id, buf, size, bytes_read_ptr);

    const u64 bytes_read = ps3->fs.read(file_id, ps3->mem.getPtr(buf), size);
    if (bytes_read_ptr) // Note: this behavior is different from sys_fs_read, which returns CELL_EFAULT in case bytes_read_ptr is null.
        ps3->mem.write<u64>(bytes_read_ptr, bytes_read);

    return Result::CELL_OK;
}

u64 CellFs::cellFsReaddir() {
    const u32 file_id = ARG0;
    const u32 dirent_ptr = ARG1;
    const u32 bytes_read_ptr = ARG2;    // bytes_read is u64
    log("cellFsReaddir(file_id: %d, dirent_ptr: 0x%08x, bytes_read_ptr: 0x%08x)\n", file_id, dirent_ptr, bytes_read_ptr);

    CellFsDirent* dirent = (CellFsDirent*)ps3->mem.getPtr(dirent_ptr);
    //const std::string filename = ps3->fs.getFileFromID(file_id).path.filename().generic_string();

    //dirent->type = ps3->fs.isDirectory(file_id) ? CELL_FS_TYPE_DIRECTORY : CELL_FS_TYPE_REGULAR;
    //std::memset(dirent->name, 0, 256);
    //std::memcpy(dirent->name, filename.c_str(), filename.length());
    //dirent->namelen = filename.length();

    // TODO: stub
    dirent->type = 0;
    dirent->namelen = 0;
    dirent->name[0] = '\0';
    ps3->mem.write<u64>(bytes_read_ptr, 0);
    //ps3->mem.write<u64>(bytes_read_ptr, sizeof(CellFsDirent));
    return Result::CELL_OK;
}

u64 CellFs::cellFsOpen() {
    const u32 path_ptr = ARG0;
    const s32 flags = ARG1;
    const u32 file_id_ptr = ARG2;
    const u32 arg_ptr = ARG3;
    const u64 size = ARG4;
    const std::string path = Helpers::readString(ps3->mem.getPtr(path_ptr));
    log("cellFsOpen(path_ptr: 0x%08x, flags: %d, file_id_ptr: 0x%08x, arg_ptr: 0x%08x, size: %d) [path: %s]\n", path_ptr, flags, file_id_ptr, arg_ptr, size, path.c_str());

    if (!ps3->fs.isValidDevice(path))   return CELL_ENOTMOUNTED;
    if (!ps3->fs.isDeviceMounted(path)) return CELL_ENOTMOUNTED;

    const u32 file_id = ps3->fs.open(path);
    if (file_id == 0) {
        return Result::CELL_ENOENT;
    }

    ps3->mem.write<u32>(file_id_ptr, file_id);
    return Result::CELL_OK;
}

u64 CellFs::cellFsStat() {
    const u32 path_ptr = ARG0;
    const u32 stat_ptr = ARG1;
    log("cellFsStat(path_ptr: 0x%08x, stat_ptr: 0x%08x)", path_ptr, stat_ptr);
    const std::string path = Helpers::readString(ps3->mem.getPtr(path_ptr));
    logNoPrefix(" [path: %s]\n", path.c_str());

    const auto device = ps3->fs.getDeviceFromPath(path);
    if (!ps3->fs.isDeviceMounted(device)) {
        log("Warning: device not mounted\n");
        return CELL_ENOTMOUNTED;
    }

    if (!ps3->fs.exists(path))
        return CELL_ENOENT;

    CellFsStat* stat = (CellFsStat*)ps3->mem.getPtr(stat_ptr);
    bool is_dir = ps3->fs.isDirectory(path);
    stat->mode = !is_dir ? (CELL_FS_S::CELL_FS_S_IFREG | 0666) : (CELL_FS_S::CELL_FS_S_IFDIR | 0777);
    stat->uid = 0;
    stat->gid = 0;
    stat->atime = 0;
    stat->mtime = 0;
    stat->ctime = 0;
    stat->size = !is_dir ? ps3->fs.getFileSize(path) : 4096;
    stat->blksize = 4096;

    return Result::CELL_OK;
}

u64 CellFs::cellFsLseek() {
    const u32 file_id = ARG0;
    const s64 offs = ARG1;
    const u32 seek_mode = ARG2;
    const u32 pos_ptr = ARG3;   // pos is u64
    log("cellFSLseek(file_id: %d, offs: %d, seek_mode: %d, pos_ptr: 0x%08x)\n", file_id, offs, seek_mode, pos_ptr);
    
    const u64 pos = ps3->fs.seek(file_id, offs, seek_mode);
    ps3->mem.write<u64>(pos_ptr, pos);

    return Result::CELL_OK;
}

u64 CellFs::cellFsGetFreeSize() {
    const u32 path_ptr = ARG0;
    const u32 block_size_ptr = ARG1;
    const u64 block_cnt_ptr = ARG2;
    const std::string path = Helpers::readString(ps3->mem.getPtr(path_ptr));
    log("cellFsGetFreeSize(path_ptr: 0x%08x, block_size_ptr: 0x%08x, block_cnt_ptr: 0x%08x) [path: %s]\n", path_ptr, block_size_ptr, block_cnt_ptr, path.c_str());

    // TODO
    return Result::CELL_OK;
}

u64 CellFs::cellFsSdataOpen() {
    const u32 path_ptr = ARG0;
    const s32 flags = ARG1;
    const u32 file_id_ptr = ARG2;
    const u32 arg_ptr = ARG3;
    const u64 size = ARG4;
    const std::string path = Helpers::readString(ps3->mem.getPtr(path_ptr));
    log("cellFsSdataOpen(path_ptr: 0x%08x, flags: %d, file_id_ptr: 0x%08x, arg_ptr: 0x%08x, size: %d) [path: %s]\n", path_ptr, flags, file_id_ptr, arg_ptr, size, path.c_str());

    const u32 file_id = ps3->fs.open(path);
    ps3->mem.write<u32>(file_id_ptr, file_id);

    if (file_id == 0) {
        Helpers::panic("cellFsSdataOpen: could not find file %s\n", path.c_str());
    }
    return Result::CELL_OK;
}

u64 CellFs::cellFsMkdir() {
    const u32 path_ptr = ARG0;
    const s32 mode = ARG1;
    const std::string path = Helpers::readString(ps3->mem.getPtr(path_ptr));
    log("cellFsMkdir(path_ptr: 0x%08x, mode: %d) [path: %s]\n", path_ptr, mode, path.c_str());

    if (path == "") return CELL_ENOENT; // TLOU does this, it's meant to happen
    if (ps3->fs.mkdir(path)) return CELL_EEXIST;

    return Result::CELL_OK;
}

u64 CellFs::cellFsFstat() {
    const u32 file_id = ARG0;
    const u32 stat_ptr = ARG1;
    log("cellFsFstat(file_id: %d, stat_ptr: 0x%08x)\n", file_id, stat_ptr);

    CellFsStat* stat = (CellFsStat*)ps3->mem.getPtr(stat_ptr);
    bool is_dir = ps3->fs.isDirectory(file_id);
    stat->mode = !is_dir ? (CELL_FS_S::CELL_FS_S_IFREG | 0666) : (CELL_FS_S::CELL_FS_S_IFDIR | 0777);
    stat->uid = 0;
    stat->gid = 0;
    stat->atime = 0;
    stat->mtime = 0;
    stat->ctime = 0;
    stat->size = !is_dir ? ps3->fs.getFileSize(file_id) : 4096;
    stat->blksize = 4096;

    return Result::CELL_OK;
}

u64 CellFs::cellFsClosedir() {
    const u32 file_id = ARG0;
    log("cellFsClosedir(file_id: %d)\n", file_id);

    if ((s32)file_id <= 0) return Result::CELL_BADF;
    ps3->fs.closedir(file_id);
    return Result::CELL_OK;
}