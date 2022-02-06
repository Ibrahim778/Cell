#include <kernel.h>
#include <paf.h>
#include "main.hpp"
#include <taihen.h>
#include "common.hpp"
#include "utils.hpp"
#include <vshbridge.h>
#include <kernel/modulemgr.h>

#define HOOK_NUM 15

SceUID hooks[HOOK_NUM];
tai_hook_ref_t hook_refs[HOOK_NUM];

Plugin *topmenu;
Widget *scene;
Widget *scrollView;

Widget *basePlane;

/*
0x2d1c:     GetIconMemoryPool
0x2c74:     InitIconMemoryPool
0xb9198:    Init Icons

0x27dc84:   unk_crashcause1
0x27c6ae:   unk_crashcause2
0x275ece:   unk_crashcause3, calls crashcause1 & 2
0xd1476:    unk_crashcause4, calls 0xD15B4 (also crashcause)
0x27d4a0:   unk_crashcause5, calls caller of crashcause4
*/

extern "C" SceUID sceKernelGetModuleIdByAddr(void*);
SceInt32 sceKernelGetModuleInfoByAddr(void* addr, SceKernelModuleInfo* pInfo) {
  SceUID modid = sceKernelGetModuleIdByAddr(addr);
  if (modid < 0)
    return modid;
  return sceKernelGetModuleInfo(modid, pInfo);  
}

class LaunchSettingsHandle : public Widget::EventCallback
{
public:

	static void onGet(SceInt32 , Widget *self, SceInt32, ScePVoid puserData)
	{
		sceAppMgrLaunchAppByName2ForShell("NPXS10015", NULL, NULL);
	}

	LaunchSettingsHandle()
	{
		eventHandler = onGet;
	}

};

SceVoid onPluginReady(Plugin *plugin)
{
	if(plugin == NULL)
	{ 
		print("Error loading plugin!\n");
		return;
	}
    else print("Made Plugin Successfully!\n");



}

void MakeNewLA()
{
	if(Plugin::Find("cell_plugin") != NULL)
	{
		onPluginReady(Plugin::Find("cell_plugin"));
		return;
	}

    Framework::PluginInitParam piParam;

    piParam.pluginName = "cell_plugin";
    piParam.resourcePath = "ur0:data/cell_plugin.rco";
    piParam.scopeName = "__main__";

    piParam.pluginStartCB = onPluginReady;

    paf::Framework::LoadPlugin(&piParam);
}


