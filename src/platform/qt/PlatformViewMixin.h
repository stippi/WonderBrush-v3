#ifndef PLATFORM_QT_PLATFORM_VIEW_MIXIN_H
#define PLATFORM_QT_PLATFORM_VIEW_MIXIN_H


#include <View.h>

#include <QPainter>
#include <QPaintEvent>


struct PlatformDrawContext {
	PlatformDrawContext(BView* view, QPaintEvent* paintEvent)
		:
		fView(view),
		fPaintEvent(paintEvent)
	{
	}

	BView* View() const
	{
		return fView;
	}

	BRect UpdateRect() const
	{
		return BRect::FromQRect(fPaintEvent->rect());
	}

	// platform specific interface follows

	QPaintEvent* PaintEvent() const
	{
		return fPaintEvent;
	}

	QPainter& Painter()
	{
		if (!fPainter.isActive())
			fPainter.begin(fView);
		return fPainter;
	}

private:
	BView*			fView;
	QPaintEvent*	fPaintEvent;
	QPainter		fPainter;
};


template<typename BaseClass>
class PlatformViewMixin : public BaseClass {
public:
	template<typename Parameter1, typename Parameter2>
	PlatformViewMixin(Parameter1 parameter1, Parameter2 parameter2)
		:
		BaseClass(parameter1, parameter2)
	{
	}

	template<typename Parameter1, typename Parameter2, typename Parameter3,
		typename Parameter4>
	PlatformViewMixin(Parameter1 parameter1, Parameter2 parameter2,
		Parameter3 parameter3, Parameter4 parameter4)
		:
		BaseClass(parameter1, parameter2, parameter3, parameter4)
	{
	}

	template<typename Parameter1, typename Parameter2, typename Parameter3,
		typename Parameter4, typename Parameter5, typename Parameter6>
	PlatformViewMixin(Parameter1 parameter1, Parameter2 parameter2,
		Parameter3 parameter3, Parameter4 parameter4, Parameter5 parameter5,
		Parameter6 parameter6)
		:
		BaseClass(parameter1, parameter2, parameter3, parameter4, parameter5,
			parameter6)
	{
	}

	virtual void PlatformDraw(PlatformDrawContext& drawContext)
	{
	}

protected:
	virtual void paintEvent(QPaintEvent* event)
	{
		PlatformDrawContext context(this, event);
		PlatformDraw(context);
	}
};


#endif // PLATFORM_QT_PLATFORM_VIEW_MIXIN_H
