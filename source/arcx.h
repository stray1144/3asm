#include <stdint.h>

#define MAX_OPERANDS 4
typedef enum operand_kind_e : uint8_t {
        OPERAND_NONE = 0,
        OPERAND_IMMEDIATE = 1 << 0,
        OPERAND_REGISTER = 1 << 1,
        OPERAND_VECTOR = 1 << 2,
        OPERAND_REFERENCE = 1 << 3,

        OPERAND_OFFSET_FLAG = 1 << 7,

        OPERAND_OFFSETTED_REGISTER = OPERAND_REGISTER | OPERAND_OFFSET_FLAG,
        OPERAND_OFFSETTED_REFERENCE = OPERAND_REFERENCE | OPERAND_OFFSET_FLAG,

        OPERAND_VALUE = OPERAND_IMMEDIATE |  OPERAND_OFFSETTED_REGISTER,
        OPERAND_WHATEVER = OPERAND_VALUE | OPERAND_OFFSETTED_REFERENCE
} operand_kind_t;

#define MAX_FLAGS 8
typedef struct flag_descriptor_s {
        char symbol;
        uint8_t encoding;
} flag_descriptor_t;

typedef struct instruction_descriptor_s {
        const char mnemonic[10];
        uint8_t operation_code;

        uint8_t flag_count;
        flag_descriptor_t flags[MAX_FLAGS];

        uint8_t operand_count;
        operand_kind_t operands[MAX_OPERANDS];
} instruction_descriptor_t;

