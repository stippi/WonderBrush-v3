/*
 * Copyright 2006-2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef PATH_H
#define PATH_H


#include <agg_path_storage.h>

#include <Archivable.h>
#include <List.h>
#include <Rect.h>
#include <String.h>

#include "BaseObject.h"
#include "Transformable.h"

class BMessage;


class Path : public BArchivable, public BaseObject {
public:

	class Listener {
	public:
								Listener();
		virtual					~Listener();

		virtual	void			PointAdded(const Path* path, int32 index) = 0;
		virtual	void			PointRemoved(const Path* path, int32 index) = 0;
		virtual	void			PointChanged(const Path* path, int32 index) = 0;
		virtual	void			PathChanged(const Path* path) = 0;
		virtual	void			PathClosedChanged(const Path* path) = 0;
		virtual	void			PathReversed(const Path* path) = 0;
	};

	class Iterator {
	public:
								Iterator() {}
		virtual					~Iterator() {}

		virtual	void			MoveTo(BPoint point) = 0;
		virtual	void			LineTo(BPoint point) = 0;
	};

								Path();
								Path(const Path& other);
								Path(const BMessage* archive);

	virtual						~Path();

	// BaseObject
	virtual	BaseObject*			Clone(CloneContext& context) const;
	virtual	const char*			DefaultName() const;

	virtual	status_t			Archive(BMessage* into,
									bool deep = true) const;

	virtual	void				AddProperties(PropertyObject* object,
									uint32 flags = 0) const;
	virtual	bool				SetToPropertyObject(
									const PropertyObject* object,
									uint32 flags = 0);

	// Path
			Path&				operator=(const Path& other);
			bool				operator==(const Path& other) const;
			bool				operator!=(const Path& other) const;

			void				MakeEmpty();

			bool				AddPoint(BPoint point);
			bool				AddPoint(const BPoint& point,
									const BPoint& pointIn,
									const BPoint& pointOut, bool connected);
			bool				AddPoint(BPoint point, int32 index);

			bool				RemovePoint(int32 index);

								// modify existing points position
			bool				SetPoint(int32 index, BPoint point);
			bool				SetPoint(int32 index, BPoint point,
									BPoint pointIn, BPoint pointOut,
									bool connected);
			bool				SetPointIn(int32 index, BPoint point);
			bool				SetPointOut(int32 index, BPoint point,
									bool mirrorDist = false);

			bool				SetInOutConnected(int32 index, bool connected);

								// query existing points position
			bool				GetPointAt(int32 index, BPoint& point) const;
			bool				GetPointInAt(int32 index, BPoint& point) const;
			bool				GetPointOutAt(int32 index, BPoint& point) const;
			bool				GetPointsAt(int32 index, BPoint& point,
									BPoint& pointIn, BPoint& pointOut,
									bool* connected = NULL) const;

			int32				CountPoints() const;

								// iterates over curve segments and returns
								// the distance and index of the point that
								// started the segment that is closest
			bool				GetDistance(BPoint point, float* distance,
									int32* index) const;

								// at curve segment indicated by "index", this
								// function looks for the closest point
								// directly on the curve and returns a "scale"
								// that indicates the distance on the curve
								// between [0..1]
			bool				FindBezierScale(int32 index, BPoint point,
									double* scale) const;
								// this function can be used to get a point
								// directly on the segment indicated by "index"
								// "scale" is on [0..1] indicating the distance
								// from the start of the segment to the end
			bool				GetPoint(int32 index, double scale,
									BPoint& point) const;

			void				SetClosed(bool closed);
			bool				IsClosed() const
									{ return fClosed; }

			BRect				Bounds() const;
			BRect				ControlPointBounds() const;

			void				Iterate(Iterator* iterator,
									float smoothScale = 1.0) const;

			void				CleanUp();
			void				Reverse();
			void				ApplyTransform(const Transformable& transform);

			void				PrintToStream() const;

			bool				GetAGGPathStorage(
									agg::path_storage& path) const;

			bool				AddListener(Listener* listener);
			bool				RemoveListener(Listener* listener);
			int32				CountListeners() const;
			Listener*			ListenerAtFast(int32 index) const;
			void				SuspendNotifications(bool suspend);


private:
	struct control_point {
		BPoint		point;		// actual point on path
		BPoint		point_in;	// control point for incomming curve
		BPoint		point_out;	// control point for outgoing curve
		bool		connected;	// if all 3 points should be on one line
	};

	static	bool				_GetAGGPathStorage(agg::path_storage& path,
									 const control_point* points, int32 count,
									 bool closed);

			BRect				_Bounds() const;
			void				_SetPoint(int32 index, BPoint point);
			void				_SetPoint(int32 index, const BPoint& point,
									const BPoint& pointIn,
									const BPoint& pointOut, bool connected);
			bool				_SetPointCount(int32 count);

			bool				_DelayNotification();

			void				_NotifyPointAdded(int32 index);
			void				_NotifyPointChanged(int32 index);
			void				_NotifyPointRemoved(int32 index);
			void				_NotifyPathChanged();
			void				_NotifyClosedChanged();
			void				_NotifyPathReversed();

private:
			BList				fListeners;
			int32				fNotificationsSuspended;
			bool				fNotificationsPending;

			control_point*		fPath;
			bool				fClosed;

			int32				fPointCount;
			int32				fAllocCount;

	mutable	BRect				fCachedBounds;
};

typedef Reference<Path>	PathRef;

#endif	// PATH_H
