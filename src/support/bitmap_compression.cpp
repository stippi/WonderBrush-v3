
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>

#include <Message.h>

#if USE_LZO
	#include "minilzo.h"
#endif

#include "RenderBuffer.h"

enum {
	COMPRESSION_NONE,
	COMPRESSION_LZO,
	COMPRESSION_ZLIB,
};

#if USE_LZO

static void* lzoWorkerMem = NULL;

// init_lzo
void*
init_lzo()
{
	if (!lzoWorkerMem)
		lzoWorkerMem = malloc(LZO1X_MEM_COMPRESS);
	return lzoWorkerMem;
}

// uninit_lzo
void
uninit_lzo()
{
	if (lzoWorkerMem)
		free(lzoWorkerMem);
}
#endif // USE_LZO

// compress_bitmap_lzo
bool
compress_bitmap_lzo(const RenderBuffer* bitmap, void** buffer, unsigned int* size)
{
	bool result = false;
#if USE_LZO
	if (bitmap) {
		lzo_byte* src = (lzo_byte*)bitmap->Bits();
		lzo_uint srcLength = bitmap->BitsLength();
		*size = srcLength  + (srcLength / 64) + 16 + 3;
		*buffer = malloc(*size);
		if (*buffer && init_lzo()) {
			if (!lzo1x_1_compress(src, srcLength,
								  (lzo_byte*)*buffer,
								  (lzo_uint*)size,
								  (lzo_byte*)lzoWorkerMem)) {
//printf("compressed %d bytes bitmap into %d bytes\n", srcLength, *size);
				if (srcLength  + (srcLength / 64) + 16 + 3 != *size)
					*buffer = realloc(*buffer, *size);
				result = true;
			} else {
				// error compressing
				free(*buffer);
				*buffer = NULL;
				*size = 0;
			}
		} else
			*size = 0;
	}
#endif // USE_LZO
	return result;
}

// decompress_bitmap_lzo
RenderBuffer*
decompress_bitmap_lzo(const void* buffer, unsigned int size, BRect frame)
{
	RenderBuffer* bitmap = NULL;
#if USE_LZO
	if (buffer) {
		bitmap = new RenderBuffer(frame);
		if (bitmap->IsValid() && init_lzo()) {
			lzo_byte* dst = (lzo_byte*)bitmap->Bits();
			lzo_uint dstLength = bitmap->BitsLength();
			if (!lzo1x_decompress((lzo_byte*)buffer,
								  (lzo_uint)size,
								  dst, &dstLength,
								  (lzo_byte*)lzoWorkerMem)) {
//printf("decompressed %d bytes into %d bytes bitmap\n", size, dstLength);
				if (dstLength != (uint32)bitmap->BitsLength()) {
					// error decompressing
					delete bitmap;
					bitmap = NULL;
				}
			}
		} else {
			delete bitmap;
			bitmap = NULL;
		}
	}
#else
	bitmap = new RenderBuffer(frame);
	memset(bitmap->Bits(), 0, bitmap->BitsLength());
#endif
	return bitmap;
}




// compress_bitmap_zlib
bool
compress_bitmap_zlib(const RenderBuffer* bitmap, void** buffer, unsigned* size)
{
	bool result = false;
	if (bitmap) {
		Bytef* src = (Bytef*)bitmap->Bits();
		uLong srcLength = bitmap->BitsLength();
		*size = (unsigned)ceilf(srcLength * 1.01) + 12;
		*buffer = malloc(*size);
		if (*buffer) {
			int ret = compress2((Bytef*)*buffer,
								(uLongf*)size,
								src,
								srcLength,
								3);
			if (ret == Z_OK) {
//printf("zlib compressed %ld bytes bitmap into %d bytes (%f%%)\n", srcLength, *size, ((float)*size / (float)srcLength) * 100.0);
				if ((unsigned)ceilf(srcLength * 1.01) + 12 != *size)
					*buffer = realloc(*buffer, *size);
				result = true;
			} else {
				// error compressing
				free(*buffer);
				*buffer = NULL;
				*size = 0;
				fprintf(stderr, "zlib compression error: %d\n", ret);
			}
		} else
			*size = 0;
	}
	return result;
}

// decompress_bitmap_zlib
RenderBuffer*
decompress_bitmap_zlib(const void* buffer, unsigned int size, BRect frame)
{
	RenderBuffer* bitmap = new RenderBuffer(frame);
	if (bitmap->IsValid()) {
		if (buffer) {
			Bytef* dst = (Bytef*)bitmap->Bits();
			uLongf dstLength = bitmap->BitsLength();

			int ret = uncompress(dst,
								 &dstLength,
								 (const Bytef*)buffer,
								 (uLong)size);
			if (ret != Z_OK || dstLength != (uint32)bitmap->BitsLength()) {
				// decompression error!
				fprintf(stderr, "decompress_bitmap_zlib() failed "
								"- corrupted input buffer or file!\n");
			}
		} else {
			memset(bitmap->Bits(), 0, bitmap->BitsLength());
		}
	} else {
		delete bitmap;
		bitmap = NULL;
	}
	return bitmap;
}



// archive_bitmap
status_t
archive_bitmap(const RenderBuffer* bitmap, BMessage* into, const char* fieldName)
{
	status_t ret = B_BAD_VALUE;
	if (bitmap && bitmap->IsValid() && into) {
		void* buffer;
		unsigned size;
#if USE_LZO
		if (compress_bitmap_lzo(bitmap, &buffer, &size)) {
			ret = into->AddData(fieldName, B_RAW_TYPE, buffer, size);
			if (ret >= B_OK)
				ret = into->AddInt32("compression", COMPRESSION_LZO);
			if (ret >= B_OK)
				ret = into->AddRect("construction bounds", bitmap->Bounds());
		}
#else
		if (compress_bitmap_zlib(bitmap, &buffer, &size)) {
			ret = into->AddData(fieldName, B_RAW_TYPE, buffer, size);
			if (ret >= B_OK)
				ret = into->AddInt32("compression", COMPRESSION_ZLIB);
			if (ret >= B_OK)
				ret = into->AddRect("construction bounds", bitmap->Bounds());
		}
#endif
	}
	return ret;
}

// extract_bitmap
status_t
extract_bitmap(RenderBuffer** bitmap, const BMessage* from, const char* fieldName)
{
	status_t ret = B_BAD_VALUE;
	if (bitmap != NULL && from != NULL) {
		*bitmap = NULL;

		const void* compressedData = NULL;
		ssize_t compressedSize = 0;
		compressedData = NULL;
		compressedSize = 0;
		BRect bounds;
		uint32 compression;
		if (((ret = from->FindData(fieldName,
								  B_RAW_TYPE, &compressedData,
								  &compressedSize)) >= B_OK
			  // this is for backward compatibility
			  || (ret = from->FindData("current compressed data",
								  B_RAW_TYPE, &compressedData,
								  &compressedSize)) >= B_OK)
			&& (ret = from->FindRect("construction bounds",
			   						 &bounds)) >= B_OK) {
		
			// compression defaults to LZO for backward compatibility
			if (from->FindInt32("compression", (int32*)&compression) < B_OK)
				compression = COMPRESSION_LZO;

			switch (compression) {
				case COMPRESSION_LZO:
					*bitmap = decompress_bitmap_lzo(compressedData,
						compressedSize, bounds);
					break;
				case COMPRESSION_ZLIB:
					*bitmap = decompress_bitmap_zlib(compressedData,
						compressedSize, bounds);
					break;
			}
		}
		if (ret < B_OK) {
			delete *bitmap;
			*bitmap = NULL;
		}
	}
	return ret;
}
