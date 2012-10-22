/*
 * Copyright 2006, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ingo Weinhold <bonefish@cs.tu-berlin.de>
 */

#include "AbstractLOAdapter.h"

#include "BuildSupport.h"

#include <Handler.h>
#include <Message.h>
#include <Messenger.h>


// constructor
AbstractLOAdapter::AbstractLOAdapter(BHandler* handler)
	: fHandler(handler),
	  fMessenger(NULL)
{
}

// constructor
AbstractLOAdapter::AbstractLOAdapter(const BMessenger& messenger)
	: fHandler(NULL),
	  fMessenger(new BMessenger(messenger))
{
}

// destructor
AbstractLOAdapter::~AbstractLOAdapter()
{
	delete fMessenger;
}

// DeliverMessage
void
AbstractLOAdapter::DeliverMessage(BMessage* message)
{
	if (fHandler != NULL)
		BMessenger(fHandler).SendMessage(message);
	else if (fMessenger != NULL)
		fMessenger->SendMessage(message);
}

// DeliverMessage
void
AbstractLOAdapter::DeliverMessage(BMessage& message)
{
	DeliverMessage(&message);
}

// DeliverMessage
void
AbstractLOAdapter::DeliverMessage(uint32 command)
{
	BMessage message(command);
	DeliverMessage(&message);
}

