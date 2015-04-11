/*
 * Copyright 2012-2015, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef PATH_POINT_H
#define PATH_POINT_H

#include "HashSetHugo.h"
#include "Path.h"

enum {
	POINT		= 1 << 0,
	POINT_IN	= 1 << 1,
	POINT_OUT	= 1 << 2,

	POINT_ALL	= POINT | POINT_IN | POINT_OUT
};

class PathPoint {
public:
	PathPoint()
		: fPath(NULL)
		, fIndex(-1)
		, fWhich(0)
	{
	}

	PathPoint(Path* path, int32 index, int32 which)
		: fPath(path)
		, fIndex(index)
		, fWhich(which)
	{
	}

	PathPoint(const PathPoint& other)
		: fPath(other.fPath)
		, fIndex(other.fIndex)
		, fWhich(other.fWhich)
	{
	}

	bool operator==(const PathPoint& other) const
	{
		return IsSameIndex(other) && fWhich == other.fWhich;
	}

	bool operator!=(const PathPoint& other) const
	{
		return !(*this == other);
	}

	PathPoint& operator=(const PathPoint& other)
	{
		fPath = other.fPath;
		fIndex = other.fIndex;
		fWhich = other.fWhich;
		return *this;
	}
	
	bool IsValid() const
	{
		return GetPath() != NULL && fIndex >= 0
			&& fIndex < fPath->CountPoints();
	}

	bool IsSameIndex(const PathPoint& other) const
	{
		return fPath == other.fPath && fIndex == other.fIndex;
	}

	Path* GetPath() const
	{
		return fPath.Get();
	}

	int32 GetIndex() const
	{
		return fIndex;
	}

	int32 GetWhich() const
	{
		return fWhich;
	}

	bool GetPoint(BPoint& point) const
	{
		if (GetPath() == NULL)
			return false;
		return fPath->GetPointAt(fIndex, point);
	}

	bool GetPointIn(BPoint& point) const
	{
		if (GetPath() == NULL)
			return false;
		return fPath->GetPointInAt(fIndex, point);
	}

	bool GetPointOut(BPoint& point) const
	{
		if (GetPath() == NULL)
			return false;
		return fPath->GetPointOutAt(fIndex, point);
	}
	
	bool OffsetBy(const BPoint& offset) const
	{
		if (GetPath() == NULL)
			return false;
		
		BPoint point;
		BPoint pointIn;
		BPoint pointOut;
		bool connected;
		if (!fPath->GetPointsAt(fIndex, point, pointIn, pointOut,
			&connected)) {
			return false;
		}
		
		point += offset;
		pointIn += offset;
		pointOut += offset;
		
		return fPath->SetPoint(fIndex, point, pointIn, pointOut,
			connected);
	}

	size_t GetHashCode() const
	{
		return (size_t)GetPath() ^ fIndex ^ (fWhich << 4);
	}

private:
	Reference<Path>	fPath;
	int32			fIndex;
	int32			fWhich;
};

typedef HashSet<PathPoint> PointSelection;

#endif	// PATH_POINT_H
