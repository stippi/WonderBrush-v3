/*
 * Copyright 2002-2010, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef PLATFORM_QT_B_ENTRY_H
#define PLATFORM_QT_B_ENTRY_H


#include <SupportDefs.h>

#include <QDir>


class BDirectory;
class BPath;


class BEntry {
public:
								BEntry();
								BEntry(const BDirectory* dir, const char* path,
									bool traverse = false);
								BEntry(const char* path, bool traverse = false);
								BEntry(const BEntry& entry);
								~BEntry();

			status_t			InitCheck() const
									{ return fInitStatus; }
			bool				Exists() const;

			// originally in BStatable
			bool				IsDirectory() const;

//			status_t			GetStat(struct stat* stat) const;

			status_t			SetTo(const BDirectory* dir, const char* path,
								   bool traverse = false);
			status_t			SetTo(const char* path, bool traverse = false);
			void				Unset();

			status_t			GetPath(BPath* path) const;
			status_t			GetParent(BEntry* entry) const;
			status_t			GetParent(BDirectory* dir) const;
			status_t			GetName(char* buffer) const;

			status_t			Rename(const char* path, bool clobber = false);
			status_t			MoveTo(BDirectory* dir, const char* path = NULL,
									bool clobber = false);
			status_t			Remove();

			bool				operator==(const BEntry& item) const;
			bool				operator!=(const BEntry& item) const;

			BEntry&				operator=(const BEntry& item);

			QString				PathString() const;

private:
			friend class BDirectory;
			friend class BPath;

private:
			status_t			_InitError(status_t error);
			status_t			_SetTo(const QDir& directory,
									const QString& fileName,
									bool traverse);

private:
			QDir*				fDirectory;
			QString				fFileName;
			status_t			fInitStatus;
};


#endif // PLATFORM_QT_B_ENTRY_H
