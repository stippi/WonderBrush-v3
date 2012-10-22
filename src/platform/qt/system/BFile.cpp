#include <File.h>

#include <unistd.h>


BFile::BFile()
	:
	fFile(NULL),
	fInitStatus(B_NO_INIT)
{
}


BFile::BFile(const char* path, uint32 openMode)
	:
	fFile(NULL),
	fInitStatus(B_NO_INIT)
{
	SetTo(path, openMode);
}


BFile::~BFile()
{
	Unset();
}


status_t
BFile::SetTo(const char* path, uint32 openMode)
{
	Unset();

	// translate the open mode
	QIODevice::OpenMode qOpenMode;
	switch (openMode & (B_READ_ONLY | B_WRITE_ONLY | B_READ_WRITE)) {
		case B_READ_ONLY:
			qOpenMode = QIODevice::ReadOnly;
			break;
		case B_WRITE_ONLY:
			qOpenMode = QIODevice::WriteOnly;
			break;
		case B_READ_WRITE:
			qOpenMode = QIODevice::ReadWrite;
			break;

		default:
			return fInitStatus = B_BAD_VALUE;
	}

	// We use the POSIX open() function, so that the open mode flags (O_EXCL,
	// O_CREAT) are handled correctly. QFile::open() cannot do that.
	int fd = ::open(path, openMode);
	if (fd < 0)
		return _WONDERBRUSH_TO_NEGATIVE_ERROR(errno);

	// create and open the file
	fFile = new(std::nothrow) QFile(QString::fromUtf8(path));
	if (fFile == NULL) {
		close(fd);
		return fInitStatus = B_NO_MEMORY;
	}

	if (!fFile->open(fd, qOpenMode, QFile::AutoCloseHandle)) {
		status_t error = _FileError(false);
		Unset();
		return fInitStatus = error;
	}

	return fInitStatus = B_OK;
}

status_t
BFile::SetTo(const QString& path, uint32 openMode)
{
	return SetTo(path.toUtf8().data(), openMode);
}


void
BFile::Unset()
{
	delete fFile;
	fFile = NULL;
	fInitStatus = B_NO_INIT;
}


ssize_t
BFile::Read(void* buffer, size_t size)
{
	if (fFile == NULL)
		return B_NO_INIT;

	qint64 result = fFile->read((char*)buffer, size);
	if (result < 0)
		return _FileError(true);

	return result;
}


ssize_t
BFile::Write(const void* buffer, size_t size)
{
	if (fFile == NULL)
		return B_NO_INIT;

	qint64 result = fFile->write((const char*)buffer, size);
	if (result < 0)
		return _FileError(true);

	return result;
}


ssize_t
BFile::ReadAt(off_t position, void* buffer, size_t size)
{
	if (fFile == NULL)
		return B_NO_INIT;

	qint64 oldPosition = fFile->pos();
	if (!fFile->seek(position))
		return _FileError(true);

	ssize_t result = Read(buffer, size);
	fFile->seek(oldPosition);
	return result;
}


ssize_t
BFile::WriteAt(off_t position, const void* buffer, size_t size)
{
	if (fFile == NULL)
		return B_NO_INIT;

	qint64 oldPosition = fFile->pos();
	if (!fFile->seek(position))
		return _FileError(true);

	ssize_t result = Write(buffer, size);
	fFile->seek(oldPosition);
	return result;
}


off_t
BFile::Seek(off_t position, uint32 seekMode)
{
	if (fFile == NULL)
		return B_NO_INIT;

	qint64 oldPosition = fFile->pos();
	qint64 finalPosition;
	switch (seekMode) {
		case SEEK_CUR:
			finalPosition = oldPosition + position;
			break;
		case SEEK_END:
			finalPosition = fFile->size() + position;
			break;
		case SEEK_SET:
			finalPosition = position;
			break;
		default:
			return B_BAD_VALUE;
	}

	if (!fFile->seek(finalPosition))
		return _FileError(true);

	return oldPosition;
}


off_t
BFile::Position() const
{
	if (fFile == NULL)
		return B_NO_INIT;

	return fFile->pos();
}


status_t
BFile::SetSize(off_t size)
{
	if (fFile == NULL)
		return B_NO_INIT;

	return fFile->resize(size) ? B_OK : _FileError(true);
}


status_t
BFile::GetSize(off_t* size) const
{
	if (fFile == NULL)
		return B_NO_INIT;

	*size = fFile->size();
	return B_OK;
}


status_t
BFile::_FileError(bool unset)
{
	if (fFile == NULL)
		return B_NO_INIT;

	QFile::FileError fileError = fFile->error();
	if (unset)
		fFile->unsetError();

	// TODO: The QFile error codes aren't particularly helpful. They mostly
	// describe during what operation the error occurred, not what kind of
	// error occurred.
	switch (fileError) {
		case QFile::TimeOutError:
			return B_BUSY;
		case QFile::PermissionsError:
			return B_PERMISSION_DENIED;
		case QFile::ReadError:
		case QFile::WriteError:
		case QFile::FatalError:
		case QFile::ResourceError:
		case QFile::OpenError:
		case QFile::AbortError:
		case QFile::UnspecifiedError:
		case QFile::RemoveError:
		case QFile::RenameError:
		case QFile::PositionError:
		case QFile::ResizeError:
		case QFile::CopyError:
		case QFile::NoError:
			// Since the caller only invokes this method when an error has
			// happened, we ignore NoError and return an error code.
		default:
			return B_ERROR;
	}
}
