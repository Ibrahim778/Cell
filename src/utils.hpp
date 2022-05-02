#ifndef TYPES_HPP_QM_REBORN
#define TYPES_HPP_QM_REBORN
#include <libdbg.h>
#include <kernel.h>
#include <appmgr.h>
#include <kernel/libkernel.h>
#include <paf.h>

using namespace paf;
using namespace ui;

class Utils
{
public:
    static Widget *GetChildByHash(Widget *parent, SceUInt32 hash);
    static SceInt32 SetWidgetLabel(const char *text, Widget *widget);
    static SceUInt32 GetHashById(const char *id);
    static SceVoid SetWidgetColor(Widget *w, float r, float g, float b, float a);
    static SceVoid SetWidgetPosition(Widget *wi, float x, float y, float z, float w);
    static SceVoid SetWidgetSize(Widget *wi, float x, float y, float z, float w);
    static Widget *GetPageByHash(Plugin *p, SceInt32 hash);
    static SceVoid PrintAllChildren(Widget *widget, int offset);
    static Widget *MakeWidget(SceInt32 id, const char *type, const char *style, Widget *root);
    static Widget *MakeWidget(const char *id, const char *type, const char *style, Widget *root);
    static SceVoid HideAllChildren(Widget *widget);
    static Widget *AddFromTemplate(Plugin *p, Widget *targetRoot, const char *id);
    static SceVoid SetWidgetTypeToColor(Widget *widget, const char *type);
};

#endif