#define INSTRUCTION_COUNT 38
static const instruction_descriptor_t instruction_lut[INSTRUCTION_COUNT] = {

        // .1 Basic Operations

        {"none"     , 0x00, 0, {}, 0, {}},
        {"move"     , 0x01, 0, {}, 2, {OPERAND_WHATEVER, OPERAND_REGISTER}},
        {"mc"       , 0x02, 3, {{'z', 1 << 0}, {'p', 1 << 1}, {'n', 1 << 2}}, 3, {OPERAND_VALUE, OPERAND_WHATEVER, OPERAND_REGISTER}},
        {"jump"     , 0x03, 1, {{'r', 1 << 0}}, 1, {OPERAND_WHATEVER}},
        {"jc"       , 0x04, 4, {{'r', 1 << 0}, {'z', 1 << 1}, {'p', 1 << 2}, {'n', 1 << 3}}, 2, {OPERAND_VALUE, OPERAND_WHATEVER}},
        {"load"     , 0x05, 0, {}, 2, {OPERAND_WHATEVER, OPERAND_REGISTER}},
        {"store"    , 0x06, 0, {}, 2, {OPERAND_WHATEVER, OPERAND_WHATEVER}},

        // .2 Arithmetic Operations

        {"add"      , 0x07, 2, {{'u', 1 << 0}, {'f', 1 << 1}}, 2, {OPERAND_WHATEVER, OPERAND_WHATEVER}},
        {"subtract" , 0x08, 2, {{'u', 1 << 0}, {'f', 1 << 1}}, 2, {OPERAND_WHATEVER, OPERAND_WHATEVER}},
        {"multiply" , 0x09, 2, {{'u', 1 << 0}, {'f', 1 << 1}}, 2, {OPERAND_WHATEVER, OPERAND_WHATEVER}},
        {"divide"   , 0x0A, 2, {{'u', 1 << 0}, {'f', 1 << 1}, {'r', 1 << 2}}, 2, {OPERAND_WHATEVER, OPERAND_WHATEVER}},
        {"lesser"   , 0x0B, 2, {{'u', 1 << 0}, {'f', 1 << 1}}, 2, {OPERAND_WHATEVER, OPERAND_WHATEVER}},
        {"equal"    , 0x0C, 2, {{'u', 1 << 0}, {'f', 1 << 1}}, 2, {OPERAND_WHATEVER, OPERAND_WHATEVER}},
        {"greater"  , 0x0D, 2, {{'u', 1 << 0}, {'f', 1 << 1}}, 2, {OPERAND_WHATEVER, OPERAND_WHATEVER}},

        // .3 Bit-wise Operations

        {"not"      , 0x0E, 1, {{'l', 1 << 0}}, 1, {OPERAND_WHATEVER}},
        {"and"      , 0x0F, 1, {{'l', 1 << 0}}, 2, {OPERAND_WHATEVER, OPERAND_WHATEVER}},
        {"or"       , 0x10, 1, {{'l', 1 << 0}}, 2, {OPERAND_WHATEVER, OPERAND_WHATEVER}},
        {"xor"      , 0x11, 1, {{'l', 1 << 0}}, 2, {OPERAND_WHATEVER, OPERAND_WHATEVER}},
        {"lshift"   , 0x12, 1, {{'r', 1 << 0}}, 2, {OPERAND_WHATEVER, OPERAND_WHATEVER}},
        {"rshift"   , 0x13, 1, {{'r', 1 << 0}}, 2, {OPERAND_WHATEVER, OPERAND_WHATEVER}},

        // TODO: add the new instructions from the new unredacted spec update... (and BuRST)
        
        // .5 Bit Operations
        {"count"    , 0x27, 2, {{'t', 1 << 0}, {'p', 1 << 1}}, 1, {OPERAND_WHATEVER}},
        {"bswap"    , 0x28, 0, {}, 1, {OPERAND_WHATEVER}},
        {"bextract" , 0x29, 0, {}, 3, {OPERAND_WHATEVER, OPERAND_VALUE, OPERAND_VALUE}},
        {"binsert"  , 0x2A, 0, {}, 4, {OPERAND_WHATEVER, OPERAND_REGISTER, OPERAND_VALUE, OPERAND_VALUE}},
        {"sextend"  , 0x2B, 0, {}, 1, {OPERAND_WHATEVER}},

        // .6 Control Operations

        {"call"     , 0xF3, 0, {}, 1, {OPERAND_WHATEVER}},
        {"return"   , 0xF4, 0, {}, 0, {}},
        {"ka"       , 0xF5, 0, {}, 1, {OPERAND_VALUE}},
        {"control"  , 0xF6, 2, {{'p', 1 << 0}, {'i', 1 << 1}}, 1, {OPERAND_VALUE}},
        {"rfa"      , 0xF7, 0, {}, 0, {}},
        {"svas"     , 0xF8, 0, {}, 1, {OPERAND_WHATEVER}},
        {"fca"      , 0xF9, 1, {{'g', 1 << 0}}, 0, {}},
        {"fce"      , 0xFA, 0, {}, 1, {OPERAND_WHATEVER}},
        {"sihan"    , 0xFB, 0, {}, 2, {OPERAND_VALUE, OPERAND_WHATEVER}},
        {"interrupt", 0xFC, 0, {}, 1, {OPERAND_VALUE}},
        {"iredirect", 0xFD, 0, {}, 1, {OPERAND_WHATEVER}},
        {"ireturn"  , 0xFE, 0, {}, 0, {}},
        {"halt"     , 0xFF, 2, {{'p', 1 << 0}, {'r', 1 << 1}}, 0, {}}
};

typedef struct register_descriptor_s {
        char mnemonic[4];
        uint8_t encoding;
        operand_kind_t kind;
} register_descriptor_t;

