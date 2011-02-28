#include "Resizer.h"

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
Resizer::Resizer()
	: BApplication("application/x-vnd.yellowbites.Resizer")
	, fTargetSize(1536)
	, fTargetThumbSize(308)
	, fTargetScale(1.0)
	, fTargetThumbScale(1.0)
	, fTargetTranslator("JPEG images")
	, fTargetFolder("/Data/home/mika/images")
	, fTargetThumbFolder("/Data/home/mika/thumbs")
{
}

// MessageReceived
void
Resizer::MessageReceived(BMessage* message)
{
	switch (message->what) {
		default:
			BApplication::MessageReceived(message);
			break;
	}
}

// ReadyToRun
void
Resizer::ReadyToRun()
{
	_PrintUsage("Resizer");
	PostMessage(B_QUIT_REQUESTED);
}

// ArgvReceived
void
Resizer::ArgvReceived(int32 argc, char** argv)
{
	printf("Resizer::ArgvReceived()\n");
	int32 i = 1;
	for (; i < argc; i++) {
		if (strcmp(argv[i], "-o") == 0) {
			if (i == argc - 1)
				break;
			fTargetFolder = argv[++i];
		} else if (strcmp(argv[i], "-s") == 0) {
			if (i == argc - 1)
				break;
			fTargetSize = atoi(argv[++i]);
		} else
			break;
	}
	if (i == argc) {
		_PrintUsage(argv[0]);
		return;
	}
	create_directory(fTargetFolder.Path(), 0777);

	RenderEngine engine;

	for (; i < argc; i++) {
		BBitmap* original = BTranslationUtils::GetBitmap(argv[i]);
		if (original == NULL) {
			fprintf(stderr, "Failed to load '%s'\n", argv[i]);
			continue;
		}
		RenderBuffer buffer(original);
		double scale;
		double thumbScale;
		if (buffer.Width() > buffer.Height()) {
			scale = fTargetScale != 1.0 ? fTargetScale
				: (double)fTargetSize / buffer.Width();
			thumbScale = fTargetThumbScale != 1.0 ? fTargetThumbScale
				: (double)fTargetThumbSize / buffer.Width();
		} else {
			scale = fTargetScale != 1.0 ? fTargetScale
				: (double)fTargetSize / buffer.Height();
			thumbScale = fTargetThumbScale != 1.0 ? fTargetThumbScale
				: (double)fTargetThumbSize / buffer.Height();
		}
		_ResizeImage(buffer, engine, scale, fTargetFolder, argv[i]);
		//_ResizeImage(buffer, engine, thumbScale, fTargetThumbFolder, argv[i]);

		delete original;
	}
}

// RefsReceived
void
Resizer::RefsReceived(BMessage* message)
{
	printf("Resizer::RefsReceived()\n");
}

// #pragma mark -

// _PrintUsage
void
Resizer::_PrintUsage(const char* appPath)
{
	printf("Usage: %s -o <target folder> -s <size> [image files]\n", appPath);
	printf("  -o  - The target folder to place the resized images in.\n");
	printf("  -s  - The length (in pixels) of the longer side of the target "
		"images. All images are scaled while maintaining their aspect "
		"ratio.\n");
}

// _ResizeImage
status_t
Resizer::_ResizeImage(const RenderBuffer& buffer, RenderEngine& engine,
	double scale, BPath path, const char* originalPath) const
{
	RenderBuffer resized(
		round(buffer.Width() * scale),
		round(buffer.Height() * scale));
	if (!resized.IsValid())
		return B_NO_MEMORY;

	engine.AttachTo(&resized);

	Transformable scaling;
	scaling.ScaleBy(B_ORIGIN, scale, scale);
	engine.SetTransformation(scaling);

	engine.DrawImage(&buffer, resized.Bounds());

	BBitmap* bitmap = new BBitmap(resized.Bounds(), B_BITMAP_NO_SERVER_LINK,
		B_RGBA32);
	if (bitmap->InitCheck() != B_OK) {
		fprintf(stderr, "Failed to create bitmap: %s\n",
			strerror(bitmap->InitCheck()));
		return bitmap->InitCheck();
	}

	resized.CopyTo(bitmap, resized.Bounds());

	BBitmapStream bitmapStream(bitmap);

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
	status_t ret = roster->Translate(&bitmapStream, NULL, NULL, &file,
		B_JPEG_FORMAT);
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
	Resizer app;
	app.Run();
	return 0;
}

