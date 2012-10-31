#ifndef PLATFORM_HAIKU_PLATFORM_VIEW_MIXIN_H
#define PLATFORM_HAIKU_PLATFORM_VIEW_MIXIN_H


#include <View.h>


struct PlatformDrawContext {
	PlatformDrawContext(BView* view, const BRect& updateRect)
		:
		fView(view),
		fUpdateRect(updateRect)
	{
	}

	BView* View() const
	{
		return fView;
	}

	const BRect& UpdateRect() const
	{
		return fUpdateRect;
	}


private:
	BView*		fView;
	BRect		fUpdateRect;
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

	virtual void Draw(BRect updateRect)
	{
		PlatformDrawContext context(this, updateRect);
		PlatformDraw(context);
	}
};


#endif // PLATFORM_HAIKU_PLATFORM_VIEW_MIXIN_H
