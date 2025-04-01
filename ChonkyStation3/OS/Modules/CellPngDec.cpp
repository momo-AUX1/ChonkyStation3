#include "CellPngDec.hpp"
#include "PlayStation3.hpp"


u64 CellPngDec::cellPngDecDecodeData() {
    const u32 handle = ARG0;
    const u32 subhandle = ARG1;
    const u32 data_ptr = ARG2;
    const u32 data_ctrl_param_ptr = ARG3;
    const u32 data_out_info_ptr = ARG4;
    log("cellPngDecDecodeData()\n");

    CellPngDecDataOutInfo* out_info = (CellPngDecDataOutInfo*)ps3->mem.getPtr(data_out_info_ptr);
    CellPngDecDataCtrlParam* ctrl = (CellPngDecDataCtrlParam*)ps3->mem.getPtr(data_ctrl_param_ptr);

    std::vector<u8> buf;
    std::vector<u8> img;
    buf.clear();
    img.clear();
    lodepng::load_file(buf, curr_file.generic_string().c_str());
    lodepng::State state;
    
    u32 n_components = 0;
    state.decoder.color_convert = 1;
    if (out_param.output_color_space == CELL_PNGDEC_RGB) {
        state.info_raw.colortype = LCT_RGB;
        n_components = 3;
    }
    else {
        state.info_png.color.colortype = LCT_RGBA;
        n_components = 4;
    }
    state.info_raw.bitdepth = out_param.output_bit_depth;

    u32 width;
    u32 height;
    u32 err;
    if (err = lodepng::decode(img, width, height, state, buf))
        Helpers::panic("Failed to decode png %s: %s\n", curr_file.generic_string().c_str(), lodepng_error_text(err));

    // Swizzle data
    if (out_param.output_color_space == CELL_PNGDEC_ARGB) {
        // Convert RGBA to ARGB
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                const u32 pixel = *(u32*)&img[y * width + (x * 4)];
                const u8 r = pixel >> 24;
                const u8 g = (pixel >> 16) & 0xff;
                const u8 b = (pixel >> 8) & 0xff;
                const u8 a = pixel & 0xff;
                *(u32*)&img[y * width + (x * 4)] = (a << 24) | (r << 16) | (g << 8) | b;
            }
        }
    }

    // Do we need padding?
    const u64 actual_width = width * n_components;
    if (ctrl->output_bytes_per_line > actual_width) {
        // buf isn't needed anymore, we can reuse it
        buf.clear();
        buf.resize(ctrl->output_bytes_per_line * height);
        for (int i = 0; i < height; i++) {
            std::memcpy(&buf[i * ctrl->output_bytes_per_line], &img[i * actual_width], actual_width);
        }
        std::memcpy(ps3->mem.getPtr(data_ptr), buf.data(), buf.size());
    }
    else {
        std::memcpy(ps3->mem.getPtr(data_ptr), img.data(), img.size());
    }

    out_info->status = 0;   // FINISHED

    return Result::CELL_OK;
}

u64 CellPngDec::cellPngDecClose() {
    log("cellPngDecClose()\n");

    // Delete temporary files
    if (curr_src.src_sel == CellPngDecStreamSrcSel::CELL_PNGDEC_BUFFER)
        fs::remove(curr_file);
    return Result::CELL_OK;
}

u64 CellPngDec::cellPngDecDestroy() {
    log("cellPngDecDestroy()\n");
    return Result::CELL_OK;
}

