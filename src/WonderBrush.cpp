#include "WonderBrush.h"

#include "BuildSupport.h"

#include <Bitmap.h>
#include <Path.h>
#include <String.h>
#include <TranslationUtils.h>

#include "BrushStroke.h"
#include "Document.h"
#include "Filter.h"
#include "FontCache.h"
#include "FontRegistry.h"
#include "Image.h"
#include "Layer.h"
#include "Rect.h"
#include "RenderBuffer.h"
#include "Shape.h"
#include "Text.h"
#include "Window.h"


static const char* sFontsDirectory = NULL;


// #pragma mark - WonderBrushBase


// constructor
WonderBrushBase::WonderBrushBase(BRect bounds)
	:
	fDocument(new Document(bounds)),
	fWindowFrame(bounds.OffsetToCopy(50, 50)),
	fWindowCount(0)
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
	if (bitmap == NULL && sFontsDirectory != NULL) {
		// A font directory is given. If it's the one in the sources, it is a
		// subdir of the data directory which contains our image.
		BPath filePath(sFontsDirectory, "../gamma_dalai_lama_gray.jpg");
		bitmap = BTranslationUtils::GetBitmap(filePath.Path());
	}

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

	Text* text = new Text((rgb_color){ 0, 0, 0, 255 });
	text->TranslateBy(BPoint(522, 31));
	text->SetWidth(200.0);
	text->SetJustify(true);
	text->SetText("This is a test of the new text layouting features.",
		Font("DejaVu Serif", "Book", 24.0), (rgb_color) { 0, 0, 0, 255 });
	fDocument->RootLayer()->AddObject(text);

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
	Brush* brush = new Brush(0.0f, 1.0f, 10.0f, 20.0f, 0.0f, 0.7f,
		Brush::FLAG_PRESSURE_CONTROLS_APHLA
			| Brush::FLAG_PRESSURE_CONTROLS_RADIUS
			| Brush::FLAG_PRESSURE_CONTROLS_HARDNESS
			| Brush::FLAG_TILT_CONTROLS_SHAPE);
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


WonderBrushBase::~WonderBrushBase()
{
	delete fDocument;
}


// #pragma mark - WonderBrush


WonderBrush::WonderBrush(int& argc, char** argv, BRect bounds)
	:
	BaseClass(argc, argv, bounds)
{
}


void
WonderBrush::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_OBSERVER_NOTICE_CHANGE:
		{
			int32 what;
			if (message->FindInt32(B_OBSERVE_ORIGINAL_WHAT, &what) == B_OK
				&& what == MSG_FONTS_CHANGED) {
				NotifyFontsLoaded();
			}
			break;
		}
		case MSG_NEW_WINDOW:
			NewWindow();
			break;
		case MSG_WINDOW_QUIT:
		{
			WindowQuit(message);
			if (BaseClass::fWindowCount == 0)
				PostMessage(B_QUIT_REQUESTED, this);
			break;
		}
		default:
			BaseClass::MessageReceived(message);
			break;
	}
}


// NewWindow
void
WonderBrush::NewWindow()
{
	fWindowFrame.OffsetBy(30, 30);

	BString windowName("WonderBrush ");
	windowName << ++fWindowCount;

	Window* window = new Window(fWindowFrame, windowName.String(),
		fDocument, fEditLayer);

	BMessage windowSettings;
	if (fSettings.FindMessage("window settings", &windowSettings) == B_OK)
		window->RestoreSettings(windowSettings);

	window->Show();
}


// WindowQuit
void
WonderBrush::WindowQuit(BMessage* message)
{
	BMessage windowSettings;
	if (message->FindMessage("window settings", &windowSettings) == B_OK) {
		fSettings.RemoveName("window settings");
		fSettings.AddMessage("window settings", &windowSettings);
	}
	message->FindRect("window frame", &fWindowFrame);

	fWindowCount--;
}


// StoreSettings
void
WonderBrush::StoreSettings()
{
	BFile file;
	status_t status = OpenSettingsFile(file, true);
	if (status != B_OK) {
		fprintf(stderr, "Failed to create application settings: %s\n",
			strerror(status));
		return;
	}

	fSettings.RemoveName("window frame");
	fSettings.AddRect("window frame", fWindowFrame);

	status = fSettings.Flatten(&file);
	if (status != B_OK) {
		fprintf(stderr, "Failed to save application settings: %s\n",
			strerror(status));
		return;
	}
}


// RestoreSettings
void
WonderBrush::RestoreSettings()
{
	BFile file;
	status_t status = OpenSettingsFile(file, false);
	if (status != B_OK) {
		if (status != B_ENTRY_NOT_FOUND)
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

// NotifyFontsLoaded
void
WonderBrush::NotifyFontsLoaded()
{
	_NotifyFontsLoaded(fDocument->RootLayer());
}

// _NotifyFontsLoaded
void
WonderBrush::_NotifyFontsLoaded(Layer* layer)
{
	for (int32 i = layer->CountObjects() - 1; i >= 0; i--) {
		Object* object = layer->ObjectAtFast(i);
		Layer* subLayer = dynamic_cast<Layer*>(object);
		if (subLayer != NULL) {
			_NotifyFontsLoaded(subLayer);
			continue;
		}
		Text* text = dynamic_cast<Text*>(object);
		if (text != NULL)
			text->UpdateLayout();
	}
}


// #pragma mark -


// main
int
main(int argc, char* argv[])
{
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--fonts") == 0 && i < argc - 1) {
			sFontsDirectory = argv[i + 1];
			printf("Using font folder: '%s'\n", sFontsDirectory);
			break;
		}
	}

	// Create app already here. For Qt this must be the first event loop
	// created and FontRegistry is a BLooper which uses an event loop, too.
	WonderBrush app(argc, argv, BRect(0, 0, 799, 599));

	if (sFontsDirectory != NULL) {
		FontRegistry* registry = FontRegistry::Default();
		if (registry->Lock()) {
			registry->AddFontDirectory(sFontsDirectory);
			registry->Unlock();
		}
	}

	app.Run();
	return 0;
}