#define REGISTER_COUNT 66
static register_descriptor_t register_lut[REGISTER_COUNT] = {
	{"r1" , 0x01, OPERAND_REGISTER},
	{"r2" , 0x02, OPERAND_REGISTER},
	{"r3" , 0x03, OPERAND_REGISTER},
	{"r4" , 0x04, OPERAND_REGISTER},
	{"r5" , 0x05, OPERAND_REGISTER},
	{"r6" , 0x06, OPERAND_REGISTER},
	{"r7" , 0x07, OPERAND_REGISTER},
	{"r8" , 0x08, OPERAND_REGISTER},
	{"r9" , 0x09, OPERAND_REGISTER},
	{"r10", 0x0A, OPERAND_REGISTER},
	{"r11", 0x0B, OPERAND_REGISTER},
	{"r12", 0x0C, OPERAND_REGISTER},
	{"r13", 0x0D, OPERAND_REGISTER},
	{"r14", 0x0E, OPERAND_REGISTER},
	{"r15", 0x0F, OPERAND_REGISTER},
	{"r16", 0x10, OPERAND_REGISTER},
	{"r17", 0x11, OPERAND_REGISTER},
	{"r18", 0x12, OPERAND_REGISTER},
	{"r19", 0x13, OPERAND_REGISTER},
	{"r20", 0x14, OPERAND_REGISTER},
	{"r21", 0x15, OPERAND_REGISTER},
	{"r22", 0x16, OPERAND_REGISTER},
	{"r23", 0x17, OPERAND_REGISTER},
	{"r24", 0x18, OPERAND_REGISTER},
	{"r25", 0x19, OPERAND_REGISTER},
	{"r26", 0x1A, OPERAND_REGISTER},
	{"r27", 0x1B, OPERAND_REGISTER},
	{"r28", 0x1C, OPERAND_REGISTER},
	{"r29", 0x1D, OPERAND_REGISTER},
	{"r30", 0x1E, OPERAND_REGISTER},
	{"r31", 0x1F, OPERAND_REGISTER},
	{"r32", 0x20, OPERAND_REGISTER},
	
	{"ra" , 0x21, OPERAND_REGISTER},
	{"rip", 0x22, OPERAND_REGISTER},
	{"rr" , 0x23, OPERAND_REGISTER},
	{"rsp", 0x24, OPERAND_REGISTER},
	{"rfp", 0x25, OPERAND_REGISTER},
	{"rcs", 0x26, OPERAND_REGISTER},
	{"rvc", 0x27, OPERAND_REGISTER},
	{"rdj", 0x28, OPERAND_REGISTER},
	{"rwm", 0x29, OPERAND_REGISTER},

	

 	{"v1" , 0x01, OPERAND_VECTOR},
	{"v2" , 0x02, OPERAND_VECTOR},
	{"v3" , 0x03, OPERAND_VECTOR},
	{"v4" , 0x04, OPERAND_VECTOR},
	{"v5" , 0x05, OPERAND_VECTOR},
	{"v6" , 0x06, OPERAND_VECTOR},
	{"v7" , 0x07, OPERAND_VECTOR},
	{"v8" , 0x08, OPERAND_VECTOR},
	{"v9" , 0x09, OPERAND_VECTOR},
	{"v10", 0x0A, OPERAND_VECTOR},
	{"v11", 0x0B, OPERAND_VECTOR},
	{"v12", 0x0C, OPERAND_VECTOR},
	{"v13", 0x0D, OPERAND_VECTOR},
	{"v14", 0x0E, OPERAND_VECTOR},
	{"v15", 0x0F, OPERAND_VECTOR},
	{"v16", 0x10, OPERAND_VECTOR},
	{"v17", 0x11, OPERAND_VECTOR},
	{"v18", 0x12, OPERAND_VECTOR},
	{"v19", 0x13, OPERAND_VECTOR},
	{"v20", 0x14, OPERAND_VECTOR},
	{"v21", 0x15, OPERAND_VECTOR},
	{"v22", 0x16, OPERAND_VECTOR},
	{"v23", 0x17, OPERAND_VECTOR},
	{"v24", 0x18, OPERAND_VECTOR},

	{"vr" , 0x19, OPERAND_VECTOR},
};

typedef struct encoded_descriptor_s {
	uint8_t register_id : 6;
	uint8_t size : 2;
} encoded_descriptor_t;
