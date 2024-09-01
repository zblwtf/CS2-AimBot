
#include "Zydis.h"
#include <print>
#include "safetyhook.hpp"
#include <iostream>

bool __fastcall CreateMove(void* pInput, int nSlot, bool bActive)
{
    int a = 1;
    return bActive;
}


void hooked_CreateMove(SafetyHookContext& ctx) {

    uintptr_t pint = ctx.rsp + 8;
    std::cout << ctx.rcx;

#if SAFETYHOOK_ARCH_X86_64
    ctx.rax = 1337;
#elif SAFETYHOOK_ARCH_X86_32
    ctx.eax = 1337;
#endif
    
}

SafetyHookMid g_hook{};

int main() {
    

    // Let's disassemble add_42 and hook its RET.
    // NOTE: On Linux we should specify -falign-functions=32 to add some padding to the function otherwise we'll
    // end up hooking the next function's prologue. This is pretty hacky in general but it's just an example.
    ZydisDecoder decoder{};

#if SAFETYHOOK_ARCH_X86_64
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);
#elif SAFETYHOOK_ARCH_X86_32
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LEGACY_32, ZYDIS_STACK_WIDTH_32);
#endif

    auto ip = reinterpret_cast<uint8_t*>(CreateMove);

    while (*ip != 0xC3) {
        ZydisDecodedInstruction ix{};

        ZydisDecoderDecodeInstruction(&decoder, nullptr, reinterpret_cast<void*>(ip), 15, &ix);

        // Follow JMPs
        if (ix.opcode == 0xE9) {
            ip += ix.length + (int32_t)ix.raw.imm[0].value.s;
        }
        else {
            ip += ix.length;
        }
    }

    CreateMove((void*)1, 2, 3);

    g_hook = safetyhook::create_mid(ip, hooked_CreateMove);

    CreateMove((void*)1, 2, 3);

    g_hook = {};



    return 0;
}