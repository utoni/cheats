/************************************
 * AoE2 HD Steam
 * Minimap/NoFog-Hack (with a restriction)
 *
 * coded by lnslbrty/dev0, \x90
 *
 * This hack may cause desyncs!
 ************************************/

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


int main(int argc, char **argv)
{
    using namespace std;

    (void) argc;
    (void) argv;

    native_data nd = {0};
    initNativeData(&nd);

    assert(get_module_proc(&nd, "Age of Empires II: HD Edition"));
    assert(get_module_base(&nd, "AoK HD.exe"));

    ModuleMemory mm(nd);
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

    while (1)
    {
        cls( GetStdHandle( STD_OUTPUT_HANDLE ));

        while (!mm.recheckPtr("MainClass"))
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
