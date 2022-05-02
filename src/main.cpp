#include <kernel.h>
#include <paf.h>
#include "main.hpp"
#include <taihen.h>
#include "common.hpp"
#include "utils.hpp"
#include <vshbridge.h>
#include <libsysmodule.h>
#include <kernel/modulemgr.h>

#define HOOK_NUM 8

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

class LaunchSettingsHandle : public Widget::EventCallback
{
public:
    static void onGet(SceInt32, Widget *self, SceInt32, ScePVoid puserData)
    {
        sceAppMgrLaunchAppByName2ForShell("NPXS10015", NULL, NULL);
    }

    LaunchSettingsHandle()
    {
        eventHandler = onGet;
    }
};

class AppPlaneEventHandler : public Widget::EventCallback
{
public:
    static void onGet(SceInt32 e, Widget *self, SceInt32, ScePVoid pUserData)
    {
        print("Called event: 0x%X\n", e);
        switch (e)
        {

        case Widget::EventFocus_Off:
            print("EventFocus_Off!\n");
            break;
        case Widget::EventFocus_On:
            print("EventFocus_On!\n");
            break;

        }
    }

    AppPlaneEventHandler()
    {
        eventHandler = onGet;
    }
};

class AppPlane
{
private:
    Plane *base;
    Widget *parent;

    Widget *buttons[32];

    SceVoid MakeBase()
    {
        if (base != NULL)
            return;
        base = (Plane *)Utils::MakeWidget("cell_app_plane_base", "plane", "_common_style_plane_transparent", parent);
        Utils::SetWidgetSize(base, 960, 480, 0, 0);
    }

public:
    AppPlane::AppPlane(Widget *parent)
    {
        this->parent = parent;
        base = NULL;

        MakeBase();

        sce_paf_memset(buttons, 0, sizeof(buttons));
    }

    SceInt32 MakeButtons()
    {
        if (base == NULL)
            return -1;

        int currXPos = -420;
        int currYPos = -180;
        for (int x = 0; x < 4; x++, currYPos += 120, currXPos = -420)
        {
            for (int i = 0; i < 8; i++, currXPos += 120)
            {
                int buttonIndex = (x * 4) + i;
                this->buttons[buttonIndex] = Utils::MakeWidget("cell_app_button", "button", "_common_default_style_button", base);
                if(this->buttons[buttonIndex] == NULL) continue;
                Utils::SetWidgetPosition(this->buttons[buttonIndex], currXPos, currYPos, 0, 0);
                Utils::SetWidgetSize(this->buttons[buttonIndex], 100, 100, 0, 0);
            }
        }

        return 0;
    }

    SceInt32 RemoveButtons()
    {
        common::Utils::WidgetStateTransition(0, base, Widget::Animation_Reset, SCE_TRUE, SCE_TRUE);
        base = NULL;

        MakeBase();

        return 0;
    }

    SceVoid RegisterHandlers()
    {
    }
};



SceVoid onPluginReady(Plugin *plugin)
{
    if (plugin == NULL)
    {
        print("Error loading plugin!\n");
        return;
    }
    else
        print("Made Plugin Successfully!\n");

    Widget *basePlane = Utils::MakeWidget("cell_base_plane", "plane", "_common_style_plane_transparent", baseWidget);
    Utils::SetWidgetSize(basePlane, 960, 544, 0, 0);

    ScrollView *scrollView = (ScrollView *)Utils::AddFromTemplate(plugin, basePlane, "main_scroll_view");
    Box *b = (Box *)Utils::GetChildByHash(scrollView, Utils::GetHashById("list_scroll_box"));

    for(int i = 0; i < 5; i++)
    {
        AppPlane aplane = AppPlane(b);
        aplane.MakeButtons();
        aplane.RegisterHandlers();
    }

}

void MakeNewLA()
{
    if (Plugin::Find("cell_plugin") != NULL)
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

SceInt32 StartThread(SceSize, void *)
{
    sceKernelDelayThread(7000000);

    print("##### STARTING MODIFICATIONS #####\n");
    topmenu = Plugin::Find("topmenu_plugin");
    print("topmenu = 0x%X\n", topmenu);
    scene = Utils::GetPageByHash(topmenu, 0x5E9535B3);
    print("scene = 0x%X\n", scene);

    scrollView = Utils::GetChildByHash(scene, 0xE61B084);
    scrollView->PlayAnimationReverse(0, Widget::Animation_Reset);

    baseWidget = Utils::GetChildByHash(scene, 0x18411BF4);
    Widget *w = Utils::GetChildByHash(scene, 0x6D895C97); // Current Page indicator thingy
    if (w)
        common::Utils::WidgetStateTransition(0, w, Widget::Animation_Reset, SCE_TRUE, SCE_TRUE);

    // Make our own
    MakeNewLA();

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
    print("[%s (0x%X)] Unk = 0x%X Callback ID: 0x%X\n", w != NULL ? w->GetType() : "", w != NULL ? w->hash : 0, unk, e);
}


SceInt32 RegisterEventCallback_hook(Widget *thisw, SceInt32 id, Widget::EventCallback *cb, SceBool a3)
{
    Widget::EventCallback *ecb = new Widget::EventCallback();
    ecb->eventHandler = (Widget::EventCallback::EventHandler)log;
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

        /*
        if (taiGetModuleInfo("SceShell", &info) >= 0)
        {

            print("Hooking....\n");
            
            hooks[0] = taiHookFunctionOffset(&hook_refs[0], info.modid, 0, 0x2d1c, 1, ReturnNULLPatch); // Icon MemoryPool init
            hooks[1] = taiHookFunctionOffset(&hook_refs[1], info.modid, 0, 0x2c74, 1, DoNothingPatch);  // Get Icon MemoryPool

            hooks[2] = taiHookFunctionOffset(&hook_refs[2], info.modid, 0, 0xb9198, 1, SpawnBubblesPatch); // Create Bubbles
            hooks[3] = taiHookFunctionOffset(&hook_refs[3], info.modid, 0, 0xc1dde, 1, DoNothingPatch); // Init Edit mode

            hooks[4] = taiHookFunctionOffset(&hook_refs[4], info.modid, 0, 0x3b9828, 1, patch); // Something to do with icons
            hooks[5] = taiHookFunctionOffset(&hook_refs[5], info.modid, 0, 0xcb2dc, 1, NewBubblePatch);
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
        */
        
        // New LA
        //sceKernelStartThread(sceKernelCreateThread("cell_start", StartThread, 191, SCE_KERNEL_16KiB, 0, SCE_KERNEL_THREAD_CPU_AFFINITY_MASK_DEFAULT, NULL), 0, NULL);

        return SCE_KERNEL_START_SUCCESS;
    }

    SceInt32 module_stop()
    {
        return SCE_KERNEL_STOP_SUCCESS;
    }
}
