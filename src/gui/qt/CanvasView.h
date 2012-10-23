#ifndef PLATFORM_QT_CANVAS_VIEW_H
#define PLATFORM_QT_CANVAS_VIEW_H


#include <Handler.h>
#include <Rect.h>

#include <QBrush>

#include "PlatformScrollArea.h"


class Document;
class RenderManager;


enum {
	MSG_ZOOM_SET		= 'zmst',
	MSG_ZOOM_IN			= 'zmin',
	MSG_ZOOM_OUT		= 'zmot',
	MSG_ZOOM_ORIGINAL	= 'zmor',
	MSG_ZOOM_TO_FIT		= 'zmft'
};


class CanvasView : public PlatformWidgetHandler<PlatformScrollArea>
{
	Q_OBJECT

public:
			enum {
				ZOOM_POLICY_ENLARGE_PIXELS	= 0,
				ZOOM_POLICY_VECTOR_SCALE	= 1
			};

public:
	explicit					CanvasView(QWidget* parent = 0);

			void				Init(Document* document,
									RenderManager* manager);

	virtual	void				MessageReceived(BMessage* message);

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

public:
			double				NextZoomInLevel(double zoom) const;
			double				NextZoomOutLevel(double zoom) const;
			void				SetZoomLevel(double zoomLevel,
										 bool mouseIsAnchor = true);

			void				SetZoomPolicy(uint32 policy);

			void				SetAutoScrolling(bool scroll);

protected:
	virtual	void				paintEvent(QPaintEvent* event);

private:
			BRect				_CanvasRect() const;
			BRect				_LayoutCanvas();
			void				_SetRenderManagerZoom();

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

			QBrush				fStripesBrush;
};


#endif // PLATFORM_QT_CANVAS_VIEW_H
