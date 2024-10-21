#include <kernel.h>
#include <paf.h>
#include "main.hpp"
#include <taihen.h>
#include "common.hpp"
#include "utils.hpp"
#include <vshbridge.h>
#include <libsysmodule.h>
#include <appmgr.h>
#include <kernel/modulemgr.h>

#define HOOK_NUM 9

SceUID hooks[HOOK_NUM];
tai_hook_ref_t hook_refs[HOOK_NUM];

Plugin *topmenu;
Widget *scene;
Widget *scrollView;

Widget *baseWidget;

/*
DEX 3.60:
    0x2d1c:     GetIconMemoryPool
    0x2c74:     InitIconMemoryPool
    0xb9198:    Init Icons
*/

SceVoid onPluginReady(Plugin *plugin)
{
    if (plugin == NULL)
    {
        print("Error loading plugin!\n"); 
        return;
    }
    else
        print("Made Plugin Successfully!\n");
}

void MakeNewLA()
{
    if (Plugin::Find("cell_plugin") != NULL)
    {
        onPluginReady(Plugin::Find("cell_plugin"));
        return;
    }

    paf::Plugin::InitParam piParam;

    piParam.pluginName = "cell_plugin";
    piParam.resourcePath = "ur0:data/cell_plugin.rco";
    piParam.scopeName = "__main__";

    piParam.pluginStartCB = onPluginReady;

    paf::Framework::LoadPlugin(&piParam);
}

SceInt32 StartThread(SceSize, void *)
{
    sceKernelDelayThread(10000000);

    auto plugin = Plugin::Find("applauncher_plugin");
    print("plugin %p\n", plugin);
    rco::Element e;
    e.hash = 0xDBE17FCE;
    auto page = plugin->GetPageByHash(&e);
    print("page %p\n", page);
    Utils::PrintAllChildren(page, 0);
    return sceKernelExitDeleteThread(0);
}

void patch(int)
{
    return;
}
void *ReturnNULLPatch(void)
{
    return NULL;
}

void DoNothingPatch(void)
{
    return;
}

void SpawnBubblesPatch(void)
{
    print("SPAWN BUBBLES!\n");
    return;
}

void log(SceInt32 e, Widget *w, SceInt32 unk, ScePVoid pUserData)
{
    print("[%s (0x%X)] Unk = 0x%X Callback ID: 0x%X Original Callback = 0x%X\n", w != NULL ? w->name() : "", w != NULL ? w->elem.hash : 0, unk, e, (int)pUserData);
}

void PluginCBHook(void **p1, Plugin *p2)
{
    //Do nothing haha    
}

void InitIconTasksPatched(graph::SurfacePool * pool)
{
    
}

void somePatch(int a1, int a2)
{
    print("somePatch: %x %d\n", a1, a2);
    return TAI_NEXT(somePatch, hook_refs[8], a1, a2);
}

SceInt32 RegisterEventCallback_hook(Widget *thisw, SceInt32 id, ui::EventCallback *cb, SceBool a3)
{
    print("[%s (0x%X)] Callback Registered: 0x%X cb->eventHandler = 0x%X a3 = 0x%X return: 0x%X\n", thisw->name(), thisw->elem.hash, id, cb->eventHandler, a3, __builtin_return_address());
    ui::EventCallback *ecb = new ui::EventCallback();
    ecb->eventHandler = (ui::EventCallback::EventHandler)log;
    ecb->pUserData = (void *)cb->eventHandler;
    thisw->RegisterEventCallback(id, ecb, 0);
    return TAI_NEXT(RegisterEventCallback_hook, hook_refs[6], thisw, id, cb, a3);
}

static int sceSysmoduleLoadModuleInternalWithArgPatched(SceUInt32 id, SceSize args, void *argp, void *unk) 
{
    int res = TAI_NEXT(sceSysmoduleLoadModuleInternalWithArgPatched, hook_refs[7], id, args, argp, unk);

    if (res >= 0 && id == SCE_SYSMODULE_INTERNAL_PAF) 
    {
        hooks[6] = taiHookFunctionImport(&hook_refs[6], (const char *)TAI_MAIN_MODULE, 0x073F8C68, 0xFB7FE189, RegisterEventCallback_hook);
    }

    return res;
}

