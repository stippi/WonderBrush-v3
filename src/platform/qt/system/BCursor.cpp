#include "BCursor.h"

#include <QBitmap>


BCursor::BCursor(const void* cursorData)
{
	uint8* data = (uint8*)cursorData;
	int size = data[0];
	int depth = data[1];
	int hotspotY = data[2];
	int hotspotX = data[3];
	if (size != 16 || depth != 1 || hotspotX >= 16 || hotspotY >= 16)
		debugger("BCursor::BCursor(): Invalid/unsupported cursor data!");

	QBitmap bitmap(QBitmap::fromData(QSize(size, size), (uchar*)(data + 4),
		QImage::Format_Mono));
	QBitmap mask(QBitmap::fromData(QSize(size, size), (uchar*)(data + 36),
		QImage::Format_Mono));
	fCursor = QCursor(bitmap, mask, hotspotX, hotspotY);
}


BCursor::BCursor(const BCursor& other)
	:
	fCursor(other.fCursor)
{
}


BCursor::BCursor(BCursorID id)
{
	switch (id) {
		case B_CURSOR_ID_SYSTEM_DEFAULT:
			fCursor = QCursor();
			break;
//		case B_CURSOR_ID_CONTEXT_MENU:
		case B_CURSOR_ID_COPY:
			fCursor = QCursor(Qt::DragCopyCursor);
			break;
		case B_CURSOR_ID_CREATE_LINK:
			fCursor = QCursor(Qt::DragLinkCursor);
			break;
		case B_CURSOR_ID_CROSS_HAIR:
			fCursor = QCursor(Qt::CrossCursor);
			break;
		case B_CURSOR_ID_FOLLOW_LINK:
			fCursor = QCursor(Qt::PointingHandCursor);
			break;
		case B_CURSOR_ID_GRAB:
			fCursor = QCursor(Qt::OpenHandCursor);
			break;
		case B_CURSOR_ID_GRABBING:
			fCursor = QCursor(Qt::ClosedHandCursor);
			break;
		case B_CURSOR_ID_HELP:
			fCursor = QCursor(Qt::WhatsThisCursor);
			break;
		case B_CURSOR_ID_I_BEAM:
			fCursor = QCursor(Qt::IBeamCursor);
			break;
//		case B_CURSOR_ID_I_BEAM_HORIZONTAL:
		case B_CURSOR_ID_MOVE:
//			fCursor = QCursor(Qt::DragMoveCursor);
			fCursor = QCursor(Qt::SizeAllCursor);
			break;
//		case B_CURSOR_ID_NO_CURSOR:
		case B_CURSOR_ID_NOT_ALLOWED:
			fCursor = QCursor(Qt::ForbiddenCursor);
			break;
		case B_CURSOR_ID_PROGRESS:
			fCursor = QCursor(Qt::BusyCursor);
				// Or Qt::WaitCursor?
			break;
		case B_CURSOR_ID_RESIZE_NORTH:
			fCursor = QCursor(Qt::SizeVerCursor);
			break;
//		case B_CURSOR_ID_RESIZE_EAST:
//		case B_CURSOR_ID_RESIZE_SOUTH:
//		case B_CURSOR_ID_RESIZE_WEST:
//		case B_CURSOR_ID_RESIZE_NORTH_EAST:
//		case B_CURSOR_ID_RESIZE_NORTH_WEST:
//		case B_CURSOR_ID_RESIZE_SOUTH_EAST:
//		case B_CURSOR_ID_RESIZE_SOUTH_WEST:
		case B_CURSOR_ID_RESIZE_NORTH_SOUTH:
			fCursor = QCursor(Qt::SizeVerCursor);
			break;
		case B_CURSOR_ID_RESIZE_EAST_WEST:
			fCursor = QCursor(Qt::SizeHorCursor);
			break;
		case B_CURSOR_ID_RESIZE_NORTH_EAST_SOUTH_WEST:
			fCursor = QCursor(Qt::SizeBDiagCursor);
			break;
		case B_CURSOR_ID_RESIZE_NORTH_WEST_SOUTH_EAST:
			fCursor = QCursor(Qt::SizeFDiagCursor);
			break;
//		case B_CURSOR_ID_ZOOM_IN:
//		case B_CURSOR_ID_ZOOM_OUT:
		default:
			debugger("BCursor::BCursor(): Unsupported cursor ID!");
			break;
	}
}


BCursor::~BCursor()
{
}


BCursor&
BCursor::operator=(const BCursor& other)
{
	fCursor = other.fCursor;
	return *this;
}
