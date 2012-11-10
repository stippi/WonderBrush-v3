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

	virtual void Draw(BRect updateRect)
	{
		PlatformDrawContext context(this, updateRect);
		PlatformDraw(context);
	}
};


#endif // PLATFORM_HAIKU_PLATFORM_VIEW_MIXIN_H
