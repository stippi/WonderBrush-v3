#ifndef CANVAS_VIEW_H
#define CANVAS_VIEW_H


#include "StateView.h"
#include "Scrollable.h"


class BMessageRunner;
class Document;
class RenderManager;

enum {
	MSG_ZOOM_SET		= 'zmst',
	MSG_ZOOM_IN			= 'zmin',
	MSG_ZOOM_OUT		= 'zmot',
	MSG_ZOOM_ORIGINAL	= 'zmor',
	MSG_ZOOM_TO_FIT		= 'zmft'
};

class CanvasView : public StateView, public Scrollable {
public:
								CanvasView(BRect frame,
									Document* document,
									RenderManager* manager);

#ifdef __HAIKU__
								CanvasView(Document* document,
									RenderManager* manager);
#endif // __HAIKU__

	virtual						~CanvasView();

	// BackBufferedStateView interface
	virtual	void				MessageReceived(BMessage* message);
	virtual	void				AttachedToWindow();
	virtual	void				FrameResized(float width, float height);
	virtual	void				GetPreferredSize(float* _width,
									float* _height);
#ifdef __HAIKU__
	virtual	BSize				MaxSize();
#endif
	virtual	void				Draw(BRect updateRect);

	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseUp(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32 transit,
										   const BMessage* dragMessage);

	virtual	bool				MouseWheelChanged(BPoint where,
												  float x, float y);

	virtual	void				ViewStateBoundsChanged();

	virtual	void				ConvertFromCanvas(BPoint* point) const;
	virtual	void				ConvertToCanvas(BPoint* point) const;

	virtual	void				ConvertFromCanvas(BRect* rect) const;
	virtual	void				ConvertToCanvas(BRect* rect) const;

	virtual	float				ZoomLevel() const;

	virtual	void				InvalidateCanvas(const BRect& bounds);

	// Scrollable interface
protected:
	virtual	void				SetScrollOffset(BPoint offset);

	virtual	void				ScrollOffsetChanged(BPoint oldOffset,
													BPoint newOffset);
	virtual	void				VisibleSizeChanged(float oldWidth,
												   float oldHeight,
												   float newWidth,
												   float newHeight);
	// CanvasView
public:
			double				NextZoomInLevel(double zoom) const;
			double				NextZoomOutLevel(double zoom) const;
			void				SetZoomLevel(double zoomLevel,
										 bool mouseIsAnchor = true);

			enum {
				ZOOM_POLICY_ENLARGE_PIXELS	= 0,
				ZOOM_POLICY_VECTOR_SCALE	= 1
			};
			void				SetZoomPolicy(uint32 policy);

			void				SetAutoScrolling(bool scroll);

	// StateView interface
protected:
	virtual	bool				_HandleKeyDown(const StateView::KeyEvent& event,
									BHandler* originalHandler);
	virtual	bool				_HandleKeyUp(const StateView::KeyEvent& event,
									BHandler* originalHandler);

	//CanvasView
private:
			BRect				_CanvasRect() const;
			BRect				_LayoutCanvas();

			void				_SetRenderManagerZoom();

			void				_UpdateToolCursor();

private:
			Document*			fDocument;
			RenderManager*		fRenderManager;

			double				fZoomLevel;
			uint32				fZoomPolicy;

			bool				fSpaceHeldDown;
			bool				fScrollTracking;
			bool				fInScrollTo;
			BPoint				fScrollTrackingStart;
			BPoint				fScrollOffsetStart;
			bool				fDelayedScrolling;

			BMessageRunner*		fAutoScroller;
};


#endif // CANVAS_VIEW_H
