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
	PlatformViewMixin(const char* name, uint32 flags)
		:
		BaseClass(name, flags)
	{
	}

	PlatformViewMixin(BRect frame, const char* name,
		uint32 resizeMask, uint32 flags)
		:
		BaseClass(frame, name, resizeMask, flags)
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
