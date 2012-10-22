#ifndef PLATFORM_QT_BHANDLER_H
#define PLATFORM_QT_BHANDLER_H


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

protected:
	virtual	void				customEvent(QEvent* event);

private:
			void				HandlerDeleted()
									{ fHandler = NULL; }

private:
			friend class BHandler;

private:
			BHandler*			fHandler;
};


class BHandler
{
public:
			typedef QSharedPointer<BHandlerProxy> ProxyPointer;

public:
								BHandler(const char* name = NULL);
	virtual						~BHandler();

			const char*			Name() const
									{ return fName; }

	virtual	void				MessageReceived(BMessage* message);

			ProxyPointer		Proxy() const
									{ return fProxy; }

			void				ObjectConstructed(QObject* object);
			void				ObjectAboutToBeDestroyed(QObject* object);

private:
			char*				fName;
			ProxyPointer		fProxy;
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

	~PlatformWidgetHandler()
	{
		ObjectAboutToBeDestroyed(this);
	}
};


#endif // PLATFORM_QT_BHANDLER_H
