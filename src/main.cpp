#include <kernel.h>
#include <paf.h>
#include "main.hpp"
#include <taihen.h>
#include "common.hpp"
#include "utils.hpp"
#include <vshbridge.h>
#include <libsysmodule.h>
#include <kernel/modulemgr.h>

extern "C" {
    SceUID sceKernelGetModuleIdByAddr(void*); SceInt32 sceKernelGetModuleInfo(SceUID, SceKernelModuleInfo*);
}

#define HOOK_NUM 8

SceUID hooks[HOOK_NUM];
tai_hook_ref_t hook_refs[HOOK_NUM];

Plugin *topmenu;
Widget *scene;
Widget *scrollView;

Widget *baseWidget;

/*
DEX 3.60: segment_0
    0x2d1c:     GetIconMemoryPool
    0x2c74:     InitIconMemoryPool
    0xb9198:    Init Icons

    IconCaptureManager is the service that creates and stores the textures for folder icons
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

    Plugin::InitParam piParam;

    piParam.pluginName = "cell_plugin";
    piParam.resourcePath = "ur0:data/cell_plugin.rco";
    piParam.scopeName = "__main__";

    piParam.pluginStartCB = onPluginReady;

    Plugin::LoadSync(piParam);
}

SceInt32 sceKernelGetModuleInfoByAddr(void* addr, SceKernelModuleInfo* pInfo) {
  SceUID modid = sceKernelGetModuleIdByAddr(addr);
  if (modid < 0)
    return modid;
  return sceKernelGetModuleInfo(modid, pInfo);  
}

SceInt32 print_address_location(void* addr) {
  SceKernelModuleInfo info;
  info.size = sizeof(info);

  SceInt32 status = sceKernelGetModuleInfoByAddr(addr, &info);
  if (status < 0)
    return status;

  SceUIntPtr target = (SceUIntPtr)(addr);
  SceInt segment = -1;
  SceUInt offset = 0;
  for (unsigned i = 0; i < (sizeof(info.segments)/sizeof(info.segments[0])); i++) {
    SceUIntPtr segBase = (SceUIntPtr)(info.segments[i].vaddr);
    SceUIntPtr segMax = segBase + info.segments[i].memsz;
    if ((segBase <= target) && (target < segMax)) {
      segment = i;
      offset = target - segBase;
      break;
    }
  }
  if (segment == -1)
    return -1;

  sceClibPrintf("%p -> [%s:%d + 0x%08X]\n", addr, info.module_name, segment, offset);
}

int module_get_offset(SceUID modid, SceSize segidx, uint32_t offset, void *stub_out){

	int res = 0;
	SceKernelModuleInfo info;

	if(segidx > 3){
		return -1;
	}

	if(stub_out == NULL){
		return -2;
	}

	res = sceKernelGetModuleInfo(modid, &info);
	if(res < 0){
		return res;
	}

	if(offset > info.segments[segidx].memsz){
		return -3;
	}
#ifndef __INTELLISENSE__
	*(uint32_t *)stub_out = (uint32_t)(info.segments[segidx].vaddr + offset);
#endif
	return 0;
}

void *unkFunc1(void *a1)
{
    return a1;
}

void unkFunc2(void *)
{
    
}

void unkFunc3(int, int ,int)
{

}

void unkFunc4(float, int ,int)
{

}

void unkFunc5(int, int)
{

}

typedef struct {
    Plugin *pPlugin;
    Widget *pWidget;
    SceByte bUnk0;
    SceByte bUnk1;
    char    unk0[2];
    int     iUnk0;
    int     iUnk1;
} unkStruct;

void *funcList[] = {
    unkFunc1,
    unkFunc2,
    unkFunc3,
    unkFunc4,
    unkFunc5,
};

void PrintWidgetInfo(Widget *w)
{
    if(w)
        print("[0x%X (%s)] %s\n", w->elem.hash, w->elem.id.c_str(), w->name());
    else print("Widget NULL\n");
}

void globalVarPatch(void **data, Plugin *plugin)
{
    TAI_NEXT(globalVarPatch, hook_refs[0], data, plugin);
    data[4] = funcList;
    paf::thread::s_mainThreadMutex.Lock();
    // effect::Play(0, (Widget *)data[0x23], effect::EffectType_Reset, SCE_TRUE, SCE_TRUE);
    // data[0x23] = 0;
    // effect::Play(0, (Widget *)data[0x21], effect::EffectType_Reset, SCE_TRUE, SCE_TRUE);
    // data[0x21] = 0;

    // effect::Play(0, (Widget *)data[0x22], effect::EffectType_Reset, SCE_TRUE, SCE_TRUE);
    // data[0x22] = 0;
    Widget *widget = (Widget *)data[0x22];
    widget->UnregisterEventCallback(0x10001, 0, 0);
    widget->UnregisterEventCallback(0x10003, 0, 0);
    widget->UnregisterEventCallback(0x10000002, 0, 0);
    widget->UnregisterEventCallback(0x800008, 0, 0);
    paf::thread::s_mainThreadMutex.Unlock();

    // effect::Play(0, ((unkStruct *)data[0x27])->pWidget, effect::EffectType_Reset, SCE_TRUE, SCE_TRUE);
    // PrintWidgetInfo((Widget *)data[0x23]);
    // PrintWidgetInfo((Widget *)data[0x21]);
    // PrintWidgetInfo((Widget *)data[0x22]);
    // PrintWidgetInfo(((unkStruct *)data[0x22])->pWidget);
    //0x23
    //0x21
    //0x22
    //0x27 unkStruct
    return;
}
SceInt32 StartThread(SceSize, void *)
{
    rco::Element e;

    sceKernelDelayThread(7000000);

    print("##### STARTING MODIFICATIONS #####\n");
    
    topmenu = Plugin::Find("topmenu_plugin");
    print("topmenu = 0x%X\n", topmenu);
    
    e.hash = 0x5E9535B3;
    scene = topmenu->GetPageByHash(&e);
    
    print("scene = 0x%X\n", scene);
    Utils::PrintAllChildren(scene, 0);
    scrollView = Utils::GetChildByHash(scene, 0xE61B084);
    effect::Play(0, scrollView, effect::EffectType_Reset, SCE_TRUE, SCE_TRUE);
    // scrollView->PlayEffectReverse(0, effect::EffectType_Reset);

    // baseWidget = Utils::GetChildByHash(scene, 0x18411BF4);
    // Widget *w = Utils::GetChildByHash(scene, 0x6D895C97); // Current Page indicator thingy
    // if (w)
    //     effect::Play(0, w, effect::EffectType_Reset, SCE_TRUE, SCE_TRUE);


    tai_module_info_t info;
    info.size = sizeof(info);
    if(taiGetModuleInfo("SceShell", &info) >= 0)
    {
        // Plugin *topmenu = Plugin::Find("topmenu_plugin");
        // void *topInterface = topmenu->GetInterface(1);

        // // void (*topmenuFunction)(int) = (void (*)(int))((int)*(int **)topInterface + 0xf0);
        // void (*unkFunc)(void) = SCE_NULL;
        // void (*bubbleDelete)(void) = SCE_NULL;

        // module_get_offset(info.modid, 0, 0xbf5f4 | 1, &unkFunc);
        // module_get_offset(info.modid, 0, 0xb92a4 | 1, &bubbleDelete);
        
        // print("%p %p\n", unkFunc, bubbleDelete);
        // // topmenuFunction(0);
        // // print("Done 1\n");
        // unkFunc();
        // print("Done 2\n");
        // bubbleDelete();
        // print("Done 3\n");
    }

    // Make our own
    // MakeNewLA();

END:
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
int **NewBubblePatch(int **a1)
{
    print("New bubble patch!\n");
    return a1;
}

void interfPatch(void *)
{
    print("Patched function called!!\n");
}

typedef struct TopMenuInterface {
    char stuff[0xEC];
    void *funcToPatch;
};

void *GetPluginInterfacePatched(char *pluginName, SceUInt32 interfaceNum)
{
    void *ret = TAI_NEXT(GetPluginInterfacePatched, hook_refs[1], pluginName, interfaceNum);
    if(ret != SCE_NULL && sce_paf_strncmp("topmenu_plugin", pluginName, 14) == 0 && interfaceNum == 1)
    {
        print("Patching interface...\n");
        // *((void **)((int)ret + 0xf0)) = interfPatch;
        print("Done!\n");
    }
    return ret;
}

void unkFuncPatch(void **interf, Plugin *plug, int unk1, int unk2, int unk3)
{
    return;
}

extern "C"
{
    SceInt32 module_start(SceSize args, ScePVoid argp)
    {
        print("Cell, By Ibrahim\n");

        tai_module_info_t info;
        info.size = sizeof(info);
        // if(taiGetModuleInfo("ScePaf", &info) >= 0)
        // {
        //     hooks[6] = taiHookFunctionImport(&hook_refs[6], (const char *)TAI_MAIN_MODULE, 0x073F8C68, 0xFB7FE189, RegisterEventCallback_hook);
        // }
        // else
        // {
        //     hooks[7] = taiHookFunctionImport(&hook_refs[7], (const char *)TAI_MAIN_MODULE, 0x03FCF19D, 0xC3C26339, sceSysmoduleLoadModuleInternalWithArgPatched);
        // }

        if (taiGetModuleInfo("SceShell", &info) >= 0)
        {

            print("Hooking....\n");
            // hooks[0] = taiHookFunctionOffset(&hook_refs[0], info.modid, 0, 0xa6246 | 1, 1, (void *)globalVarPatch);
            // hooks[1] = taiHookFunctionOffset(&hook_refs[1], info.modid, 0, 0x27ceda | 1, 1, (void *)unkFuncPatch);
            
            hooks[0] = taiHookFunctionOffset(&hook_refs[0], info.modid, 0, 0x2d1c, 1, (void *)ReturnNULLPatch); // Get Icon MemoryPool
            hooks[1] = taiHookFunctionOffset(&hook_refs[1], info.modid, 0, 0x2c74, 1, (void *)DoNothingPatch);  // Icon MemoryPool Init

            hooks[2] = taiHookFunctionOffset(&hook_refs[2], info.modid, 0, 0xb9198, 1, (void *)SpawnBubblesPatch); // Create Bubbles
            hooks[3] = taiHookFunctionOffset(&hook_refs[3], info.modid, 0, 0xc1dde, 1, (void *)DoNothingPatch); // Init Edit mode

            hooks[4] = taiHookFunctionOffset(&hook_refs[4], info.modid, 0, 0x3b9828, 1, (void *)patch); // Something to do with icons
            hooks[5] = taiHookFunctionOffset(&hook_refs[5], info.modid, 0, 0xcb2dc, 1, (void *)NewBubblePatch);
#ifdef _DEBUG
            for (int i = 0; i < HOOK_NUM; i++)
                print("%d. 0x%X\n", i + 1, hooks[i]);
#endif
        }
//         else
//         {
//             print("Error getting shell modinfo\n");
//             return SCE_KERNEL_START_FAILED;
//         }
        
        // New LA
        sceKernelStartThread(sceKernelCreateThread("cell_start", StartThread, 191, SCE_KERNEL_16KiB, 0, SCE_KERNEL_THREAD_CPU_AFFINITY_MASK_DEFAULT, NULL), 0, NULL);

        return SCE_KERNEL_START_SUCCESS;
    }

    SceInt32 module_stop(SceSize args, ScePVoid argp)
    {
        return SCE_KERNEL_STOP_SUCCESS;
    }
}
