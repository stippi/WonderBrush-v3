#include "Resizer.h"

#include <Bitmap.h>
#include <BitmapStream.h>
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
	printf("Resizer::ReadyToRun()\n");
	PostMessage(B_QUIT_REQUESTED);
}

// ArgvReceived
void
Resizer::ArgvReceived(int32 argc, char** argv)
{
	printf("Resizer::ArgvReceived()\n");

	RenderEngine engine;

	for (int32 i = 1; i < argc; i++) {
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
		_ResizeImage(buffer, engine, thumbScale, fTargetThumbFolder, argv[i]);

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

