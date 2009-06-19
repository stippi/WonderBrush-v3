#include "App.h"

#include <String.h>

#include "Document.h"
#include "Filter.h"
#include "Layer.h"
#include "Rect.h"
#include "Shape.h"
#include "Window.h"

// constructor
App::App(BRect bounds)
	: BApplication("application/x-vnd.stippi.DocumentFramework")
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
		(rgb_color){ 90, 0, 20, 210 }));
	fDocument->RootLayer()->AddObject(new Rect(BRect(100, 140, 180, 170),
		(rgb_color){ 255, 215, 20, 100 }));

	Layer* subLayer = new Layer(bounds);
	fDocument->RootLayer()->AddObject(subLayer);

	fDocument->RootLayer()->AddObject(new Filter(5.0));
	fDocument->RootLayer()->AddObject(new Shape(BRect(180, 40, 320, 170),
		(rgb_color){ 255, 100, 50, 210 }));
	fDocument->RootLayer()->AddObject(new Rect(BRect(200, 10, 280, 70),
		(rgb_color){ 255, 200, 50, 80 }));

	subLayer->AddObject(new Rect(BRect(150, 200, 210, 330),
		(rgb_color){ 55, 120, 80, 120 }));
	subLayer->AddObject(new Filter(20.0));

	subLayer->AddObject(new Rect(BRect(120, 100, 510, 530),
		(rgb_color){ 55, 180, 120, 200 }));

	Layer* subSubLayer = new Layer(bounds);
	subLayer->AddObject(subSubLayer);

	subSubLayer->AddObject(new Rect(BRect(420, 320, 650, 390),
		(rgb_color){ 185, 120, 120, 220 }));

	subSubLayer->AddObject(new Shape(BRect(460, 185, 590, 300),
		(rgb_color){ 255, 120, 180, 160 }));

	fEditLayer = fDocument->RootLayer();
}

// MessageReceived
void
App::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_NEW_WINDOW:
			_NewWindow();
			break;
		case MSG_WINDOW_QUIT:
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
App::ReadyToRun()
{
	_NewWindow();
}

// _NewWindow
void
App::_NewWindow()
{
	fWindowFrame.OffsetBy(30, 30);

	BString windowName("Document Framework ");
	windowName << ++fWindowCount;

	Window* window = new Window(fWindowFrame, windowName.String(),
		fDocument, fEditLayer);
	window->Show();
}

