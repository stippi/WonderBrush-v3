/*
 * Copyright 2002-2009, Haiku Inc.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Tyler Dauwalder
 *		Ingo Weinhold, bonefish@users.sf.net
 */

/*!
	\file Path.cpp
	BPath implementation.
*/

#include "BPath.h"

#include <new>

#include <Directory.h>
#include <Entry.h>
#include <StorageDefs.h>
#include <String.h>


using namespace std;


//! Creates an uninitialized BPath object.
BPath::BPath()
	:
	fName(NULL),
	fCStatus(B_NO_INIT)
{
}


/*! Creates a copy of the given BPath object.
	\param path the object to be copied
*/
BPath::BPath(const BPath& path)
	:
	fName(NULL),
	fCStatus(B_NO_INIT)
{
	*this = path;
}


/*!	\brief Creates a BPath object and initializes it to the filesystem entry
	specified by the given BEntry object.
	\param entry the BEntry object
*/
BPath::BPath(const BEntry* entry)
	:
	fName(NULL),
	fCStatus(B_NO_INIT)
{
	SetTo(entry);
}


/*! \brief Creates a BPath object and initializes it to the specified path or
		   path and filename combination.

	\param dir The base component of the pathname. May be absolute or relative.
		   If relative, it is reckoned off the current working directory.
	\param leaf The (optional) leaf component of the pathname. Must be
		   relative. The value of leaf is concatenated to the end of \a dir
		   (a "/" will be added as a separator, if necessary).
	\param normalize boolean flag used to force normalization; normalization
		   may occur even if false (see \ref _MustNormalize).
*/
BPath::BPath(const char* dir, const char* leaf, bool normalize)
	:
	fName(NULL),
	fCStatus(B_NO_INIT)
{
	SetTo(dir, leaf, normalize);
}


/*! \brief Creates a BPath object and initializes it to the specified directory
	 and filename combination.
	\param dir Refers to the directory that provides the base component of the
		   pathname.
	\param leaf The (optional) leaf component of the pathname. Must be
		   relative. The value of leaf is concatenated to the end of \a dir
		   (a "/" will be added as a separator, if necessary).
	\param normalize boolean flag used to force normalization; normalization
		   may occur even if false (see \ref _MustNormalize).
*/
BPath::BPath(const BDirectory* dir, const char* leaf, bool normalize)
	:
	fName(NULL),
	fCStatus(B_NO_INIT)
{
	SetTo(dir, leaf, normalize);
}


//! Destroys the BPath object and frees any of its associated resources.
BPath::~BPath()
{
	Unset();
}


/*! \brief Returns the status of the most recent construction or SetTo() call.
	\return \c B_OK, if the BPath object is properly initialized, an error
			code otherwise.
*/
status_t
BPath::InitCheck() const
{
	return fCStatus;
}


/*! \brief Reinitializes the object to the specified filesystem entry.
	\param entry the BEntry
	\return
	- \c B_OK: The initialization was successful.
	- \c B_BAD_VALUE: \c NULL \a entry.
	- \c B_NAME_TOO_LONG: The pathname is longer than \c B_PATH_NAME_LENGTH.
	- other error codes.
*/
status_t
BPath::SetTo(const BEntry* entry)
{
	Unset();
	if (entry == NULL)
		return B_BAD_VALUE;

	fCStatus = entry->InitCheck();
	if (fCStatus != B_OK)
		return fCStatus;

	return SetTo(entry->PathString().toUtf8().data());
}


