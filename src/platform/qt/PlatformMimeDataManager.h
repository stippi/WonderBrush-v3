#ifndef PLATFORM_QT_MIME_DATA_MANAGER_H
#define PLATFORM_QT_MIME_DATA_MANAGER_H


#include <QMap>
#include <QMutex>
#include <QObject>


class BMessage;
class QMimeData;


class PlatformMimeDataManager : public QObject {
	Q_OBJECT

public:
								PlatformMimeDataManager();

	static	PlatformMimeDataManager* Manager();

			QMimeData*			CreateMimeData(BMessage* message);
									// takes over the message (also on error)
			BMessage*			MessageFor(const QMimeData* mimeData);

private slots:
			void				_MimeDataDeleted(QObject* object);

private:
			QMutex				fMutex;
			QMap<const QObject*, BMessage*> fMimeDataMap;
};


#endif // PLATFORM_QT_MIME_DATA_MANAGER_H
