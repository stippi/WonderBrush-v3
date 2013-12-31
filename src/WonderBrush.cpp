#include "WonderBrush.h"

#include "BuildSupport.h"

#include <Alert.h>
#include <Bitmap.h>
#include <Catalog.h>
#include <Path.h>
#include <Roster.h>
#include <String.h>
#include <TranslationUtils.h>

#include "BrushStroke.h"
#include "ColorShade.h"
#include "Document.h"
#include "Filter.h"
#include "FilterDropShadow.h"
#include "FontCache.h"
#include "FontRegistry.h"
#include "Image.h"
#include "Layer.h"
#include "MessageImporter.h"
#include "NativeSaver.h"
#include "Rect.h"
#include "RenderBuffer.h"
#include "Shape.h"
#include "Text.h"
#include "Window.h"


static BString sFontsDirectory;

#define B_TRANSLATION_CONTEXT "WonderBrush"


// #pragma mark - WonderBrushBase


// constructor
WonderBrushBase::WonderBrushBase(BRect bounds)
	:
	fDocument(new Document(bounds)),
	fWindowFrame(bounds.OffsetToCopy(50, 50)),
	fWindowCount(0)
{
	if (sFontsDirectory.Length() == 0) {
		app_info info;
		be_app->GetAppInfo(&info);
		BPath path(&info.ref);
		path.GetParent(&path);
		if (path.SetTo(path.Path(), "data/fonts") == B_OK)
			sFontsDirectory = path.Path();
	}

	// create dummy contents for document
	fDocument->RootLayer()->AddObject(new Rect(BRect(50, 100, 110, 130),
		(rgb_color){ 255, 120, 0, 120 }));
	fDocument->RootLayer()->AddObject(new Rect(BRect(10, 10, 80, 60),
		(rgb_color){ 200, 20, 80, 180 }));
	fDocument->RootLayer()->AddObject(new Filter(10.0));
	fDocument->RootLayer()->AddObject(new Rect(BRect(30, 40, 120, 200),
		(rgb_color){ 97, 215, 255, 255 }));

	Layer* subLayer = new Layer(bounds);
	fDocument->RootLayer()->AddObject(subLayer);

	fDocument->RootLayer()->AddObject(new Filter(5.0));

	PathRef path(new Path(), true);
	BRect shapeArea(180, 40, 320, 170);
	path->AddPoint(BPoint(shapeArea.left, shapeArea.top));
	path->AddPoint(BPoint((shapeArea.left + shapeArea.right) / 2,
		shapeArea.top + shapeArea.Height() / 3));

	path->AddPoint(BPoint(shapeArea.right, shapeArea.top));
	path->AddPoint(BPoint(shapeArea.right - shapeArea.Width() / 3,
		(shapeArea.top + shapeArea.bottom) / 2));

	path->AddPoint(BPoint(shapeArea.right, shapeArea.bottom));
	path->AddPoint(BPoint((shapeArea.left + shapeArea.right) / 2,
		shapeArea.bottom - shapeArea.Height() / 3));

	path->AddPoint(BPoint(shapeArea.left, shapeArea.bottom));
	path->AddPoint(BPoint(shapeArea.left + shapeArea.Width() / 3,
		(shapeArea.top + shapeArea.bottom) / 2));
	path->SetClosed(true);

	fDocument->GlobalResources().AddObject(path.Get());

	Shape* shape = new Shape(path,
		(rgb_color){ 255, 100, 50, 210 });
	shape->RotateBy(BPoint(250, 105), 5);
	fDocument->RootLayer()->AddObject(shape);
	fDocument->RootLayer()->AddObject(new Rect(BRect(200, 10, 280, 70),
		(rgb_color){ 255, 200, 50, 80 }));

	Layer* shadowLayer = new Layer(bounds);
	fDocument->RootLayer()->AddObject(shadowLayer);

	Rect* rect = new Rect(BRect(100, 140, 220, 170),
		(rgb_color){ 255, 215, 20, 100 });
	rect->SetRoundCornerRadius(10.0);

	Style* rectStyle = rect->Style();

	// Publish color as resource, otherwise this connection cannot be
	// restored when saving/loading the document.
	const ColorProviderRef& rectColor
		= rectStyle->FillPaint()->GetColorProvider();
	fDocument->GlobalResources().AddObject(rectColor.Get());

	ColorShade* shade = new ColorShade(rectColor);
	shade->SetHue(1.0f);
	shade->SetValue(-0.25f);

	rectStyle->SetStrokePaint(
		PaintRef(new Paint(ColorProviderRef(shade, true)), true));
	rectStyle->SetStrokeProperties(
		StrokePropertiesRef(new StrokeProperties(8.0f, RoundJoin), true));

	fDocument->RootLayer()->AddObject(rect);

	BBitmap* bitmap = BTranslationUtils::GetBitmap(
		"/boot/home/Desktop/gamma_dalai_lama_gray.jpg");
	if (bitmap == NULL && sFontsDirectory.Length() > 0) {
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
		shadowLayer->AddObject(image);

		buffer->RemoveReference();
		delete bitmap;
	} else
		printf("Test bitmap file not found or failed to load.\n");

	Text* text = new Text((rgb_color){ 0, 0, 0, 255 });
	text->TranslateBy(BPoint(522, 31));
	text->SetWidth(200.0);
	text->SetAlignment(TEXT_ALIGNMENT_JUSTIFY);

	text->Append("There are only ",
		Font("DejaVu Serif", "Book", 24.0), (rgb_color) { 0, 0, 0, 255 });
	text->Append("three",
		Font("Source Sans Pro", "Regular", 46.0),
		(rgb_color) { 0, 170, 255, 255 });
	text->Append(" kind of people in this ",
		Font("DejaVu Serif", "Book", 24.0), (rgb_color) { 0, 0, 0, 255 });
	text->Append("world",
		Font("DejaVu Serif", "Book", 24.0),
		(rgb_color) { 169, 255, 0, 255 });
	text->Append(".\n",
		Font("DejaVu Serif", "Book", 24.0), (rgb_color) { 0, 0, 0, 255 });
	text->Append("Those who ",
		Font("Courier Prime", "Regular", 24.0), (rgb_color) { 0, 0, 0, 255 });
	text->Append("can",
		Font("Courier Prime", "Italic", 32.0), (rgb_color) { 0, 0, 0, 255 });
	text->Append(" count and those who ",
		Font("Courier Prime", "Regular", 24.0), (rgb_color) { 0, 0, 0, 255 });
	text->Append("can't",
		Font("Courier Prime", "Italic", 24.0),
		(rgb_color) { 255, 80, 40, 232 });
	text->Append(".",
		Font("Courier Prime", "Regular", 24.0),
		(rgb_color) { 0, 0, 0, 255 });

	shadowLayer->AddObject(text);

	FilterDropShadow* dropShadow = new FilterDropShadow(2.0f);
	dropShadow->SetOpacity(200.0f);
	dropShadow->SetOffsetY(1.0f);

	shadowLayer->AddObject(dropShadow);

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

	subSubLayer->AddObject(new Shape(path,
		(rgb_color){ 255, 0, 169, 255 }));

	Layer* subSubSubLayer = new Layer(bounds);
	subSubSubLayer->ScaleBy(BPoint(0, 0), 1.8, 1.8);
	subSubLayer->AddObject(subSubSubLayer);

	Style* style = new Style();
	style->SetFillPaint(
		PaintRef(new Paint((rgb_color){ 255, 175, 252, 210 }), true));
	style->SetStrokePaint(
		PaintRef(new Paint((rgb_color){ 20, 60, 255, 150 }), true));
	style->SetStrokeProperties(
		StrokePropertiesRef(new StrokeProperties(5.0f, ButtCap, RoundJoin,
			4.0f), true));
	fDocument->GlobalResources().AddObject(style);

	Shape* shapeWidthGlobalStyle = new Shape();
	shapeWidthGlobalStyle->AddPath(path);
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
			NewWindow(fDocument);
			break;

		case MSG_NEW_DOCUMENT:
		{
			DocumentRef document = NewDocument();
			if (document.Get() != NULL)
				NewWindow(document.Get());
			break;
		}

		case MSG_WINDOW_QUIT:
			WindowQuit(message);
			if (BaseClass::fWindowCount == 0)
				PostMessage(B_QUIT_REQUESTED, this);
			break;

		case MSG_OPEN:
			Open(message);
			break;

		case MSG_SAVE_AS:
		case MSG_EXPORT_AS:
			SaveAs(message);
			break;

		default:
			BaseClass::MessageReceived(message);
			break;
	}
}

