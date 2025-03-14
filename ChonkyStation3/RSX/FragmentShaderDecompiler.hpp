#pragma once

#include <common.hpp>
#include <logger.hpp>
#include <BitField.hpp>

#include <string>
#include <unordered_map>
#include <format>
#include <algorithm>

#include <FragmentShader.hpp>
#include <FragmentInstruction.hpp>
#include <RSXShaderOpcodes.hpp>

#ifdef __XBOX_BUILD
#ifdef CONST
#undef CONST
#endif
#endif

// Circular dependency
class PlayStation3;

class FragmentShaderDecompiler {
public:
    FragmentShaderDecompiler(PlayStation3* ps3) : ps3(ps3) {}
    PlayStation3* ps3;

    MAKE_LOG_FUNCTION(log, fragment_shader);

    std::string decompile(FragmentShader& shader_program);
    FragmentInstruction fetchInstr(u32 addr);
    u32 fetch32(u32 addr);
    static u32 swap(u32 v) { return (v >> 16) | (v << 16); }

    std::string addConstant(float x, float y, float z, float w);
    std::string addUniform(u32 addr);
    bool isUniform(std::string name);
    void declareFunction(std::string name, std::string code, std::string& shader);
    void markInputAsUsed(std::string name, int location);
    void markRegAsUsed(std::string name, int location);
    void enableInput(u32 idx);

    std::string source(FragmentInstruction& instr, int s);
    std::string dest(FragmentInstruction& instr);
    std::string mask(FragmentInstruction& instr, int& num_lanes);
    std::string getType(const int num_lanes);
    std::string getCond(FragmentInstruction& instr);
    bool hasCond(FragmentInstruction& instr);

    u32 curr_offs = 0;
    int next_constant = 0;
    std::string constants;
    std::string uniforms;
    std::vector<std::string> uniform_names;
    bool used_inputs[16];
    bool used_regs[256];    // TODO: how many are there????
    bool is_input[22];
    std::string inputs;
    std::string regs;
    std::string initialization;

    enum FRAGMENT_SOURCE_TYPE {
        TEMP = 0,
        INPUT = 1,
        CONST = 2
    };

    const std::string input_names[15] = {
        "fs_wpos",
        "fs_col0",
        "fs_col1",
        "fs_fog",
        "fs_tex0",
        "fs_tex1",
        "fs_tex2",
        "fs_tex3",
        "fs_tex4",
        "fs_tex5",
        "fs_tex6",
        "fs_tex7",
        "fs_tex8",
        "fs_tex9",
        "fs_ssa"
    };

    std::unordered_map<u8, std::string> fragment_opcodes {
        { RSXFragment::NOP,          "NOP" },
        { RSXFragment::MOV,          "MOV" },
        { RSXFragment::MUL,          "MUL" },
        { RSXFragment::ADD,          "ADD" },
        { RSXFragment::MAD,          "MAD" },
        { RSXFragment::DP3,          "DP3" },
        { RSXFragment::DP4,          "DP4" },
        { RSXFragment::DST,          "DST" },
        { RSXFragment::MIN,          "MIN" },
        { RSXFragment::MAX,          "MAX" },
        { RSXFragment::SLT,          "SLT" },
        { RSXFragment::SGE,          "SGE" },
        { RSXFragment::SLE,          "SLE" },
        { RSXFragment::SGT,          "SGT" },
        { RSXFragment::SNE,          "SNE" },
        { RSXFragment::SEQ,          "SEQ" },
        { RSXFragment::FRC,          "FRC" },
        { RSXFragment::FLR,          "FLR" },
        { RSXFragment::KIL,          "KIL" },
        { RSXFragment::PK4,          "PK4" },
        { RSXFragment::UP4,          "UP4" },
        { RSXFragment::DDX,          "DDX" },
        { RSXFragment::DDY,          "DDY" },
        { RSXFragment::TEX,          "TEX" },
        { RSXFragment::TXP,          "TXP" },
        { RSXFragment::TXD,          "TXD" },
        { RSXFragment::RCP,          "RCP" },
        { RSXFragment::RSQ,          "RSQ" },
        { RSXFragment::EX2,          "EX2" },
        { RSXFragment::LG2,          "LG2" },
        { RSXFragment::LIT,          "LIT" },
        { RSXFragment::LRP,          "LRP" },
        { RSXFragment::STR,          "STR" },
        { RSXFragment::SFL,          "SFL" },
        { RSXFragment::COS,          "COS" },
        { RSXFragment::SIN,          "SIN" },
        { RSXFragment::PK2,          "PK2" },
        { RSXFragment::UP2,          "UP2" },
        { RSXFragment::POW,          "POW" },
        { RSXFragment::PKB,          "PKB" },
        { RSXFragment::UPB,          "UPB" },
        { RSXFragment::PK16,         "PK16" },
        { RSXFragment::UP16,         "UP16" },
        { RSXFragment::BEM,          "BEM" },
        { RSXFragment::PKG,          "PKG" },
        { RSXFragment::UPG,          "UPG" },
        { RSXFragment::DP2A,         "DP2A" },
        { RSXFragment::TXL,          "TXL" },
        { RSXFragment::TXB,          "TXB" },
        { RSXFragment::TEXBEM,       "TEXBEM" },
        { RSXFragment::TXPBEM,       "TXPBEM" },
        { RSXFragment::BEMLUM,       "BEMLUM" },
        { RSXFragment::REFL,         "REFL" },
        { RSXFragment::TIMESWTEX,    "TIMESWTEX" },
        { RSXFragment::DP2,          "DP2" },
        { RSXFragment::NRM,          "NRM" },
        { RSXFragment::DIV,          "DIV" },
        { RSXFragment::DIVSQ,        "DIVSQ" },
        { RSXFragment::LIF,          "LIF" },
        { RSXFragment::FENCT,        "FENCT" },
        { RSXFragment::FENCB,        "FENCB" },
        { RSXFragment::BRK,          "BRK" },
        { RSXFragment::CAL,          "CAL" },
        { RSXFragment::IFE,          "IFE" },
        { RSXFragment::LOOP,         "LOOP" },
        { RSXFragment::REP,          "REP" },
        { RSXFragment::RET,          "RET" },
    };
};