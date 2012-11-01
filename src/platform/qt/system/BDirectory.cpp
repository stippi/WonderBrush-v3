#include "BDirectory.h"

#include <string.h>

#include "BEntry.h"


BDirectory::BDirectory()
	:
	fDirectory(NULL),
	fIterator(NULL),
	fInitStatus(B_NO_INIT)
{
}


BDirectory::BDirectory(const char* path)
	:
	fDirectory(NULL),
	fIterator(NULL),
	fInitStatus(B_NO_INIT)
{
	SetTo(path);
}


BDirectory::BDirectory(const BEntry* entry)
	:
	fDirectory(NULL),
	fIterator(NULL),
	fInitStatus(B_NO_INIT)
{
	SetTo(entry);
}


BDirectory::~BDirectory()
{
	Unset();
}


status_t
BDirectory::SetTo(const char* path)
{
	if (strlen(path) == 0)
		return _InitError(B_BAD_VALUE);

	Unset();

	fDirectory = new(std::nothrow) QDir(QString::fromUtf8(path));
	if (fDirectory == NULL)
		return _InitError(B_NO_MEMORY);

	QString absolutePath = fDirectory->canonicalPath();
	if (absolutePath.isEmpty())
		return _InitError(B_ENTRY_NOT_FOUND);
	fDirectory->setPath(absolutePath);

	if (!fDirectory->exists())
		return _InitError(B_ENTRY_NOT_FOUND);

	fIterator = new(std::nothrow) QDirIterator(*fDirectory);
	if (fIterator == NULL)
		return _InitError(B_NO_MEMORY);

	return fInitStatus = B_OK;
}


status_t
BDirectory::SetTo(const BEntry* entry)
{
	if (entry == NULL || entry->InitCheck() != B_OK)
		return _InitError(B_BAD_VALUE);

	return SetTo(entry->PathString().toUtf8().data());
}


void
BDirectory::Unset()
{
	delete fIterator;
	delete fDirectory;

	fIterator = NULL;
	fDirectory = NULL;
	fInitStatus = B_NO_INIT;
}


status_t
BDirectory::GetEntry(BEntry* entry) const
{
	if (entry == NULL)
		return B_BAD_VALUE;

	if (fDirectory == NULL) {
		entry->Unset();
		return B_NO_INIT;
	}

	return entry->SetTo(fDirectory->path().toUtf8().data(), false);
}


status_t
BDirectory::GetNextEntry(BEntry* entry, bool traverse)
{
	if (entry == NULL)
		return B_BAD_VALUE;
	if (fIterator == NULL) {
		entry->Unset();
		return B_NO_INIT;
	}

	QString fileName;
	do {
		if (fIterator->next().isEmpty()) {
			entry->Unset();
			return B_ENTRY_NOT_FOUND;
		}
		fileName = fIterator->fileName();
	} while (fileName == QString::fromUtf8(".")
		|| fileName == QString::fromUtf8(".."));

	return entry->_SetTo(*fDirectory, fileName, traverse);
}


status_t
BDirectory::Rewind()
{
	if (fIterator == NULL)
		return B_NO_INIT;

	QDirIterator* iterator = new(std::nothrow) QDirIterator(*fDirectory);
	if (iterator == NULL)
		return B_NO_MEMORY;

	delete fIterator;
	fIterator = iterator;

	return B_OK;
}


status_t
BDirectory::_InitError(status_t error)
{
	Unset();
	return fInitStatus = error;
}
