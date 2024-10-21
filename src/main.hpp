#ifndef MAIN_HPP
#define MAIN_HPP

#define HIDE_CHILD(widget, num) { Widget *childNo##num = widget->GetChildByNum(num); if(childNo##num != NULL) { childNo##num->PlayAnimationReverse(0, Widget::Animation_Reset); print("Deleted: 0x%X\n", childNo##num->hash); } else print("Error widget is NULL!\n"); }


#ifdef _DEBUG

#define print sceClibPrintf

#else

#define print void(NULL);

#endif

#endif