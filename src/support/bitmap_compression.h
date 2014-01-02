// bitmap_compression.h

#ifndef BITMAP_COMPRESSION_H
#define BITMAP_COMPRESSION_H

#include <SupportDefs.h>

class BBitmap;
class BMessage;
class RenderBuffer;

status_t
archive_buffer(const RenderBuffer* buffer, BMessage* into, const char* fieldName);

status_t
extract_buffer(RenderBuffer** buffer, const BMessage* from, const char* fieldName);

status_t
archive_bitmap(const BBitmap* bitmap, BMessage* into, const char* fieldName);

status_t
extract_bitmap(BBitmap** bitmap, const BMessage* from, const char* fieldName);

#endif // BITMAP_COMPRESSION_H
