#include "PlatformMimeDataManager.h"

#include <GraphicsDefs.h>
#include <Message.h>
#include <String.h>

#include <QMimeData>

#include "AutoDeleter.h"


PlatformMimeDataManager::PlatformMimeDataManager()
	:
	fMutex(),
	fMimeDataMap()
{
}


/*static*/ PlatformMimeDataManager*
PlatformMimeDataManager::Manager()
{
	static PlatformMimeDataManager* manager = new PlatformMimeDataManager;
	return manager;
}


QMimeData*
PlatformMimeDataManager::CreateMimeData(BMessage* message)
{
	ObjectDeleter<BMessage> messageDeleter(message);

	QMimeData* mimeData = new(std::nothrow) QMimeData;
	if (mimeData == NULL)
		return NULL;
	ObjectDeleter<QMimeData> mimeDataDeleter(mimeData);

	bool mimeDataSet = false;

	const void* plainTextData;
	ssize_t plainTextDataSize;
	if (message->FindData("text/plain", B_MIME_TYPE, &plainTextData,
			&plainTextDataSize) == B_OK) {
		BString plainText((const char*)plainTextData, plainTextDataSize);
		mimeData->setText(plainText);
		mimeDataSet = true;
	}

	const void* colorData;
	ssize_t colorDataSize;
	if (message->FindData("RGBColor", B_RGB_COLOR_TYPE, &colorData,
			&colorDataSize) == B_OK
		&& colorDataSize == sizeof(rgb_color)) {
		rgb_color color;
		memcpy(&color, colorData, sizeof(color));
		mimeData->setColorData(QColor(color));
		mimeDataSet = true;
	}

// TODO: Translate more!

	if (!mimeDataSet)
		return NULL;

	if (!connect(mimeData, SIGNAL(destroyed(QObject*)),
			SLOT(_MimeDataDeleted(QObject*)))) {
		return NULL;
	}

	QMutexLocker mutexLocker(&fMutex);
	fMimeDataMap.insert(mimeData, message);

	messageDeleter.Detach();
	return mimeDataDeleter.Detach();
}


BMessage*
PlatformMimeDataManager::MessageFor(const QMimeData* mimeData)
{
	if (mimeData == NULL)
		return NULL;

	QMutexLocker mutexLocker(&fMutex);

	BMessage* message = fMimeDataMap.value(mimeData);
	if (message != NULL)
		return message;

	mutexLocker.unlock();

// TODO: create message!
	return NULL;
}


void
PlatformMimeDataManager::_MimeDataDeleted(QObject* object)
{
	QMutexLocker mutexLocker(&fMutex);
	if (BMessage* message = fMimeDataMap.value(object)) {
		fMimeDataMap.remove(object);
		mutexLocker.unlock();
		delete message;
	}
}
