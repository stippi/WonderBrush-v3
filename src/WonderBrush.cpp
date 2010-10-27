#include "WonderBrush.h"

#include <Bitmap.h>
#include <Directory.h>
#include <FindDirectory.h>
#include <File.h>
#include <Path.h>
#include <String.h>
#include <TranslationUtils.h>

#include "BrushStroke.h"
#include "Document.h"
#include "Filter.h"
#include "Image.h"
#include "Layer.h"
#include "Rect.h"
#include "RenderBuffer.h"
#include "Shape.h"
#include "Window.h"

// constructor
WonderBrush::WonderBrush(BRect bounds)
	: BApplication("application/x-vnd.Yellowbites.WonderBrush2")
	, fDocument(new Document(bounds))
	, fWindowFrame(bounds.OffsetToCopy(50, 50))
	, fWindowCount(0)
{
	// create dummy contents for document
	fDocument->RootLayer()->AddObject(new Rect(BRect(50, 100, 110, 130),
		(rgb_color){ 255, 120, 0, 120 }));
	fDocument->RootLayer()->AddObject(new Rect(BRect(10, 10, 80, 60),
		(rgb_color){ 200, 20, 80, 180 }));
	fDocument->RootLayer()->AddObject(new Filter(10.0));
	fDocument->RootLayer()->AddObject(new Rect(BRect(30, 40, 120, 200),
		(rgb_color){ 97, 215, 255, 255 }));
	fDocument->RootLayer()->AddObject(new Rect(BRect(100, 140, 180, 170),
		(rgb_color){ 255, 215, 20, 100 }));

	Layer* subLayer = new Layer(bounds);
	fDocument->RootLayer()->AddObject(subLayer);

	fDocument->RootLayer()->AddObject(new Filter(5.0));
	Shape* shape = new Shape(BRect(180, 40, 320, 170),
		(rgb_color){ 255, 100, 50, 210 });
	shape->RotateBy(BPoint(250, 105), 5);
	fDocument->RootLayer()->AddObject(shape);
	fDocument->RootLayer()->AddObject(new Rect(BRect(200, 10, 280, 70),
		(rgb_color){ 255, 200, 50, 80 }));

	BBitmap* bitmap = BTranslationUtils::GetBitmap(
		"/boot/home/Desktop/gamma_dalai_lama_gray.jpg");
	if (bitmap != NULL) {
		RenderBuffer* buffer = new RenderBuffer(bitmap);
		Image* image = new Image(buffer);
		image->ScaleBy(BPoint(buffer->Width() / 2, buffer->Height() / 2),
			0.5, 0.5);
//		image->RotateBy(BPoint(buffer->Width() / 2, buffer->Height() / 2),
//			2);
		fDocument->RootLayer()->AddObject(image);

		buffer->RemoveReference();
		delete bitmap;
	} else
		printf("Test bitmap file not found or failed to load.\n");

	Rect* transformedRect = new Rect(BRect(150, 200, 210, 330),
		(rgb_color){ 55, 120, 80, 120 });
	transformedRect->ScaleBy(BPoint(180, 265), 1.0, 0.5);
	subLayer->AddObject(transformedRect);
	subLayer->AddObject(new Filter(20.0));

	subLayer->AddObject(new Rect(BRect(120, 100, 510, 530),
		(rgb_color){ 110, 127, 0, 255 }));

	subLayer->RotateBy(BPoint(400, 300), -30);

	Layer* subSubLayer = new Layer(bounds);
	subLayer->AddObject(subSubLayer);

	subSubLayer->RotateBy(BPoint(400, 300), -15);

	subSubLayer->AddObject(new Rect(BRect(420, 320, 650, 390),
		(rgb_color){ 0, 255, 0, 240 }));

	subSubLayer->AddObject(new Shape(BRect(460, 185, 590, 300),
		(rgb_color){ 255, 0, 169, 255 }));

	Layer* subSubSubLayer = new Layer(bounds);
	subSubLayer->AddObject(subSubSubLayer);

	Style* style = new Style();
	style->SetFillPaint(Paint((rgb_color){ 255, 175, 252, 210 }));
	style->SetStrokePaint(Paint((rgb_color){ 20, 60, 255, 150 }));
	style->SetStrokeProperties(StrokeProperties(5.0f, ButtCap, RoundJoin,
		4.0f));
	fDocument->GlobalResources().AddObject(style);

	Shape* shapeWidthGlobalStyle = new Shape();
	shapeWidthGlobalStyle->SetArea(BRect(120, 80, 230, 190));
	shapeWidthGlobalStyle->SetStyle(style);
	subSubSubLayer->AddObject(shapeWidthGlobalStyle);
	
	Rect* rectWidthGlobalStyle = new Rect();
	rectWidthGlobalStyle->SetArea(BRect(150, 330, 240, 420));
	rectWidthGlobalStyle->SetStyle(style);
	subLayer->AddObject(rectWidthGlobalStyle);

	BrushStroke* brushStroke = new BrushStroke();
	Brush* brush = new Brush(10.0f, 20.0f, 0.0f, 0.7f);
	brushStroke->SetBrush(brush);
	brush->RemoveReference();
	brushStroke->Stroke().AppendObject(
		StrokePoint(BPoint(150, 50), 0.2f, 0.0f, 0.0f));
	brushStroke->Stroke().AppendObject(
		StrokePoint(BPoint(200, 20), 1.0f, 0.0f, 0.0f));
	brushStroke->Stroke().AppendObject(
		StrokePoint(BPoint(250, 80), 0.8f, 0.0f, 0.0f));
	brushStroke->Stroke().AppendObject(
		StrokePoint(BPoint(300, 50), 0.1f, 0.0f, 0.0f));
	subLayer->AddObject(brushStroke);

	fEditLayer = fDocument->RootLayer();
}

