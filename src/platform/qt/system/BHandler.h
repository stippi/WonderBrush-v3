#ifndef PLATFORM_QT_BHANDLER_H
#define PLATFORM_QT_BHANDLER_H


#include <Archivable.h>
#include <String.h>

#include <QObject>
#include <QPointer>
#include <QSharedPointer>


class BHandler;
class BMessage;


class BHandlerProxy : public QObject {
	Q_OBJECT

public:
	explicit					BHandlerProxy(BHandler* handler);
								~BHandlerProxy();

			BHandler*			Handler() const
									{ return fHandler; }
			int32				Token() const;

			// conceptually private
			void				HandlerDeleted()
									{ fHandler = NULL; }

protected:
	virtual	void				customEvent(QEvent* event);

private:
			friend class BHandler;

private:
			BHandler*			fHandler;
};


class BHandler : public BArchivable
{
public:
			typedef QSharedPointer<BHandlerProxy> ProxyPointer;

public:
								BHandler(BMessage* archive);
								BHandler(const char* name = NULL);
	virtual						~BHandler();

			const char*			Name() const
									{ return fName; }
			void				SetName(const char* name);

	virtual	void				MessageReceived(BMessage* message);

	virtual	void				SendNotices(uint32 what,
									const BMessage* notice = NULL);
			bool				IsWatched() const;

			ProxyPointer		Proxy() const
									{ return fProxy; }
			int32				Token() const
									{ return fToken; }
	static	ProxyPointer		ProxyForToken(int32 token);

			// conceptually private
			void				SetToken(int32 token)
									{ fToken = token; }

			void				ObjectConstructed(QObject* object);
			void				ObjectAboutToBeDestroyed(QObject* object);

private:
			char*				fName;
			ProxyPointer		fProxy;
			int32				fToken;
};


template<typename BaseClass>
class PlatformObjectHandler : public BaseClass, public BHandler {
public:
	PlatformObjectHandler(QObject* parent = NULL)
		:
		BaseClass(parent),
		BHandler()
	{
		ObjectConstructed(this);
	}

	PlatformObjectHandler(const char* name, QObject* parent = NULL)
		:
		BaseClass(parent),
		BHandler(name)
	{
		ObjectConstructed(this);
	}

	~PlatformObjectHandler()
	{
		ObjectAboutToBeDestroyed(this);
	}
};


template<typename BaseClass>
class PlatformWidgetHandler : public BaseClass, public BHandler {
public:
	PlatformWidgetHandler(QWidget* parent = NULL)
		:
		BaseClass(parent),
		BHandler()
	{
		ObjectConstructed(this);
	}

	PlatformWidgetHandler(const char* name, QWidget* parent = NULL)
		:
		BaseClass(parent),
		BHandler(name)
	{
		ObjectConstructed(this);
	}

	PlatformWidgetHandler(BMessage* archive)
		:
		BaseClass(),
		BHandler(archive)
	{
		ObjectConstructed(this);
	}

	~PlatformWidgetHandler()
	{
		ObjectAboutToBeDestroyed(this);
	}
};


#endif // PLATFORM_QT_BHANDLER_H