u64 CellPngDec::cellPngDecReadHeader() {
    const u32 handle = ARG0;
    const u32 subhandle = ARG1;
    const u32 info_ptr = ARG2;
    log("cellPngDecReadHeader(handle: 0x%08x, subhandle: 0x%08x, info_ptr: 0x%08x)\n", handle, subhandle, info_ptr);

    CellPngDecInfo* info = (CellPngDecInfo*)ps3->mem.getPtr(info_ptr);

    std::vector<u8> buf;
    std::vector<u8> img;
    buf.clear();
    img.clear();
    lodepng::load_file(buf, curr_file.generic_string().c_str());
    lodepng::State state;
    u32 width;
    u32 height;
    u32 err;
    if (err = lodepng::decode(img, width, height, state, buf))
        Helpers::panic("Failed to decode png %s: %s\n", curr_file.generic_string().c_str(), lodepng_error_text(err));
    //displayPNGInfo(state.info_png);

    info->width = width;
    info->height = height;
    info->n_components = lodepng_get_channels(&state.info_png.color);
    switch (info->n_components) {
    case 3: info->color_space = CELL_PNGDEC_RGB;    break;
    case 4: info->color_space = CELL_PNGDEC_RGBA;   break;
    default:    Helpers::panic("pngdec: %d components\n", info->n_components);
    }
    info->bit_depth = state.info_png.color.bitdepth;
    info->interlace_method = 0; // no interlace
    info->chunk_info = 0;

    curr_info.width = info->width;
    curr_info.height = info->height;
    curr_info.n_components = info->n_components;
    curr_info.bit_depth = info->bit_depth;

    return Result::CELL_OK;
}

u64 CellPngDec::cellPngDecCreate() {
    const u32 handle_ptr = ARG0;
    const u32 thread_in_param_ptr = ARG1;
    const u32 thread_out_param_ptr = ARG2;

    ps3->mem.write<u32>(handle_ptr, ps3->handle_manager.request());
    CellPngDecThreadOutParam* out = (CellPngDecThreadOutParam*)ps3->mem.getPtr(thread_out_param_ptr);
    out->codec_version = PNG_CODEC_VERSION;

    log("cellPngDecCreate(handle_ptr: 0x%08x, thread_in_param_ptr: 0x%08x, thread_out_param_ptr: 0x%08x)\n", handle_ptr, thread_in_param_ptr, thread_out_param_ptr);
    return Result::CELL_OK;
}

u64 CellPngDec::cellPngDecOpen() {
    const u32 handle = ARG0;
    const u32 subhandle_ptr = ARG1;
    const u32 src_ptr = ARG2;
    const u32 open_info_ptr = ARG3;
    log("cellPngDecOpen(handle: 0x%08x, subhandle_ptr: 0x%08x, src_ptr: 0x%08x, open_info_ptr: 0x%08x)\n", handle, subhandle_ptr, src_ptr, open_info_ptr);

    CellPngDecSrc* src = (CellPngDecSrc*)ps3->mem.getPtr(src_ptr);

    curr_src.src_sel = src->src_sel;
    log("Source type: %s\n", curr_src.src_sel == CellPngDecStreamSrcSel::CELL_PNGDEC_BUFFER ? "BUFFER" : "FILE");
    if (src->src_sel == CellPngDecStreamSrcSel::CELL_PNGDEC_FILE) {
        curr_src.filename = ps3->mem.getPtr(src->filename_ptr);
        curr_src.file_offs = src->file_offs;
        curr_src.file_size = src->file_size;
    }
    else if (src->src_sel == CellPngDecStreamSrcSel::CELL_PNGDEC_BUFFER) {
        curr_src.stream = ps3->mem.getPtr(src->stream_ptr);
        curr_src.stream_size = src->stream_size;
    }
    curr_src.spu_thread_enable = src->spu_thread_enable;

    // Load buffer
    if (src->src_sel == CellPngDecStreamSrcSel::CELL_PNGDEC_BUFFER) {
        // Dump buffer
        std::string filename = std::format("{:08x}.png", (u32)src->stream_ptr);
        curr_file = filename;
        std::ofstream file(filename, std::ios::binary);
        file.write((const char*)curr_src.stream, curr_src.stream_size);
    }
    else {
        curr_file = ps3->fs.guestPathToHost((const char*)curr_src.filename);
        log("file: %s\n", curr_src.filename);
        log("file_offs: %d\n", curr_src.file_offs);
        log("file_size: %d\n", curr_src.file_size);
    }

    return Result::CELL_OK;
}

