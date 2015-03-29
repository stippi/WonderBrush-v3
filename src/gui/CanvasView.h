#ifndef CANVAS_VIEW_H
#define CANVAS_VIEW_H


#include "BuildSupport.h"
#include "Document.h"
#include "StateView.h"
#include "Scrollable.h"


class BMessageRunner;
class RenderManager;


#define CANVAS_VIEW_AUTO_SCROLL_DELAY		40000 // 40 ms
#define CANVAS_VIEW_USE_DELAYED_SCROLLING	0
#define CANVAS_VIEW_USE_NATIVE_SCROLLING	1

// native scrolling is Haiku only
#ifdef WONDERBRUSH_PLATFORM_QT
#	undef CANVAS_VIEW_USE_NATIVE_SCROLLING
#	define CANVAS_VIEW_USE_NATIVE_SCROLLING	0
#endif


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
									EditContext& editContext,
									RenderManager* manager);
								CanvasView(Document* document,
									EditContext& editContext,
									RenderManager* manager);

	virtual						~CanvasView();

	// BackBufferedStateView interface
	virtual	void				MessageReceived(BMessage* message);
	virtual	void				AttachedToWindow();
	virtual	void				FrameResized(float width, float height);
	virtual	void				GetPreferredSize(float* _width,
									float* _height);
	virtual	BSize				MaxSize();
	virtual void				PlatformDraw(PlatformDrawContext& drawContext);

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
									float oldHeight, float newWidth,
									float newHeight);
	// CanvasView
public:
			double				NextZoomInLevel(double zoom) const;
			double				NextZoomOutLevel(double zoom) const;
			void				SetZoomLevel(double zoomLevel,
									bool mouseIsAnchor = true);
			void				SetZoomLevel(double zoomLevel,
									BPoint viewAnchor,
									BPoint canvasAnchor);

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

private:
			class PlatformDelegate;
			friend class DocumentListener;

	//CanvasView
private:
			BRect				_CanvasRect() const;
			BRect				_LayoutCanvas();

			void				_SetRenderManagerZoom();

			void				_UpdateToolCursor();

private:
			PlatformDelegate*	fPlatformDelegate;

			Document*			fDocument;
			Document::Listener*	fDocumentListener;
			RenderManager*		fRenderManager;

			double				fZoomLevel;
			uint32				fZoomPolicy;

			bool				fSpaceHeldDown;
			bool				fScrollTracking;
			bool				fInScrollTo;
			BPoint				fScrollTrackingStart;
			bool				fDelayedScrolling;

			BMessageRunner*		fAutoScroller;
};


#endif // CANVAS_VIEW_H
