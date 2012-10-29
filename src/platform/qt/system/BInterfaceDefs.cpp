#include <InterfaceDefs.h>

#include <View.h>

#include <QApplication>


uint32
modifiers()
{
	return BView::FromQtModifiers(QApplication::keyboardModifiers());
}


rgb_color
ui_color(color_which which)
{
// TODO:...
	return make_color(0, 0, 0);
}