// MessageReceived
void
WonderBrush::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_NEW_WINDOW:
			_NewWindow();
			break;
		case MSG_WINDOW_QUIT:
			message->FindRect("window frame", &fWindowFrame);
			fWindowCount--;
			if (fWindowCount == 0)
				PostMessage(B_QUIT_REQUESTED, this);
			break;
		default:
			BApplication::MessageReceived(message);
			break;
	}
}

// ReadyToRun
void
WonderBrush::ReadyToRun()
{
	_RestoreSettings();
	_NewWindow();
}

// QuitRequested
bool
WonderBrush::QuitRequested()
{
	_StoreSettings();
	return BApplication::QuitRequested();
}

// #pragma mark -

// _NewWindow
void
WonderBrush::_NewWindow()
{
	fWindowFrame.OffsetBy(30, 30);

	BString windowName("WonderBrush ");
	windowName << ++fWindowCount;

	Window* window = new Window(fWindowFrame, windowName.String(),
		fDocument, fEditLayer);
	window->Show();
}

// _OpenSettingsFile
status_t
WonderBrush::_OpenSettingsFile(BFile& file, bool forWriting)
{
	BPath path;
	status_t ret = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	if (ret != B_OK) {
		fprintf(stderr, "Failed to find the user settings directory.\n");
		return ret;
	}
	ret = path.Append("WonderBrush3");
	if (ret != B_OK) {
		fprintf(stderr, "Failed to initialize the settings path.\n");
		return ret;
	}

	ret = create_directory(path.Path(), 0666);
	if (ret != B_OK) {
		fprintf(stderr, "Failed to create the settings path.\n");
		return ret;
	}

	ret = path.Append("main_settings");
	if (ret != B_OK) {
		fprintf(stderr, "Failed to initialize the settings path.\n");
		return ret;
	}

	if (forWriting) {
		return file.SetTo(path.Path(), B_CREATE_FILE | B_ERASE_FILE
			| B_WRITE_ONLY);
	} else {
		return file.SetTo(path.Path(), B_READ_ONLY);
	}
}

// _StoreSettings
void
WonderBrush::_StoreSettings()
{
	BFile file;
	if (_OpenSettingsFile(file, true) != B_OK) {
		fprintf(stderr, "Failed to create application settings.\n");
		return;
	}

	fSettings.RemoveName("window frame");
	fSettings.AddRect("window frame", fWindowFrame);

	if (fSettings.Flatten(&file) != B_OK) {
		fprintf(stderr, "Failed to save application settings.\n");
		return;
	}
}

// _RestoreSettings
void
WonderBrush::_RestoreSettings()
{
	BFile file;
	if (_OpenSettingsFile(file, false) != B_OK) {
		fprintf(stderr, "Failed to open application settings.\n");
		return;
	}

	if (fSettings.Unflatten(&file) != B_OK) {
		fprintf(stderr, "Failed to read application settings.\n");
		return;
	}

	if (fSettings.FindRect("window frame", &fWindowFrame) == B_OK)
		fWindowFrame.OffsetBy(-30, -30);
}

// #pragma mark -

// main
int
main(int argc, const char* argv[])
{
	WonderBrush app(BRect(0, 0, 799, 599));
	app.Run();
	return 0;
}
