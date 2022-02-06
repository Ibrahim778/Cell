#include "utils.hpp"
#include "common.hpp"
#include "main.hpp"

SceVoid Utils::SetWidgetColor(Widget *w, float r, float g, float b, float a)
{
    Widget::Color col;
    col.r = r;
    col.g = g;
    col.b = b;
    col.a = a;
    w->SetColor(&col);
}

SceVoid Utils::SetWidgetPosition(Widget *wi, float x, float y, float z, float w)
{
    SceFVector4 v;
    v.x = x;
    v.y = y;
    v.z = z;
    v.w = w;
    wi->SetPosition(&v);
}

SceVoid Utils::SetWidgetSize(Widget *wi, float x, float y, float z, float w)
{
    SceFVector4 v;
    v.x = x;
    v.y = y;
    v.z = z;
    v.w = w;
    wi->SetSize(&v);
}

Widget *Utils::GetChildByHash(Widget *Parent, SceUInt32 hash)
{
    if(Parent == NULL) return NULL;
    Resource::Element e;
    e.hash = hash;
    return Parent->GetChildByHash(&e, 0);
}

Widget *Utils::GetSceneByHash(Plugin *p, SceInt32 hash)
{
    Resource::Element e;
    e.hash = hash;
    return p->GetSceneByHash(&e);
}

Widget *Utils::MakeWidget(const char *id, const char *type, const char *style, Widget *root)
{
    Resource::Element search;
    Resource::Element widgetInfo;
    Resource::Element styleInfo;

    search.id = id;
    widgetInfo.hash = search.GetHashById(&search);

    search.id = style;
    styleInfo.hash = search.GetHashById(&search);

    return topmenu->CreateWidgetWithStyle(root, type, &widgetInfo, &styleInfo);
}

Widget *Utils::MakeWidget(SceInt32 id, const char *type, const char *style, Widget *root)
{
    Resource::Element search;
    Resource::Element widgetInfo;
    Resource::Element styleInfo;

    widgetInfo.hash = id;

    search.id = style;
    styleInfo.hash = search.GetHashById(&search);

    return topmenu->CreateWidgetWithStyle(root, type, &widgetInfo, &styleInfo);
}

SceInt32 Utils::SetWidgetLabel(const char *text, Widget *widget)
{
    if(text == NULL || widget == NULL || text[0] == '\0' /* empty string */ ) return;
    WString wstr;
    String str;
    str = text;
    str.ToWString(&wstr);
    print("Assigning to 0x%X text: %s str: %s wstr: %ls\n", widget->hash, text, str.data, wstr.data);

    return widget->SetLabel(&wstr);
}

Widget *Utils::AddFromTemplate(Plugin *p, Widget *targetRoot, const char *id)
{
    Resource::Element e;
    e.id = id;
    e.hash = e.GetHashById(&e);

    Plugin::TemplateInitParam tinit;
    p->TemplateOpen(targetRoot, &e, &tinit);

    return targetRoot->GetChildByNum(targetRoot->childNum - 1);
}

SceUInt32 Utils::GetHashById(const char *id)
{
    Resource::Element e;
    e.id = id;
    return e.GetHashById(&e);
}

SceVoid Utils::HideAllChildren(Widget *widget)
{
    for(int i = 0; i < widget->childNum; i++)
    {
        Widget *w = widget->GetChildByNum(i);
        if(w != NULL) 
        {
            w->PlayAnimationReverse(0, Widget::Animation_Reset);
            w->animationStatus &= 0x80;
        }
    }
}

bool checkFileExist(const char *path)
{
    return paf::io::Misc::Exists(path);
}

SceVoid Utils::PrintAllChildren(Widget *widget, int offset)
{
    for (int i = 0; i < widget->childNum; i++)
    {
        for (int i = 0; i < offset; i++) print(" ");
        WString str;
        widget->GetChildByNum(i)->GetLabel(&str);
        print("%d: 0x%X (%s, \"%ls\")\n", i, widget->GetChildByNum(i)->hash, widget->GetChildByNum(i)->GetType(), str.data);
        Utils::PrintAllChildren(widget->GetChildByNum(i), offset + 4);
    }
    if(widget->childNum == 0 && offset == 0) print("No Children to Print!\n");
}

SceVoid Utils::SetWidgetTypeToColor(Widget *widget, const char *type)
{
    for (int i = 0; i < widget->childNum; i++)
    {
        if(sce_paf_strncmp(widget->GetChildByNum(i)->GetType(), type, 20) == 0)
            widget->GetChildByNum(i)->PlayAnimationReverse(0, Widget::Animation_Reset);
        Utils::SetWidgetTypeToColor(widget->GetChildByNum(i), type);
    }
}