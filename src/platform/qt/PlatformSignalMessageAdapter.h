#ifndef PLATFORM_QT_SIGNAL_MESSAGE_ADAPTER_H
#define PLATFORM_QT_SIGNAL_MESSAGE_ADAPTER_H


#include <Message.h>
#include <Messenger.h>
#include <String.h>

#include <QList>
#include <QObject>


class PlatformSignalMessageAdapter : public QObject {
	Q_OBJECT

public:
								PlatformSignalMessageAdapter(
									QObject* parent = NULL);
								~PlatformSignalMessageAdapter();

			bool				Connect(QObject* sender, const char* signal,
									const BMessenger& target,
									const BMessage& message);
			void				DisconnectAll();

			bool				AddSource(QObject* sender, const char* signal);

			void				SetTarget(const BMessenger& target);
			void				SetMessage(const BMessage& message);

			void				Suspend();
			void				Resume();

private:
			struct SignalSource;

private slots:
			void				_SendMessage();

private:
			QList<SignalSource>	fSources;
			BMessenger			fTarget;
			BMessage			fMessage;
			int32				fSuspended;
};


#endif // PLATFORM_QT_SIGNAL_MESSAGE_ADAPTER_H
