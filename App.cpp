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
	fDocument->RootLayer()->AddObject(new Filter(5.0));
	fDocument->RootLayer()->AddObject(new Shape(BRect(180, 40, 320, 170),
		(rgb_color){ 255, 100, 50, 210 }));
	fDocument->RootLayer()->AddObject(new Rect(BRect(200, 10, 280, 70),
		(rgb_color){ 255, 200, 50, 80 }));
}

// MessageReceived
void
App::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_NEW_WINDOW:
			_NewWindow();
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

	Window* window = new Window(fWindowFrame, windowName.String(), fDocument);
	window->Show();
}

