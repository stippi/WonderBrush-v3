/*
 * Copyright 2002-2009, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef PLATFORM_QT_B_PATH_H
#define PLATFORM_QT_B_PATH_H


#include <StorageDefs.h>


// Forward declarations
class BDirectory;
class BEntry;


class BPath {
public:
								BPath();
								BPath(const BPath& path);
								BPath(const BEntry* entry);
								BPath(const char* dir, const char* leaf = NULL,
									bool normalize = false);
								BPath(const BDirectory* dir,
									const char* leaf = NULL,
									bool normalize = false);

								~BPath();

			status_t			InitCheck() const;

			status_t			SetTo(const BEntry* entry);
			status_t			SetTo(const char* path, const char* leaf = NULL,
									bool normalize = false);
			status_t			SetTo(const BDirectory* dir,
									const char* leaf = NULL,
									bool normalize = false);
			void				Unset();

			status_t			Append(const char* path,
									bool normalize = false);

			const char*			Path() const;
			const char*			Leaf() const;
			status_t			GetParent(BPath* path) const;

			bool				operator==(const BPath& item) const;
			bool				operator==(const char* path) const;
			bool				operator!=(const BPath& item) const;
			bool				operator!=(const char* path) const;
			BPath&				operator=(const BPath& item);
			BPath&				operator=(const char* path);

private:
			status_t			_InitError(status_t error);
			status_t			_SetPath(const char* path);
	static	bool				_MustNormalize(const char* path,
									status_t* _error);

private:
			char*				fName;
			status_t			fCStatus;
};


#endif // PLATFORM_QT_B_PATH_H
