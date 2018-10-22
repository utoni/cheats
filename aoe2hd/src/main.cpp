#include <windows.h>
#include <psapi.h>
#include <stdio.h>
#include <assert.h>

#include <vector>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

#include "CodeGenerator.h"
#include "CodePatcher.h"
#include "CodeInjector.h"
#include "ModuleMemory.h"
extern "C" {
#include "native.h"
#include "aoe2hd.h"
}


int main()
{
    using namespace std;

    native_data nd = {0};
    initNativeData(&nd);

    assert(get_module_proc(&nd, "Age of Empires II: HD Edition"));
    assert(get_module_info(&nd, "AoK HD.exe"));

    ModuleMemory mm(nd);
    printf("PlayerCurrent........: 0x%08lX\n", mm.scanProcMem("PlayerCurrent1", "C7 45 FC 01 00 00 00 C7 05 ?? ?? ?? ?? 00 00 00 00 8B 8E 30 01 00 00", 9));
    printf("PlayerCurrent........: 0x%08lX\n", mm.scanMappedMem("PlayerCurrent2", "C7 45 FC 01 00 00 00 C7 05 ?? ?? ?? ?? 00 00 00 00 8B 8E 30 01 00 00", 9));
    printf("PlayerNameArray......: 0x%08lX\n", mm.scanMappedMem("PlayerNameArray", "D8 1F ?? ?? 3E 00 00 00 80 2A ?? ?? 3E 00 00 00", 16));
    printf("PlayerNameArraySub1..: 0x%08lX\n", mm.getPtr("PlayerNameArraySub1", mm.getPtr("PlayerNameArray"), 0x5C));
    printf("PlayerStructArray....: 0x%08lX\n", mm.scanProcMem("PlayerStructArray", "01 34 47 30 01 AC A2 2E 01 00 00 00 00 9C F2 2F 01", -155));
    printf("PlayerStructArraySub1: 0x%08lX\n", mm.getPtr("PlayerStructArraySub1", mm.getPtr("PlayerStructArray"), 0xC8));
    printf("PlayerStructArraySub2: 0x%08lX\n", mm.getPtr("PlayerStructArraySub2", mm.getPtr("PlayerStructArraySub1"), 0x184));

    mm.ptrSetDependency("PlayerNameArraySub1", "PlayerNameArray");
    mm.ptrSetDependency("PlayerStructArraySub1", "PlayerStructArray");
    mm.ptrSetDependency("PlayerStructArraySub2", "PlayerStructArraySub1");

    for (unsigned long i = 1; i < 9; ++i)
    {
        stringstream player;
        player << "Player" << i;
        mm.getPtr(player.str(), (unsigned long) mm.getPtr("PlayerStructArraySub2"), 0x8 * i);
        mm.ptrSetDependency(player.str(), "PlayerStructArraySub2");

        stringstream player_res;
        player_res << "Player" << i << "Resources";
        mm.getPtr(player_res.str(), (unsigned long) mm.getPtr(player.str()), 0x3C);
        mm.ptrSetDependency(player_res.str(), player.str());

        stringstream player_name;
        player_name << "Player" << i << "Name";
        mm.getPtr(player_name.str(), (unsigned long) mm.getPtr("PlayerNameArraySub1"), 0xBC + (0x68 * (i-1)));
        mm.ptrSetDependency(player_name.str(), "PlayerNameArraySub1");

        printf("%s: 0x%08lX | Resources: 0x%08lX | Name: 0x%08lX\n", player.str().c_str(), mm.getPtr(player.str()),
               mm.getPtr(player_res.str()), mm.getPtr(player_name.str()));
    }

    printf("DRAW_HIGHSCORE_CALL..: 0x%08lX\n", mm.scanProcMem("DRAW_HIGHSCORE_CALL", "FF B5 7C FF FF FF 2B 8D 78 FF FF FF 8D 45 D8 50 51 52 8D 45 8C 50 8D 4D C0", 19, false));
    printf("DRAW_HIGHSCORE_FN....: 0x%08lX\n", mm.scanProcMem("DRAW_HIGHSCORE_FN", "55 8B EC 6A FF 68 ?? ?? ?? ?? 64 A1 ?? ?? ?? ?? 50 83 EC 38 A1 ?? ?? ?? ?? 33 C5 89 45 F0 53 56 57 50 8D 45 F4 64 A3 ?? ?? ?? ?? 8B F1 8B 5D 08", 0, false));

    system("pause");

    CodeInjector ci(nd);
    assert(ci.allocCodeSegment("HighscoreHack"));
    CodeGenerator original(nd), inject(nd);

    inject.addCode("8B 85 1C FF FF FF 8A 00 3C 89 0F 85 6B 00 00 00 60 8B 85 1C FF FF FF FF 48 05 83 85 14 FF FF FF 50"
                   "8B 85 14 FF FF FF C7 00 41 41 41 41 C7 40 04 42 42 42 42 FF B5 20 FF FF FF FF B5 1C FF FF FF"
                   "8B 85 14 FF FF FF 80 38 00 0F 84 03 00 00 00 40 EB F4 2B 85 14 FF FF FF 50 FF B5 14 FF FF FF"
                   "FF B5 10 FF FF FF E8 ?? ?? ?? ?? 61 8B 85 1C FF FF FF FF 40 05 83 AD 14 FF FF FF 50"
                   "E8 ?? ?? ?? ?? E9 ?? ?? ?? ??");

#if 0
    mm.getPtr("MainClass",      nd.proc.modbase,             0x009C7774);
    mm.getPtr("GameClass",      mm.getPtr("MainClass"),      0x4);
    mm.getPtr("PlayerArray",    mm.getPtr("GameClass"),      0x184);
    mm.getPtr("PlayerNameBase", nd.proc.modbase,                     0x006DB62C);
    mm.getPtr("PlayerNamePtr0", mm.getPtr("PlayerNameBase"), 0x794);
    mm.getPtr("PlayerNamePtr1", mm.getPtr("PlayerNamePtr0"), 0x5C);

    mm.ptrSetDependency("GameClass",      "MainClass");
    mm.ptrSetDependency("PlayerArray",    "GameClass");
    mm.ptrSetDependency("PlayerNamePtr0", "PlayerNameBase");
    mm.ptrSetDependency("PlayerNamePtr1", "PlayerNamePtr0");

    for (unsigned long i = 1; i < 9; ++i)
    {
        stringstream player;
        player << "Player" << i;
        mm.getPtr(player.str(), (unsigned long)mm.getPtr("PlayerArray"), 0x8 * i);
        mm.ptrSetDependency(player.str(), "PlayerArray");

        stringstream player_res;
        player_res << "Player" << i << "Resources";
        mm.getPtr(player_res.str(), (unsigned long)mm.getPtr(player.str()), 0x3C);
        mm.ptrSetDependency(player_res.str(), player.str());

        stringstream player_name;
        player_name << "Player" << i << "Name";
        mm.getPtr(player_name.str(), (unsigned long)mm.getPtr("PlayerNamePtr1"), 0xBC + (0x60 * (i-1)));
        mm.ptrSetDependency(player_name.str(), "PlayerNamePtr1");
    }

    CodeInjector ci(nd);
    assert(ci.allocCodeSegment("MapCode"));
    CodePatcher cp(nd);
    CodeGenerator original(nd), injected(nd);

    {
        original.addCode({DUMMY5});
        injected.addCode({MAP_NOFOG0}).addCode({MAP_NOFOGI}).addCode({MAP_NOFOG1}).addCode({DUMMY5});

        assert(ci.addCode("MapCode", "NoFog", injected.buildSize()));
        injected.setRel32JMP(3, MAP_NOFOG, ci.getCodeAddr("MapCode", "NoFog"));
        ci.setCode("MapCode", "NoFog", injected.buildAndClear());

        original.setRel32JMP(0, ci.getCodeAddr("MapCode", "NoFog"), MAP_NOFOG, true).addCode({0x90});
        assert(cp.addPatch("NoFog", MAP_NOFOG, {MAP_NOFOG0,MAP_NOFOG1}, original.buildAndClear()));
        cp.setPatchSuspend("NoFog", 1);
        assert(cp.autoPatch("NoFog"));
    }
    {
        original.addCode({DUMMY5});
        injected.addCode({MAP_MINI0}).addCode({MAP_MINII}).addCode({MAP_MINI1}).addCode({DUMMY5});

        assert(ci.addCode("MapCode", "MiniMap", injected.buildSize()));
        injected.setRel32JMP(3, MAP_MINI, ci.getCodeAddr("MapCode", "MiniMap"));
        ci.setCode("MapCode", "MiniMap", injected.buildAndClear());

        original.setRel32JMP(0, ci.getCodeAddr("MapCode", "MiniMap"), MAP_MINI, true).addCode({0x90,0x90,0x90,0x90});
        assert(cp.addPatch("MiniMap", MAP_MINI, {MAP_MINI0,MAP_MINI1}, original.buildAndClear()));
        cp.setPatchSuspend("MiniMap", 1);
        assert(cp.autoPatch("MiniMap"));
    }
    {
        original.addCode({DUMMY5});
        injected.addCode({MAP_SMTH0}).addCode({MAP_SMTHI}).addCode({MAP_SMTH1}).addCode({DUMMY5})
        .addCode({DUMMY5,DUMMY5,DUMMY5,DUMMY5,DUMMY5});

        assert(ci.addCode("MapCode", "Smth", injected.buildSize()));
        injected.setRel32JMP(3, MAP_SMTH, ci.getCodeAddr("MapCode", "Smth"));
        ci.setCode("MapCode", "Smth", injected.buildAndClear());

        original.setRel32JMP(0, ci.getCodeAddr("MapCode", "Smth"), MAP_SMTH, true).addCode({0x90,0x90,0x90,0x90});
        assert(cp.addPatch("Smth", MAP_SMTH, {MAP_SMTH0,MAP_SMTH1}, original.buildAndClear()));
        cp.setPatchSuspend("Smth", 1);
        assert(cp.autoPatch("Smth"));
    }
    {
        original.addCode({DUMMY5});
        injected.addCode({MAP_UNIT0}).addCode({MAP_UNITI}).addCode({MAP_UNIT1}).addCode({DUMMY5});

        assert(ci.addCode("MapCode", "Units", injected.buildSize()));
        injected.setRel32JMP(3, MAP_UNIT, ci.getCodeAddr("MapCode", "Units"));
        ci.setCode("MapCode", "Units", injected.buildAndClear());

        original.setRel32JMP(0, ci.getCodeAddr("MapCode", "Units"), MAP_UNIT, true).addCode({0x90,0x90,0x90,0x90,0x90});
        assert(cp.addPatch("Units", MAP_UNIT, {MAP_UNIT0,MAP_UNIT1}, original.buildAndClear()));
        cp.setPatchSuspend("Units", 1);
        assert(cp.autoPatch("Units"));
    }

    cout << ci.toString() << endl;
    cout << cp.toString() << endl;
    cout << "[PRESS A KEY TO CONTINUE]" << endl;
    system("pause");
#endif

    while (1)
    {
        cls( GetStdHandle( STD_OUTPUT_HANDLE ));

        while (!mm.recheckPtr("PlayerStructArray"))
        {
            Sleep(1000);
        }
        mm.revalidateAllPtr();

        for (unsigned long i = 1; i < 9; ++i)
        {

            stringstream player_res;
            player_res << "Player" << i << "Resources";
            struct resources res = {0};
            if (!mm.getData(player_res.str(), &res, sizeof(res)))
            {
                continue;
            }

            cout << "player[" << i << "]: "
                 << "wood.: " << setw(8) << dec << (unsigned long)res.wood << ", "
                 << "food.: " << setw(8) << dec << (unsigned long)res.food << ", "
                 << "gold.: " << setw(8) << dec << (unsigned long)res.gold << ", "
                 << "stone: " << setw(8) << dec << (unsigned long)res.stone << endl;
            cout << "          " << setw(8)
                 << "rpop.: " << setw(8) << dec << res.remainingPop << ", "
                 << "tpop.: " << setw(8) << dec << res.currentPop << endl;

            stringstream player_name;
            player_name << "Player" << i << "Name";
            char name[32] = {0};
            mm.getData(player_name.str(), &name, 31);
            cout << "          " << name << endl;
        }

        //cout << mm.toString() << endl;
        //cout << mm.toStringStats() << endl;
        //system("pause");
        Sleep(1000);
    }
    CloseHandle(nd.proc.hndl);

    return 0;
}
