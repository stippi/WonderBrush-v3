/*
 * Copyright 2002-2006, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef PLATFORM_QT_B_DIRECTORY_H
#define PLATFORM_QT_B_DIRECTORY_H


#include <StorageDefs.h>

#include <QDir>
#include <QDirIterator>


class BEntry;
//class BFile;
//class BSymLink;


class BDirectory {
public:
								BDirectory();
								BDirectory(const BDirectory& other);
								BDirectory(const BEntry* entry);
								BDirectory(const char* path);
								BDirectory(const BDirectory* dir,
									const char* path);
								~BDirectory();

			status_t			InitCheck() const
									{ return fInitStatus; }

			status_t			SetTo(const BEntry* entry);
			status_t			SetTo(const char* path);
			status_t			SetTo(const BDirectory* dir, const char* path);
			void				Unset();

			status_t			GetEntry(BEntry* entry) const;

//			bool				IsRootDirectory() const;

//			status_t			FindEntry(const char* path, BEntry* entry,
//									bool traverse = false) const;

//			bool				Contains(const char* path,
//									int32 nodeFlags = B_ANY_NODE) const;
//			bool				Contains(const BEntry* entry,
//									int32 nodeFlags = B_ANY_NODE) const;

//			status_t			GetStatFor(const char* path,
//									struct stat* st) const;

			status_t			GetNextEntry(BEntry* entry,
									bool traverse = false);
//			int32				GetNextDirents(dirent* buf, size_t bufSize,
//									int32 count = INT_MAX);
			status_t			Rewind();
//			int32				CountEntries();

//			status_t			CreateDirectory(const char* path,
//									BDirectory* dir);
//			status_t			CreateFile(const char* path, BFile* file,
//									bool failIfExists = false);
//			status_t			CreateSymLink(const char* path,
//									const char* linkToPath, BSymLink *link);

//			BDirectory&			operator=(const BDirectory& dir);

private:
			friend class BEntry;
//			friend class BFile;

private:
			status_t			_InitError(status_t error);

private:
			QDir*				fDirectory;
			QDirIterator*		fIterator;
			status_t			fInitStatus;
};


#endif // PLATFORM_QT_B_DIRECTORY_H
