#include "Cropper.h"

#include <Bitmap.h>
#include <BitmapStream.h>
#include <Directory.h>
#include <File.h>
#include <Rect.h>
#include <TranslationUtils.h>
#include <TranslatorRoster.h>

#include "RenderBuffer.h"
#include "RenderEngine.h"

// constructor
Cropper::Cropper()
	: BApplication("application/x-vnd.yellowbites.Cropper")
	, fTargetWidth(-1)
	, fTargetHeight(-1)
	, fTargetFolder("/boot/home/Desktop")
	, fTargetFormat(B_JPEG_FORMAT)
{
}

// MessageReceived
void
Cropper::MessageReceived(BMessage* message)
{
	switch (message->what) {
		default:
			BApplication::MessageReceived(message);
			break;
	}
}

// ReadyToRun
void
Cropper::ReadyToRun()
{
//	_PrintUsage("Cropper");
	PostMessage(B_QUIT_REQUESTED);
}

// ArgvReceived
void
Cropper::ArgvReceived(int32 argc, char** argv)
{
	int32 i = 1;
	for (; i < argc; i++) {
		if (strcmp(argv[i], "-o") == 0) {
			if (i == argc - 1)
				break;
			fTargetFolder = argv[++i];
		} else if (strcmp(argv[i], "-w") == 0) {
			if (i == argc - 1)
				break;
			fTargetWidth = atoi(argv[++i]);
		} else if (strcmp(argv[i], "-h") == 0) {
			if (i == argc - 1)
				break;
			fTargetHeight = atoi(argv[++i]);
		} else if (strcmp(argv[i], "-f") == 0) {
			if (i == argc - 1)
				break;
			const char* name = argv[++i];
			bool found = false;

			BTranslatorRoster* roster = BTranslatorRoster::Default();
			translator_id* translatorIDs;
			int32 idCount;
			roster->GetAllTranslators(&translatorIDs, &idCount);
			for (int32 t = 0; t < idCount; t++) {
				const translation_format* formats;
				int32 formatCount;
				roster->GetOutputFormats(translatorIDs[t], &formats,
					&formatCount);
				for (int32 f = 0; f < formatCount; f++) {
					if (formats[f].group != B_TRANSLATOR_BITMAP)
						continue;
					if (formats[f].type == B_TRANSLATOR_BITMAP)
						continue;
					if (strcmp(formats[f].name, name) == 0) {
						fTargetFormat = formats[f].type;
						found = true;
						break;
					}
				}
				if (found)
					break;
			}
			if (!found) {
				printf("Did not find translator \"%s\". Use -l to "
					"list available transators.\n", name);
				return;
			}
			delete[] translatorIDs;
		} else if (strcmp(argv[i], "-l") == 0) {
			BTranslatorRoster* roster = BTranslatorRoster::Default();
			translator_id* translatorIDs;
			int32 idCount;
			roster->GetAllTranslators(&translatorIDs, &idCount);
			printf("Available formats:\n");
			for (int32 t = 0; t < idCount; t++) {
				const translation_format* formats;
				int32 formatCount;
				roster->GetOutputFormats(translatorIDs[t], &formats,
					&formatCount);
				for (int32 f = 0; f < formatCount; f++) {
					if (formats[f].group != B_TRANSLATOR_BITMAP)
						continue;
					if (formats[f].type == B_TRANSLATOR_BITMAP)
						continue;
					printf("  \"%s\"\n", formats[f].name);
				}
			}
			delete[] translatorIDs;
			return;
		} else
			break;
	}
	if (i == argc) {
		_PrintUsage(argv[0]);
		return;
	}
	create_directory(fTargetFolder.Path(), 0777);

	for (; i < argc; i++) {
		BBitmap* original = BTranslationUtils::GetBitmap(argv[i]);
		if (original == NULL) {
			fprintf(stderr, "Failed to load '%s'\n", argv[i]);
			continue;
		}
		int width = original->Bounds().IntegerWidth() + 1;
		if (fTargetWidth > 0 && fTargetWidth < width)
			width = fTargetWidth;
		int height = original->Bounds().IntegerHeight() + 1;
		if (fTargetHeight > 0 && fTargetWidth < height)
			height = fTargetHeight;

		_CropImage(original, width, height, fTargetFolder, argv[i]);

		delete original;
	}
}

// RefsReceived
void
Cropper::RefsReceived(BMessage* message)
{
	printf("Cropper::RefsReceived()\n");
}

// #pragma mark -

// _PrintUsage
void
Cropper::_PrintUsage(const char* appPath)
{
	printf("Usage: %s -o <target folder> -f <translator> -w <width> -h <height> [image files]\n", appPath);
	printf("  -o  - The target folder to place the resized images in.\n");
	printf("  -w  - The width of the resulting images. No horizontal cropping if ommited.\n");
	printf("  -h  - The height of the resulting images. No vertical cropping if ommited.\n");
	printf("  -f  - Use the specified translator.\n");
	printf("Usage: %s -l\n", appPath);
	printf("  -l  - List all available translators.\n");
}

// _CropImage
status_t
Cropper::_CropImage(const BBitmap* bitmap, int width, int height, BPath path,
	const char* originalPath) const
{
	BRect croppedRect(0, 0, width - 1, height - 1);
	BBitmap* resultBitmap = new BBitmap(croppedRect,
		B_BITMAP_NO_SERVER_LINK, B_RGBA32);
	status_t ret = resultBitmap->InitCheck();
	if (ret != B_OK) {
		fprintf(stderr, "Failed to create bitmap: %s\n", strerror(ret));
		delete bitmap;
		return ret;
	}

	uint8* src = (uint8*)bitmap->Bits();
	uint8* dst = (uint8*)resultBitmap->Bits();
	uint32 srcBPR = bitmap->BytesPerRow();
	uint32 dstBPR = resultBitmap->BytesPerRow();
	uint32 bytes = width * 4;

	for (int y = 0; y < height; y++) {
		memcpy(dst, src, bytes);
		src += srcBPR;
		dst += dstBPR;
	}

	BBitmapStream bitmapStream(resultBitmap);

	BString name(originalPath);
	name.Remove(0, name.FindLast('/') + 1);
	path.Append(name);
	BFile file(path.Path(), B_CREATE_FILE | B_ERASE_FILE | B_READ_WRITE);
	if (file.InitCheck() != B_OK) {
		fprintf(stderr, "Failed to create file '%s': %s\n",
			path.Path(), strerror(file.InitCheck()));
		return file.InitCheck();
	}

	BTranslatorRoster* roster = BTranslatorRoster::Default();
	ret = roster->Translate(&bitmapStream, NULL, NULL, &file, fTargetFormat);
	if (ret != B_OK) {
		fprintf(stderr, "Failed to translate file '%s': %s\n",
			path.Path(), strerror(ret));
		return ret;
	}

	return B_OK;
}

// #pragma mark -

// main
int
main(int argc, const char* argv[])
{
	Cropper app;
	app.Run();
	return 0;
}

