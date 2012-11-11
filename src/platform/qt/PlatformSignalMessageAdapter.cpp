#include "PlatformSignalMessageAdapter.h"


struct PlatformSignalMessageAdapter::SignalSource {
	SignalSource(QObject* sender, const BString& signal)
		:
		fSender(sender),
		fSignal(signal)
	{
	}

	bool Connect(QObject* receiver, const char* slot) const
	{
		return receiver->connect(fSender, fSignal.String(), slot);
	}

	void Disconnect(QObject* receiver, const char* slot) const
	{
		fSender->disconnect(fSignal.String(), receiver, slot);
	}

private:
	QObject*	fSender;
	BString		fSignal;
};


PlatformSignalMessageAdapter::PlatformSignalMessageAdapter(QObject* parent)
	:
	QObject(parent),
	fSources(),
	fTarget(),
	fMessage(),
	fSuspended(0)
{
}


PlatformSignalMessageAdapter::~PlatformSignalMessageAdapter()
{
}


bool
PlatformSignalMessageAdapter::Connect(QObject* sender, const char* signal,
	const BMessenger& target, const BMessage& message)
{
	DisconnectAll();
	SetTarget(target);
	SetMessage(message);
	return AddSource(sender, signal);
}


void
PlatformSignalMessageAdapter::DisconnectAll()
{
	foreach (const SignalSource& source, fSources)
		source.Disconnect(this, SLOT(_SendMessage()));
	fSources.clear();
}


bool
PlatformSignalMessageAdapter::AddSource(QObject* sender, const char* signal)
{
	if (sender == NULL || signal == NULL)
		return false;

	SignalSource source(sender, signal);
	if (!source.Connect(this, SLOT(_SendMessage())))
		return false;

	fSources.append(source);
	return true;
}


void
PlatformSignalMessageAdapter::SetTarget(const BMessenger& target)
{
	fTarget = target;
}


void
PlatformSignalMessageAdapter::SetMessage(const BMessage& message)
{
	fMessage = message;
}


void
PlatformSignalMessageAdapter::Suspend()
{
	fSuspended++;
}


void
PlatformSignalMessageAdapter::Resume()
{
	if (fSuspended > 0)
		fSuspended--;
}


void
PlatformSignalMessageAdapter::_SendMessage()
{
	if (fSuspended == 0)
		fTarget.SendMessage(&fMessage);
}