/*!	\brief Reinitializes the object to the specified path or path and file
	name combination.
	\param path the path name
	\param leaf the leaf name (may be \c NULL)
	\param normalize boolean flag used to force normalization; normalization
		   may occur even if false (see \ref _MustNormalize).
	\return
	- \c B_OK: The initialization was successful.
	- \c B_BAD_VALUE: \c NULL \a path or absolute \a leaf.
	- \c B_NAME_TOO_LONG: The pathname is longer than \c B_PATH_NAME_LENGTH.
	- other error codes.
	\note \code path.SetTo(path.Path(), "new leaf") \endcode is safe.
*/
status_t
BPath::SetTo(const char* path, const char* leaf, bool normalize)
{
	if (path == NULL)
		return _InitError(B_BAD_VALUE);
	if (leaf != NULL && QDir::isAbsolutePath(QString::fromUtf8(leaf)))
		return _InitError(B_BAD_VALUE);

	// we always normalize relative paths
	normalize |= QDir::isRelativePath(QString::fromUtf8(path));

	// build a new path from path and leaf
	char newPath[B_PATH_NAME_LENGTH];

	// copy path first
	uint32 pathLen = strlen(path);
	if (pathLen >= sizeof(newPath))
		return _InitError(B_NAME_TOO_LONG);

	strcpy(newPath, path);

	// append leaf, if supplied
	if (leaf != NULL) {
		bool needsSeparator = (pathLen > 0 && path[pathLen - 1] != '/');
		uint32 wholeLen = pathLen + (needsSeparator ? 1 : 0) + strlen(leaf);
		if (wholeLen >= sizeof(newPath))
			return _InitError(B_NAME_TOO_LONG);

		if (needsSeparator) {
			newPath[pathLen] = '/';
			pathLen++;
		}
		strcpy(newPath + pathLen, leaf);
	}

	// check, if necessary to normalize
	if (!normalize) {
		status_t error = B_OK;
		normalize = _MustNormalize(newPath, &error);
		if (error != B_OK)
			return _InitError(error);
	}

	// normalize the path, if necessary, otherwise just set it
	status_t error;
	if (normalize) {
		// create a BEntry and initialize us with this entry
		BEntry entry;
		error = entry.SetTo(newPath, false);
		if (error == B_OK)
			return SetTo(&entry);
	} else
		error = _SetPath(newPath);

	// cleanup, if something went wrong
	if (error != B_OK)
		Unset();
	fCStatus = error;
	return error;
}


/*!	\brief Reinitializes the object to the specified directory and relative
	path combination.
	\param dir Refers to the directory that provides the base component of the
		   pathname.
	\param path the relative path name (may be \c NULL)
	\param normalize boolean flag used to force normalization; normalization
		   may occur even if false (see \ref _MustNormalize).
	\return
	- \c B_OK: The initialization was successful.
	- \c B_BAD_VALUE: \c NULL \a dir or absolute \a path.
	- \c B_NAME_TOO_LONG: The pathname is longer than \c B_PATH_NAME_LENGTH.
	- other error codes.
*/
status_t
BPath::SetTo(const BDirectory* dir, const char* path, bool normalize)
{
	status_t error = (dir && dir->InitCheck() == B_OK ? B_OK : B_BAD_VALUE);
	// get the path of the BDirectory
	BEntry entry;
	if (error == B_OK)
		error = dir->GetEntry(&entry);
	BPath dirPath;
	if (error == B_OK)
		error = dirPath.SetTo(&entry);
	// let the other version do the work
	if (error == B_OK)
		error = SetTo(dirPath.Path(), path, normalize);
	if (error != B_OK)
		Unset();
	fCStatus = error;
	return error;
}


/*!	\brief Returns the object to an uninitialized state. The object frees any
	resources it allocated and marks itself as uninitialized.
*/
void
BPath::Unset()
{
	_SetPath(NULL);
	fCStatus = B_NO_INIT;
}


/*!	\brief Appends the given (relative) path to the end of the current path.
	This call fails if the path is absolute or the object to which you're
	appending is uninitialized.
	\param path relative pathname to append to current path (may be \c NULL).
	\param normalize boolean flag used to force normalization; normalization
		   may occur even if false (see \ref _MustNormalize).
	\return
	- \c B_OK: The initialization was successful.
	- \c B_BAD_VALUE: The object is not properly initialized.
	- \c B_NAME_TOO_LONG: The pathname is longer than \c B_PATH_NAME_LENGTH.
	- other error codes.
*/
status_t
BPath::Append(const char* path, bool normalize)
{
	status_t error = (InitCheck() == B_OK ? B_OK : B_BAD_VALUE);
	if (error == B_OK)
		error = SetTo(Path(), path, normalize);
	if (error != B_OK)
		Unset();
	fCStatus = error;
	return error;
}


/*! \brief Returns the object's complete path name.
	\return
	- the object's path name, or
	- \c NULL, if it is not properly initialized.
*/
const char*
BPath::Path() const
{
	return fName;
}


/*! \brief Returns the leaf portion of the object's path name.
	The leaf portion is defined as the string after the last \c '/'. For
	the root path (\c "/") it is the empty string (\c "").
	\return
	- the leaf portion of the object's path name, or
	- \c NULL, if it is not properly initialized.
*/
const char*
BPath::Leaf() const
{
	if (InitCheck() != B_OK)
		return NULL;

	const char* result = fName + strlen(fName);
	// There should be no need for the second condition, since we deal
	// with absolute paths only and those contain at least one '/'.
	// However, it doesn't harm.
	while (*result != '/' && result > fName)
		result--;
	result++;

	return result;
}


