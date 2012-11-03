/*
 * Copyright 2012 Ingo Weinhold <ingo_weinhold@gmx.de>
 * Copyright 2012 Stephan Aßmus <superstippi@gmx.de>
 */

#include <typeinfo>

#include <dlfcn.h>
#include <map>
#include <stdio.h>
#include <stdlib.h>

#include <Autolock.h>
#include <Locker.h>
#include <image.h>
#include <String.h>

struct StackFrame {
	StackFrame*		previous;
	void*			returnAddress;
};

static BLocker sPrintLock("print stack trace locker");

typedef std::map<addr_t, BString> SymbolMap;

static SymbolMap sSymbolMap;


static bool
init_symbol_map()
{
	system("objdump -D --demangle generated/distro-haiku/WonderBrush "
		" | egrep \"^\\w\" > /boot/home/Desktop/dump");

	FILE* file = fopen("/boot/home/Desktop/dump", "r");

	char buffer[65535];
	while (fgets(buffer, sizeof(buffer), file) != NULL) {
		addr_t address;
		char symbol[65535];
		if (sscanf(buffer, "%zx <%s>:", &address, symbol) == 2) {
			sSymbolMap[address] = symbol;
		}
	}

	return true;
}

static BString
look_up_symbol(addr_t address)
{
	static bool initialized = init_symbol_map();

	BString bestMatch;
	addr_t bestMatchAddress = 0;

	for (SymbolMap::iterator it = sSymbolMap.begin();
			it != sSymbolMap.end(); ++it) {
		addr_t symbolAddress = it->first;
		if (address >= symbolAddress) {
			if (bestMatchAddress == 0
				|| address - bestMatchAddress > address - symbolAddress) {
				bestMatch = it->second;
				bestMatchAddress = symbolAddress;
			}
		}
	}

	return bestMatch;
}

static bool
find_image(void* address, image_info& info)
{
	int32 cookie = 0;
	while (get_next_image_info(B_CURRENT_TEAM, &cookie, &info) == B_OK) {
		if (address >= info.text && address < (char*)info.text + info.text_size)
			return true;
	}

	return false;
}


static BString
get_obj_dump_file()
{
	system("objdump -D --demangle generated/distro-haiku/WonderBrush "
		"> /boot/home/Desktop/dump");
	return "/boot/home/Desktop/dump";
}


void
print_stack_trace()
{
	static BString objDumpFile = get_obj_dump_file();

	printf("  Stack trace [%ld]:\n", find_thread(NULL));
	StackFrame* frame = (StackFrame*)get_stack_frame();
	while (frame != NULL && frame->returnAddress != NULL) {
		image_info info;
		if (find_image(frame->returnAddress, info)
			&& info.type == B_APP_IMAGE) {
			addr_t address = (addr_t)frame->returnAddress - (addr_t)info.text;

			BString match = look_up_symbol(address);
			if (!match.IsEmpty())
				printf("    %p (%s)\n", frame->returnAddress, match.String());
			else
				printf("    %p (?)\n", frame->returnAddress);
		} else {
			Dl_info info;
			const char* name = NULL;
			if (dladdr(frame->returnAddress, &info) != 0) {
				name = info.dli_sname;
			}
			printf("    %p (%s)\n", frame->returnAddress, name);
		}
		frame = frame->previous;
	}
}

BLocker&
get_stack_trace_locker()
{
	return sPrintLock;
}