// RefsReceived
void
WonderBrush::RefsReceived(BMessage* message)
{
	bool append;
	if (message->FindBool("append", &append) != B_OK)
		append = false;

	Document* document;
	if (message->FindPointer("document", (void**)&document) != B_OK)
		document = NULL;

	// TODO: Check if the document still exists

	// When appending, we need to know a document.
	if (append && document == NULL)
		return;

	entry_ref ref;
	if (append) {
		for (int32 i = 0; message->FindRef("refs", i, &ref) == B_OK; i++)
			OpenDocument(document, ref, true);
	} else {
		for (int32 i = 0; message->FindRef("refs", i, &ref) == B_OK; i++) {
			if (document != NULL && i == 0) {
				OpenDocument(document, ref, true);
			} else {
				DocumentRef document = NewDocument();
				if (OpenDocument(document.Get(), ref, true) == B_OK)
					NewWindow(document.Get());
			}
		}
	}
}

// InitialWindow
void
WonderBrush::InitialWindow()
{
	NewWindow(fDocument);
}

// NewWindow
Window*
WonderBrush::NewWindow(Document* document)
{
	fWindowFrame.OffsetBy(30, 30);

	BString windowName("WonderBrush ");
	windowName << ++fWindowCount;

	Window* window = new Window(fWindowFrame, windowName.String(),
		document, document->RootLayer());

	BMessage windowSettings;
	if (fSettings.FindMessage("window settings", &windowSettings) == B_OK)
		window->RestoreSettings(windowSettings);

	window->Show();
	return window;
}


