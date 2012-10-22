#ifndef PLATFORM_QT_CANVAS_VIEW_H
#define PLATFORM_QT_CANVAS_VIEW_H


#include <Handler.h>
#include <Rect.h>

#include <QAbstractScrollArea>
#include <QBrush>


class Document;
class RenderManager;


class CanvasView : public PlatformWidgetHandler<QAbstractScrollArea>
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

protected:
	virtual	void				paintEvent(QPaintEvent* event);
	virtual	void				resizeEvent(QResizeEvent * event);
	virtual	void				scrollContentsBy(int dx, int dy);

private:
			BRect				_CanvasRect() const;
			void				_UpdateScrollPosition();

private:
			Document*			fDocument;
			RenderManager*		fRenderManager;

			double				fZoomLevel;
			uint32				fZoomPolicy;

			QBrush				fStripesBrush;
};


#endif // PLATFORM_QT_CANVAS_VIEW_H
