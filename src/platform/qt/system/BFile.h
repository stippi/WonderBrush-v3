#ifndef PLATFORM_QT_FILE_H
#define PLATFORM_QT_FILE_H


#include <DataIO.h>
#include <StorageDefs.h>

#include <QFile>


class BFile : public BPositionIO {
public:
								BFile();
								BFile(const char* path, uint32 openMode);
	virtual						~BFile();

			status_t			InitCheck() const
									{ return fInitStatus; }

			status_t			SetTo(const char* path, uint32 openMode);
			status_t			SetTo(const QString& path, uint32 openMode);
			void				Unset();

	virtual	ssize_t				Read(void* buffer, size_t size);
	virtual	ssize_t				Write(const void* buffer, size_t size);

	virtual	ssize_t				ReadAt(off_t position, void* buffer,
									size_t size);
	virtual	ssize_t				WriteAt(off_t position, const void* buffer,
									size_t size);

	virtual	off_t				Seek(off_t position, uint32 seekMode);
	virtual	off_t				Position() const;

	virtual	status_t			SetSize(off_t size);
	virtual	status_t			GetSize(off_t* size) const;

private:
			status_t			_FileError(bool unset);

private:
			QFile*				fFile;
			status_t			fInitStatus;
};



#endif	// PLATFORM_QT_FILE_H