// NewDocument
DocumentRef
WonderBrush::NewDocument()
{
	Document* document = new Document(BRect(0, 0, 799, 599));
	return DocumentRef(document, true);
}

enum {
	REF_NONE = 0,
	REF_MESSAGE,
//	REF_BITMAP,
//	REF_SVG
};

// OpenDocument
status_t
WonderBrush::OpenDocument(Document* document, const entry_ref& ref, bool append)
{
	AutoWriteLocker locker(document);

	status_t ret;
	if (document == NULL)
		ret = B_NO_MEMORY;
	else
		ret = ImportDocument(document, ref);

	if (ret != B_OK) {
		// inform user of failure at this point
		BString helper(B_TRANSLATE("Opening the document failed!"));
		helper << "\n\n" << B_TRANSLATE("Error: ") << strerror(ret);
		BAlert* alert = new BAlert(
			B_TRANSLATE_CONTEXT("bad news", "Title of error alert"),
			helper.String(), 
			B_TRANSLATE_CONTEXT("Bummer", 
				"Cancel button - error alert"),	
			NULL, NULL);
		// launch alert asynchronously
		alert->SetFlags(alert->Flags() | B_CLOSE_ON_ESCAPE);
		alert->Go(NULL);
		return ret;
	}

	return B_OK;
}

// ImportDocument
status_t
WonderBrush::ImportDocument(Document* document, const entry_ref& ref) const
{
	DocumentRef documentRef(document);

	BFile file(&ref, B_READ_ONLY);
	status_t ret = file.InitCheck();
	if (ret != B_OK)
		return ret;

	// try different file types
	MessageImporter msgImporter(documentRef);
	ret = msgImporter.Import(file);
	if (ret == B_OK) {
		document->SetNativeSaver(new(std::nothrow) NativeSaver(ref));
		return B_OK;
	}
	
//	file.Seek(0, SEEK_SET);
//	SVGImporter svgImporter;
//	ret = svgImporter.Import(icon, &ref);
//	if (ret == B_OK) {
//		document->SetExportSaver(
//			new SimpleFileSaver(new SVGExporter(), ref));
//		return B_OK;
//	}

	return B_ERROR;
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
	// Next window opens in place of the window that just closed
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
			printf("Using font folder: '%s'\n", sFontsDirectory.String());
			break;
		}
	}
	// Create app already here. For Qt this must be the first event loop
	// created and FontRegistry is a BLooper which uses an event loop, too.
	WonderBrush app(argc, argv, BRect(0, 0, 799, 599));

	if (sFontsDirectory.Length() > 0) {
		FontRegistry* registry = FontRegistry::Default();
		if (registry->Lock()) {
			registry->AddFontDirectory(sFontsDirectory);
			registry->Unlock();
		}
	}

	app.Run();
	return 0;
}