int **NewBubblePatch(int **a1)
{
    print("New bubble patch!\n");
    return a1;
}

extern "C"
{
    SceInt32 module_start()
    {
        print("Cell, By Ibrahim\n");

        tai_module_info_t info;
        info.size = sizeof(info);
        if(taiGetModuleInfo("ScePaf", &info) >= 0)
        {
            hooks[6] = taiHookFunctionImport(&hook_refs[6], (const char *)TAI_MAIN_MODULE, 0x073F8C68, 0xFB7FE189, RegisterEventCallback_hook);
        }
        else
        {
            hooks[7] = taiHookFunctionImport(&hook_refs[7], (const char *)TAI_MAIN_MODULE, 0x03FCF19D, 0xC3C26339, sceSysmoduleLoadModuleInternalWithArgPatched);
        }

        
        if (taiGetModuleInfo("SceShell", &info) >= 0)
        {

            print("Hooking....\n");
        
            //hooks[0] = taiHookFunctionOffset(&hook_refs[0], info.modid, 0, 0xa6246, 1, PluginCBHook); //Init
            // hooks[1] = taiHookFunctionOffset(&hook_refs[1], info.modid, 0, 0xa5d0a, 1, PluginCBHook); //Start
            // hooks[2] = taiHookFunctionOffset(&hook_refs[2], info.modid, 0, 0xa5da6, 1, PluginCBHook); //stop
            // hooks[3] = taiHookFunctionOffset(&hook_refs[3], info.modid, 0, 0xa5dd8, 1, PluginCBHook); //exit
            
            hooks[0] = taiHookFunctionOffset(&hook_refs[0], info.modid, 0, 0x2d1c, 1, ReturnNULLPatch); // Get Icon MemoryPool
            hooks[1] = taiHookFunctionOffset(&hook_refs[1], info.modid, 0, 0x2c74, 1, DoNothingPatch);  // Icon MemoryPool init

            hooks[2] = taiHookFunctionOffset(&hook_refs[2], info.modid, 0, 0xb9198, 1, SpawnBubblesPatch); // Create Bubbles
            hooks[3] = taiHookFunctionOffset(&hook_refs[3], info.modid, 0, 0xc1dde, 1, DoNothingPatch); // Init Edit mode

            hooks[4] = taiHookFunctionOffset(&hook_refs[4], info.modid, 0, 0x3b9828, 1, InitIconTasksPatched);

            hooks[5] = taiHookFunctionOffset(&hook_refs[5], info.modid, 0, 0xa6246, 1, PluginCBHook);
            hooks[8] = taiHookFunctionOffset(&hook_refs[8], info.modid, 0, 0x1c1e2a, 1, somePatch);
            // hooks[4] = taiHookFunctionOffset(&hook_refs[4], info.modid, 0, 0x3b9828, 1, patch); // Something to do with icons
            // hooks[5] = taiHookFunctionOffset(&hook_refs[5], info.modid, 0, 0xcb2dc, 1, NewBubblePatch);

#ifdef _DEBUG
            for (int i = 0; i < HOOK_NUM; i++)
                print("%d. 0x%X\n", i + 1, hooks[i]);
#endif
        }
        else
        {
            print("Error getting shell modinfo\n");
            return SCE_KERNEL_START_FAILED;
        }
        
        
        // New LA
        //sceKernelStartThread(sceKernelCreateThread("cell_start", StartThread, 191, SCE_KERNEL_16KiB, 0, SCE_KERNEL_THREAD_CPU_AFFINITY_MASK_DEFAULT, NULL), 0, NULL);

        return SCE_KERNEL_START_SUCCESS;
    }

    SceInt32 module_stop()
    {
        for(int i = 0; i < HOOK_NUM; i++)
            taiHookRelease(hooks[i], hook_refs[i]);
        return SCE_KERNEL_STOP_SUCCESS;
    }
}