SceInt32 print_address_location(void* addr) 
{
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

void *ReturnNULLPatch(void)
{
    return NULL;
}

void DoNothingPatch(void)
{
    return;
}

SceInt32 AddTemplateHook(paf::Plugin *plugin, paf::ui::Widget *targetRoot, paf::Resource::Element *templateSearchParam, paf::Plugin::TemplateInitParam *param)
{
    print("[%s] Adding Template: targetRoot = 0x%X, search = 0x%X (%s)\n", plugin->pluginName, targetRoot->hash, templateSearchParam->hash, templateSearchParam->id.data);
    return TAI_NEXT(AddTemplateHook, hook_refs[0], plugin, targetRoot, templateSearchParam, param);
}

Widget *MakeStyleHook(paf::Plugin *plugin, paf::ui::Widget *parent, const char *widgetType, paf::Resource::Element *widgetInfo, paf::Resource::Element *styleInfo)
{
    print("[%s] Making %s: parent = 0x%X, style = 0x%X (%s), widget = 0x%X (%s)\n", plugin->pluginName, widgetType, parent->hash, styleInfo->hash, styleInfo->id.data, widgetInfo->hash, widgetInfo->id.data);
    return TAI_NEXT(MakeStyleHook, hook_refs[1], plugin, parent, widgetType, widgetInfo, styleInfo);
}

Widget *MakeSceneHook(paf::Plugin *plugin, Resource::Element *sceneInfo, Plugin::SceneInitParam *initParam)
{
    print("[%s] Making Scene: hash = 0x%X\n", plugin->pluginName, sceneInfo->hash);
    return TAI_NEXT(MakeSceneHook, hook_refs[2], plugin, sceneInfo, initParam);
}

Widget *GetChildByHashHook(Widget *parent, paf::Resource::Element *widgetInfo, SceUInt32 param)
{
    Widget *next = TAI_NEXT(GetChildByHashHook, hook_refs[3], parent, widgetInfo, param);
    print("[%s (0x%X)] Getting: 0x%X Got Child: (%s), called from: %p\n", parent->GetType(), parent->hash, widgetInfo->hash, next == NULL ? "None" : next->GetType(), __builtin_return_address());
    return next;
}

Widget *GetChildByNumHook(Widget *parent, SceUInt32 childNum)
{
    Widget *child = TAI_NEXT(GetChildByNumHook, hook_refs[4], parent, childNum);
    print("[%s (0x%X)] Getting child at number: %d, Got: (%s, 0x%X), called from %p\n", parent->GetType(), parent->hash, childNum, child != NULL ? child->GetType() : "None", child != NULL ? child->hash : 0, __builtin_return_address());
    return child;
}

SceUInt32 GetHashByIdHook(Resource::Element *caller, Resource::Element *out)
{
    SceUInt32 r = TAI_NEXT(GetHashByIdHook, hook_refs[5], caller, out);
    print("[0x%X, %s] Output: %s, 0x%X called from: %p\n", caller->hash, caller->id.data, out->id.data, out->hash, __builtin_return_address());
    return r;
}

SceInt32 SetPositionHook(Widget *widget, const SceFVector4 *pPosition, SceFloat32 a2 = 0.0f, SceInt32 a3 = 0, SceInt32 a4 = 0x10000, SceInt32 a5 = 0, SceInt32 a6 = 0, SceInt32 a7 = 0)
{
    print("[%s (0x%X)] Setting Position: X: %2f Y: %2f Z: %2f W: %2f %p\n", widget->GetType(), widget->hash, pPosition->x, pPosition->y, pPosition->z, pPosition->w, __builtin_return_address());
    return TAI_NEXT(SetPositionHook, hook_refs[6], widget, pPosition, a2, a3, a4, a5, a6, a7);
}

SceInt32 StartThread(SceSize, void*)
{	
	sceKernelDelayThread(7000000);

	print("##### STARTING MODIFICATIONS #####\n");
    topmenu = Plugin::Find("topmenu_plugin");
    print("topmenu = 0x%X\n", topmenu);
    scene = Utils::GetSceneByHash(topmenu, 0x5E9535B3);
    print("scene = 0x%X\n", scene);

    Widget *w = scene->GetChildByNum(0);
    print("%d: 0x%X -> %s\n",0, w->hash, w->GetType());

    w->PlayAnimationReverse(0, Widget::Animation_Reset);

	scrollView = Utils::GetChildByHash(scene, 0xE61B084);
    scrollView->PlayAnimationReverse(0, Widget::Animation_Reset);

    basePlane = Utils::GetChildByHash(scene, 0x18411BF4);

	//Make our own
	MakeNewLA();

END:
	return sceKernelExitDeleteThread(0);

}

void patch(int)
{
    return;
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

	*(uint32_t *)stub_out = (uint32_t)(info.segments[segidx].vaddr + offset);

	return 0;
}

extern "C"
{
    SceInt32 module_start()
    {
        print("Cell, By Ibrahim\n");


		tai_module_info_t info;
		info.size = sizeof(info);
		if(taiGetModuleInfo("SceShell", &info) >= 0)
		{
            
            print("Hooking....\n");
            //hooks[0] = taiHookFunctionOffset(&hook_refs[0], info.modid, 0, 0x27419c, 1, LAChangePatch);
            //hook_id = taiHookFunctionImport(&hook_ref, "SceShell", TAI_ANY_LIBRARY, 0x6735B503, ShellPatch);

            //hooks[7] = taiHookFunctionOffset(&hook_refs[7], info.modid, 0, 0xc0366, 1, ShellPatch);
            hooks[7] = taiHookFunctionOffset(&hook_refs[7], info.modid, 0, 0x2d1c, 1, ReturnNULLPatch); // Icon MemoryPool init
            hooks[8] = taiHookFunctionOffset(&hook_refs[8], info.modid, 0, 0x2c74, 1, DoNothingPatch); // Get Icon MemoryPool

            hooks[9] = taiHookFunctionOffset(&hook_refs[9], info.modid, 0, 0xb9198, 1, DoNothingPatch); // Create Bubbles
            hooks[10] = taiHookFunctionOffset(&hook_refs[10], info.modid, 0, 0xc1dde, 1, DoNothingPatch); // Init Edit mode

            hooks[11] = taiHookFunctionOffset(&hook_refs[11], info.modid, 0, 0x3b9828, 1, patch); // Something to do with icons

            //hooks[9] = taiHookFunctionOffset(&hook_refs[9], info.modid, 0, 0x27dc84, 1, ShellPatch);
            //hooks[10] = taiHookFunctionOffset(&hook_refs[10], info.modid, 0, 0x27c6ae, 1, ShellPatch2);
            //hooks[11] = taiHookFunctionOffset(&hook_refs[11], info.modid, 0, 0x275ece, 1, MaybePatch);

            /*
            hooks[9] = taiHookFunctionOffset(&hook_refs[9], info.modid, 0, 0xc0366, 1, ShellPatch);
            hooks[10] = taiHookFunctionOffset(&hook_refs[10], info.modid, 0, 0xb94a0, 1, ShellPatch2);
            hooks[11] = taiHookFunctionOffset(&hook_refs[11], info.modid, 0, 0xb1cea, 1, ShellPatch3);
            */
        }
		else
		{ 
			print("Error getting shell modinfo\n");
			return SCE_KERNEL_START_FAILED;
		}


		//New LA widgets
        sceKernelStartThread(sceKernelCreateThread("cell_start", StartThread, 191, SCE_KERNEL_16KiB, 0, SCE_KERNEL_THREAD_CPU_AFFINITY_MASK_DEFAULT, NULL), 0, NULL);
        
        return SCE_KERNEL_START_SUCCESS;
    }

    SceInt32 module_stop()
    {
        return SCE_KERNEL_STOP_SUCCESS;
    }
}