/*! \brief Calls the argument's SetTo() method with the name of the
	object's parent directory.
	No normalization is done.
	\param path the BPath object to be initialized to the parent directory's
		   path name.
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: \c NULL \a path.
	- \c B_ENTRY_NOT_FOUND: The object represents \c "/".
	- other error code returned by SetTo().
*/
status_t
BPath::GetParent(BPath* path) const
{
	if (path == NULL)
		return B_BAD_VALUE;

	status_t error = InitCheck();
	if (error != B_OK)
		return error;

	int32 length = strlen(fName);
	if (length == 1) {
		// handle "/" (path is supposed to be absolute)
		return B_ENTRY_NOT_FOUND;
	}

	char parentPath[B_PATH_NAME_LENGTH];
	length--;
	while (fName[length] != '/' && length > 0)
		length--;
	if (length == 0) {
		// parent dir is "/"
		length++;
	}
	memcpy(parentPath, fName, length);
	parentPath[length] = '\0';

	return path->SetTo(parentPath);
}


/*! \brief Performs a simple (string-wise) comparison of paths.
	No normalization takes place! Uninitialized BPath objects are considered
	to be equal.
	\param item the BPath object to be compared with
	\return \c true, if the path names are equal, \c false otherwise.
*/
bool
BPath::operator==(const BPath& item) const
{
	return *this == item.Path();
}


/*! \brief Performs a simple (string-wise) comparison of paths.
	No normalization takes place!
	\param path the path name to be compared with
	\return \c true, if the path names are equal, \c false otherwise.
*/
bool
BPath::operator==(const char* path) const
{
	return (InitCheck() != B_OK && path == NULL)
		|| (fName != NULL && path != NULL && strcmp(fName, path) == 0);
}


/*! \brief Performs a simple (string-wise) comparison of paths.
	No normalization takes place! Uninitialized BPath objects are considered
	to be equal.
	\param item the BPath object to be compared with
	\return \c true, if the path names are not equal, \c false otherwise.
*/
bool
BPath::operator!=(const BPath& item) const
{
	return !(*this == item);
}


/*! \brief Performs a simple (string-wise) comparison of paths.
	No normalization takes place!
	\param path the path name to be compared with
	\return \c true, if the path names are not equal, \c false otherwise.
*/
bool
BPath::operator!=(const char* path) const
{
	return !(*this == path);
}


/*! \brief Initializes the object to be a copy of the argument.
	\param item the BPath object to be copied
	\return \c *this
*/
BPath&
BPath::operator=(const BPath& item)
{
	if (this != &item)
		*this = item.Path();
	return *this;
}


/*! \brief Initializes the object to be a copy of the argument.
	Has the same effect as \code SetTo(path) \endcode.
	\param path the path name to be assigned to this object
	\return \c *this
*/
BPath&
BPath::operator=(const char* path)
{
	if (path == NULL)
		Unset();
	else
		SetTo(path);
	return *this;
}


status_t
BPath::_InitError(status_t error)
{
	Unset();
	return fCStatus = error;
}


/*!	\brief Sets the supplied path.
	The path is copied. If \c NULL, the object's path is set to NULL as well.
	The object's old path is deleted.
	\param path the path to be set
	\return
	- \c B_OK: Everything went fine.
	- \c B_NO_MEMORY: Insufficient memory.
*/
status_t
BPath::_SetPath(const char* path)
{
	status_t error = B_OK;
	const char* oldPath = fName;
	// set the new path
	if (path) {
		fName = new(nothrow) char[strlen(path) + 1];
		if (fName)
			strcpy(fName, path);
		else
			error = B_NO_MEMORY;
	} else
		fName = NULL;

	// delete the old one
	delete[] oldPath;
	return error;
}


/*! \brief Checks a path to see if normalization is required.

	The following items require normalization:
		- Relative pathnames (after concatenation; e.g. "boot/ltj")
		- The presence of "." or ".." ("/boot/ltj/../ltj/./gwar")
		- Redundant slashes ("/boot//ltj")
		- A trailing slash ("/boot/ltj/")

	\param _error A pointer to an error variable that will be set if the input
		is not a valid path.
	\return
		- \c true: \a path requires normalization
		- \c false: \a path does not require normalization
*/
bool
BPath::_MustNormalize(const char* path, status_t* _error)
{
	// Check for useless input
	if (path == NULL || path[0] == 0) {
		if (_error != NULL)
			*_error = B_BAD_VALUE;
		return false;
	}

	QString pathString = QString::fromUtf8(path);
	return QDir::isRelativePath(pathString)
		|| QDir::cleanPath(pathString) != pathString;
}


/*!
	\var char *BPath::fName
	\brief Pointer to the object's path name string.
*/

/*!
	\var status_t BPath::fCStatus
	\brief The object's initialization status.
*/
