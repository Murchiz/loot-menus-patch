#include "fose/PluginAPI.h"
#include "fose_common/SafeWrite.h"
#include <windows.h>
#include <Psapi.h>

void PatchLootMenuMemory()
{
    HMODULE hLootMenu = GetModuleHandleA("F3LootMenu.dll");
    if (!hLootMenu) return;

    MODULEINFO modInfo;
    if (!GetModuleInformation(GetCurrentProcess(), hLootMenu, &modInfo, sizeof(modInfo))) return;

    UInt8* base = (UInt8*)modInfo.lpBaseOfDll;
    UInt32 size = modInfo.SizeOfImage;

    // 1. Patch GetItemValue (AlchemyItem cast issue)
    for (UInt32 i = 0; i < size - 10; i++)
    {
        if (base[i] == 0x80 && base[i+2] == 0x04 && base[i+3] == 0x2F)
        {
            UInt8 regByte = base[i+1];
            if (regByte == 0x78 || regByte == 0x79 || regByte == 0x7A || 
                regByte == 0x7B || regByte == 0x7E || regByte == 0x7F)
            {
                if (base[i+4] == 0x75)
                {
                    SafeWrite8((UInt32)&base[i+4], 0xEB);
                }
                else if (base[i+4] == 0x0F && base[i+5] == 0x85)
                {
                    SafeWrite8((UInt32)&base[i+4], 0xE9);
                    SafeWrite8((UInt32)&base[i+5], base[i+6]);
                    SafeWrite8((UInt32)&base[i+6], base[i+7]);
                    SafeWrite8((UInt32)&base[i+7], base[i+8]);
                    SafeWrite8((UInt32)&base[i+8], base[i+9]);
                    SafeWrite8((UInt32)&base[i+9], 0x90);
                }
            }
        }
    }

    // 2. Patch 'if (value > 0)' checks before SendCrimeEvent (0x70C030)
    for (UInt32 i = 0; i < size - 4; i++)
    {
        if (*(UInt32*)(base + i) == 0x0070C030)
        {
            // Found a SendCrimeEvent reference. Look backwards for the conditional jump.
            // The conditional jump must skip past this call.
            for (int j = 1; j < 128; j++)
            {
                UInt8* instr = base + i - j;
                
                // Check for 2-byte conditional jumps (7x)
                if ((instr[0] & 0xF0) == 0x70)
                {
                    UInt32 targetAddr = (UInt32)(instr + 2 + (SInt8)instr[1]);
                    // If jump target is AFTER the SendCrimeEvent reference, it's our branch!
                    if (targetAddr > (UInt32)(base + i + 4) && targetAddr < (UInt32)(base + i + 64))
                    {
                        // NOP the jump
                        SafeWrite16((UInt32)instr, 0x9090);
                        break;
                    }
                }
                
                // Check for 6-byte conditional jumps (0F 8x)
                if (instr[0] == 0x0F && (instr[1] & 0xF0) == 0x80)
                {
                    UInt32 targetAddr = (UInt32)(instr + 6 + *(SInt32*)(instr + 2));
                    if (targetAddr > (UInt32)(base + i + 4) && targetAddr < (UInt32)(base + i + 64))
                    {
                        // NOP the 6-byte jump
                        SafeWrite8((UInt32)instr, 0x90);
                        SafeWrite8((UInt32)instr+1, 0x90);
                        SafeWrite32((UInt32)instr+2, 0x90909090);
                        break;
                    }
                }
            }
        }
    }
}

FOSEMessagingInterface* g_msg;

void MessageHandler(FOSEMessagingInterface::Message* msg)
{
    if (msg->type == FOSEMessagingInterface::kMessage_PostLoad)
    {
        PatchLootMenuMemory();
    }
}

extern "C"
{
    bool FOSEPlugin_Query(const FOSEInterface* fose, PluginInfo* info)
    {
        info->infoVersion = PluginInfo::kInfoVersion;
        info->name = "LootMenuKarmaPatch";
        info->version = 1;
        return true;
    }

    bool FOSEPlugin_Load(const FOSEInterface* fose)
    {
        g_msg = (FOSEMessagingInterface*)fose->QueryInterface(kInterface_Messaging);
        if (g_msg)
        {
            g_msg->RegisterListener(fose->GetPluginHandle(), "FOSE", MessageHandler);
        }
        return true;
    }
}
