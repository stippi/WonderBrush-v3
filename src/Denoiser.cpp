#include "Denoiser.h"

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
Denoiser::Denoiser()
	: BApplication("application/x-vnd.yellowbites.Denoiser")
	, fPrintUsage(true)

	, fAmplitude(60.0f)
	, fSharpness(0.7f)
	, fAnisotropy(0.6f)
	, fAlpha(0.6f)
	, fSigma(1.1f)
	, fDL(0.8f)
	, fDA(30.0f)
	, fGaussPrecision(2.0f)
	, fInterpolationType(0)
	, fFastAproximation(true)

	, fTargetFolder("/boot/home/Desktop")
	, fTargetFormat(B_JPEG_FORMAT)
{
}

// MessageReceived
void
Denoiser::MessageReceived(BMessage* message)
{
	switch (message->what) {
		default:
			BApplication::MessageReceived(message);
			break;
	}
}

// ReadyToRun
void
Denoiser::ReadyToRun()
{
	if (fPrintUsage)
		_PrintUsage("Denoiser");
	PostMessage(B_QUIT_REQUESTED);
}

// ArgvReceived
void
Denoiser::ArgvReceived(int32 argc, char** argv)
{
	fPrintUsage = false;
	int32 i = 1;
	for (; i < argc; i++) {
		if (strcmp(argv[i], "-o") == 0) {
			if (i == argc - 1)
				break;
			fTargetFolder = argv[++i];
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

	RenderEngine engine;

	for (; i < argc; i++) {
		BPath path(argv[i]);
		BBitmap* original = BTranslationUtils::GetBitmap(path.Path());
		if (original == NULL) {
			fprintf(stderr, "Failed to load '%s'\n", argv[i]);
			continue;
		}
		RenderBuffer buffer(original);

		_DenoiseImage(buffer, engine, fTargetFolder, argv[i]);

		delete original;
	}
}

// RefsReceived
void
Denoiser::RefsReceived(BMessage* message)
{
	printf("Denoiser::RefsReceived()\n");
}

// #pragma mark -

// _PrintUsage
void
Denoiser::_PrintUsage(const char* appPath)
{
	printf("Usage: %s -o <target folder> -f <translator> -w <width> -h <height> [image files]\n", appPath);
	printf("  -o  - The target folder to place the resized images in.\n");
	printf("  -f  - Use the specified translator.\n");
	printf("Usage: %s -l\n", appPath);
	printf("  -l  - List all available translators.\n");
}

// _DenoiseImage
status_t
Denoiser::_DenoiseImage(const RenderBuffer& buffer, RenderEngine& engine,
	BPath path, const char* originalPath) const
{
	engine.Denoise(&buffer, fAmplitude, fSharpness, fAnisotropy, fAlpha,
		fSigma, fDL, fDA, fGaussPrecision, fInterpolationType,
		fFastAproximation);

	BBitmap* bitmap = new BBitmap(buffer.Bounds(), B_BITMAP_NO_SERVER_LINK,
		B_RGBA32);
	status_t ret = bitmap->InitCheck();
	if (ret != B_OK) {
		fprintf(stderr, "Failed to create bitmap: %s\n", strerror(ret));
		delete bitmap;
		return ret;
	}

	buffer.CopyTo(bitmap, buffer.Bounds());
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
	Denoiser app;
	app.Run();
	return 0;
}