u64 CellPngDec::cellPngDecSetParameter() {
    const u32 handle = ARG0;
    const u32 subhandle = ARG1;
    const u32 in_param_ptr = ARG2;
    const u32 out_param_ptr = ARG3;
    log("cellPngDecSetParameter(handle: 0x%08x, subhandle: 0x%08x, in_param_ptr: 0x%08x, out_param_ptr: 0x%08x)\n", handle, subhandle, in_param_ptr, out_param_ptr);

    CellPngDecInParam* in = (CellPngDecInParam*)ps3->mem.getPtr(in_param_ptr);
    CellPngDecOutParam* out = (CellPngDecOutParam*)ps3->mem.getPtr(out_param_ptr);

    out->output_width_byte = (curr_info.width * curr_info.n_components * curr_info.bit_depth) / 8;  // Number of bytes in a line
    out->output_width = curr_info.width;
    out->output_height = curr_info.height;
    out->output_components = curr_info.n_components;
    out->output_bit_depth = in->output_bit_depth;
    out->output_mode = in->output_mode;
    out->output_color_space = in->output_color_space;
    out->use_mem_space = 0; // ??

    out_param.output_width_byte = out->output_width_byte;
    out_param.output_width = out->output_width;
    out_param.output_height = out->output_height;
    out_param.output_components = out->output_components;
    out_param.output_bit_depth = out->output_bit_depth;
    out_param.output_mode = out->output_mode;
    out_param.output_color_space = out->output_color_space;
    out_param.use_mem_space = out->use_mem_space;

    return Result::CELL_OK;
}

void CellPngDec::displayPNGInfo(const LodePNGInfo& info) {
    const LodePNGColorMode& color = info.color;

    std::cout << "Compression method: " << info.compression_method << std::endl;
    std::cout << "Filter method: " << info.filter_method << std::endl;
    std::cout << "Interlace method: " << info.interlace_method << std::endl;
    std::cout << "Color type: " << color.colortype << std::endl;
    std::cout << "Bit depth: " << color.bitdepth << std::endl;
    std::cout << "Bits per pixel: " << lodepng_get_bpp(&color) << std::endl;
    std::cout << "Channels per pixel: " << lodepng_get_channels(&color) << std::endl;
    std::cout << "Is greyscale type: " << lodepng_is_greyscale_type(&color) << std::endl;
    std::cout << "Can have alpha: " << lodepng_can_have_alpha(&color) << std::endl;
    std::cout << "Palette size: " << color.palettesize << std::endl;
    std::cout << "Has color key: " << color.key_defined << std::endl;
    if (color.key_defined) {
        std::cout << "Color key r: " << color.key_r << std::endl;
        std::cout << "Color key g: " << color.key_g << std::endl;
        std::cout << "Color key b: " << color.key_b << std::endl;
    }
    std::cout << "Texts: " << info.text_num << std::endl;
    for (size_t i = 0; i < info.text_num; i++) {
        std::cout << "Text: " << info.text_keys[i] << ": " << info.text_strings[i] << std::endl << std::endl;
    }
    std::cout << "International texts: " << info.itext_num << std::endl;
    for (size_t i = 0; i < info.itext_num; i++) {
        std::cout << "Text: "
            << info.itext_keys[i] << ", "
            << info.itext_langtags[i] << ", "
            << info.itext_transkeys[i] << ": "
            << info.itext_strings[i] << std::endl << std::endl;
    }
    std::cout << "Time defined: " << info.time_defined << std::endl;
    if (info.time_defined) {
        const LodePNGTime& time = info.time;
        std::cout << "year: " << time.year << std::endl;
        std::cout << "month: " << time.month << std::endl;
        std::cout << "day: " << time.day << std::endl;
        std::cout << "hour: " << time.hour << std::endl;
        std::cout << "minute: " << time.minute << std::endl;
        std::cout << "second: " << time.second << std::endl;
    }
    std::cout << "Physics defined: " << info.phys_defined << std::endl;
    if (info.phys_defined) {
        std::cout << "physics X: " << info.phys_x << std::endl;
        std::cout << "physics Y: " << info.phys_y << std::endl;
        std::cout << "physics unit: " << info.phys_unit << std::endl;
    }
}
