// bitmap_compression.h

#ifndef BITMAP_COMPRESSION_H
#define BITMAP_COMPRESSION_H

#include <SupportDefs.h>

class RenderBuffer;
class BMessage;

status_t
archive_bitmap(const RenderBuffer* buffer, BMessage* into, const char* fieldName);

status_t
extract_bitmap(RenderBuffer** buffer, const BMessage* from, const char* fieldName);

#endif // BITMAP_COMPRESSION_H
