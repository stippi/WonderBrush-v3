#include "BEntry.h"

#include "BDirectory.h"
#include "BPath.h"

#include <QFileInfo>


BEntry::BEntry()
	:
	fDirectory(NULL),
	fFileName(),
	fInitStatus(B_NO_INIT)
{
}


BEntry::~BEntry()
{
	Unset();
}


bool
BEntry::Exists() const
{
	return fDirectory != NULL && fDirectory->exists(fFileName);
}


bool
BEntry::IsDirectory() const
{
	return fDirectory != NULL && QFileInfo(*fDirectory, fFileName).isDir();
}


status_t
BEntry::SetTo(const char* path, bool traverse)
{
	if (path == NULL || strlen(path) == 0)
		return _InitError(B_BAD_VALUE);

	// get the last path component
	QString filePath = QString::fromUtf8(path);
	QString leaf = filePath.section(QLatin1Char('/'), -1, -1,
		QString::SectionSkipEmpty);
	if (leaf.isEmpty()) {
		// This might be the (a) root directory.
		BDirectory directory;
		status_t error = directory.SetTo(path);
		if (error != B_OK)
			return _InitError(error);

		if (!directory.fDirectory->isRoot()) {
			// That shouldn't happen.
			return _InitError(B_BAD_VALUE);
		}

		return _SetTo(*directory.fDirectory, QString(QLatin1Char('.')), false);
	}

	// If the leaf component is not canonical (i.e. '.' or '..'), construct a
	// directory and get its entry.
	if (leaf == QString(QLatin1Char('.')) || leaf == QString::fromUtf8("..")) {
		BDirectory directory;
		status_t error = directory.SetTo(path);
		if (error != B_OK)
			return _InitError(error);

		return directory.GetEntry(this);
	}

	// We've got a regular leaf component.
	QString directoryPath = filePath.left(filePath.lastIndexOf(leaf));
	if (directoryPath.isEmpty()) {
		// relative path without directory component
		return _SetTo(QDir::current(), leaf, traverse);
	}

	// We've got a usable directory path.
	BDirectory directory;
	status_t error = directory.SetTo(directoryPath.toUtf8().data());
	if (error != B_OK)
		return _InitError(error);

	return _SetTo(*directory.fDirectory, leaf, traverse);
}


void
BEntry::Unset()
{
	delete fDirectory;
	fDirectory = NULL;
	fFileName.clear();
	fInitStatus = B_NO_INIT;
}


status_t
BEntry::GetPath(BPath* path) const
{
	if (path == NULL)
		return B_BAD_VALUE;

	if (fDirectory == NULL) {
		path->Unset();
		return B_NO_INIT;
	}

	return path->SetTo(PathString().toUtf8().data());
}


status_t
BEntry::_InitError(status_t error)
{
	Unset();
	return fInitStatus = error;
}


status_t
BEntry::_SetTo(const QDir& directory, const QString& fileName, bool traverse)
{
	Unset();

	fDirectory = new(std::nothrow) QDir(directory);
	if (fDirectory == NULL)
		return _InitError(B_NO_MEMORY);

	fFileName = fileName;

	// traverse, if necessary
	if (traverse && fFileName != QString(QLatin1Char('.'))) {
		QFileInfo fileInfo(*fDirectory, fFileName);
		if (!fileInfo.exists())
			return _InitError(B_ENTRY_NOT_FOUND);
		if (fileInfo.isSymLink()) {
			QString canonicalPath = fileInfo.canonicalFilePath();
			if (canonicalPath.isEmpty())
				return _InitError(B_ENTRY_NOT_FOUND);
			return SetTo(canonicalPath.toUtf8().data(), false);
		}
	}

	return fInitStatus = B_OK;
}


QString
BEntry::PathString() const
{
	if (fDirectory == NULL)
		return QString();

	if (fFileName == QString(QLatin1Char('.')))
		return fDirectory->path();

	return fDirectory->filePath(fFileName);
}
