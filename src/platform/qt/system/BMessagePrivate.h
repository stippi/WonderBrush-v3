/*
 * Copyright 2005-2010, Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Lotz <mmlr@mlotz.ch>
 */
#ifndef _MESSAGE_PRIVATE_H_
#define _MESSAGE_PRIVATE_H_

#include <Message.h>

#include <MessageUtils.h>


#define MESSAGE_BODY_HASH_TABLE_SIZE	5
#define MAX_DATA_PREALLOCATION			40960
#define MAX_FIELD_PREALLOCATION			50


static const int32 kPortMessageCode = 'pjpp';


enum {
	MESSAGE_FLAG_VALID = 0x0001,
	MESSAGE_FLAG_REPLY_REQUIRED = 0x0002,
	MESSAGE_FLAG_REPLY_DONE = 0x0004,
	MESSAGE_FLAG_IS_REPLY = 0x0008,
	MESSAGE_FLAG_WAS_DELIVERED = 0x0010,
	MESSAGE_FLAG_HAS_SPECIFIERS = 0x0020,
	MESSAGE_FLAG_WAS_DROPPED = 0x0040,
	MESSAGE_FLAG_PASS_BY_AREA = 0x0080
};


enum {
	FIELD_FLAG_VALID = 0x0001,
	FIELD_FLAG_FIXED_SIZE = 0x0002,
};


struct BMessage::field_header {
	uint16		flags;
	uint16		name_length;
	type_code	type;
	uint32		count;
	uint32		data_size;
	uint32		offset;
	int32		next_field;
} _PACKED;


struct BMessage::message_header {
	uint32		format;
	uint32		what;
	uint32		flags;

	int32		target;
	int32		current_specifier;
	area_id		message_area;

	// reply info
	port_id		reply_port;
	int32		reply_target;
	team_id		reply_team;

	// body info
	uint32		data_size;
	uint32		field_count;
	uint32		hash_table_size;
	int32		hash_table[MESSAGE_BODY_HASH_TABLE_SIZE];

	/*	The hash table does contain indexes into the field list and
		not direct offsets to the fields. This has the advantage
		of not needing to update offsets in two locations.
		The hash table must be reevaluated when we remove a field
		though.
	*/
} _PACKED;


class BMessage::Private {
	public:
		Private(BMessage *msg)
			:
			fMessage(msg)
		{
		}

		Private(BMessage &msg)
			:
			fMessage(&msg)
		{
		}

		int32
		GetTarget()
		{
			return fMessage->fHeader->target;
		}

		bool
		UsePreferredTarget()
		{
			return fMessage->fHeader->target == B_PREFERRED_TOKEN;
		}

		void
		SetWasDropped(bool wasDropped)
		{
			if (wasDropped)
				fMessage->fHeader->flags |= MESSAGE_FLAG_WAS_DROPPED;
			else
				fMessage->fHeader->flags &= ~MESSAGE_FLAG_WAS_DROPPED;
		}

		status_t
		Clear()
		{
			return fMessage->_Clear();
		}

		status_t
		InitHeader()
		{
			return fMessage->_InitHeader();
		}

		BMessage::message_header*
		GetMessageHeader()
		{
			return fMessage->fHeader;
		}

		BMessage::field_header*
		GetMessageFields()
		{
			return fMessage->fFields;
		}

		uint8*
		GetMessageData()
		{
			return fMessage->fData;
		}

		void*
		ArchivingPointer()
		{
			return fMessage->fArchivingPointer;
		}

		void
		SetArchivingPointer(void* pointer)
		{
			fMessage->fArchivingPointer = pointer;
		}

	private:
		BMessage* fMessage;
};

#endif	// _MESSAGE_PRIVATE_H_
