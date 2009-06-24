// ColumnTreeViewTest.cpp

#include <stdio.h>
#include <stdlib.h>

#include <Message.h>
#include <OS.h>

#include "HGroup.h"
#include "MButton.h"
#include "MBViewWrapper.h"
#include "MWindow.h"
#include "VGroup.h"

#include "Column.h"
#include "ColumnTreeItem.h"
#include "ColumnTreeModel.h"
#include "ColumnTreeView.h"
#include "DefaultColumnTreeModel.h"
#include "MScrollView.h"
#include "TextColumnTreeItem.h"
#include "Thread.h"

// message what fields
enum {
	MSG_ADD_ITEM	= 'addi',
	MSG_REMOVE_ITEM	= 'remi',
	MSG_CLEAR_TREE	= 'cler',
};

// cmp_text_items
static
int
cmp_text_items(const ColumnTreeItem* _item1, const ColumnTreeItem* _item2,
			   const Column* column)
{
	const TextColumnTreeItem* item1
		= dynamic_cast<const TextColumnTreeItem*>(_item1);
	const TextColumnTreeItem* item2
		= dynamic_cast<const TextColumnTreeItem*>(_item2);
	if (!item1 || !item2)
		return 0;
	int32 index = column->Index();
	return strcmp(item1->TextAt(index), item2->TextAt(index));
}

// TestThread
class TestThread : public Thread {
public:
								TestThread();

protected:
	virtual	int32				ThreadFunction();
};

// TestHandler
class TestHandler : public BHandler {
public:
								TestHandler(ColumnTreeView* treeView);

	virtual	void				MessageReceived(BMessage* message);

private:
			ColumnTreeView*		fTreeView;
			ColumnTreeModel*	fTreeModel;
};

static TestThread	sTestThread;

// TestThread

// constructor
TestThread::TestThread()
	: Thread("init CTV test thread", B_NORMAL_PRIORITY)
{
	Resume();
}

// ThreadFunction
int32
TestThread::ThreadFunction()
{
	snooze(2000000);
	// create tree view
	ColumnTreeView* treeView = new ColumnTreeView();
	Column* column1 = new Column("Column 1", "column1", 150,
								  COLUMN_MOVABLE | COLUMN_VISIBLE
								  | COLUMN_SORT_KEYABLE);
	treeView->AddColumn(column1);
	Column* column2 = new Column("Column 2", "column2", 150,
								  COLUMN_MOVABLE | COLUMN_VISIBLE
								  | COLUMN_SORT_KEYABLE);
	treeView->AddColumn(column2);
	treeView->SetModel(new DefaultColumnTreeModel);
	treeView->SetSortCompareFunction(cmp_text_items);
	// create scroll view
	MScrollView* scrollView = new MScrollView(treeView,
		SCROLL_HORIZONTAL | SCROLL_VERTICAL | SCROLL_HORIZONTAL_MAGIC
		| SCROLL_VERTICAL_MAGIC, BRect(0, 0, 100, 300), "test tree", 0, 0);
	scrollView->SetScrollTarget(treeView);
	// create window
	MWindow* window = new MWindow(BRect(50.0, 50.0, 850.0, 650.0), "CTV Test",
								  B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS);
	TestHandler* handler = new TestHandler(treeView);
	window->AddHandler(handler);
	// create view hierarchy
	BView* topview =
	new VGroup (
		// tree
		scrollView,
		// buttons
		new HGroup(
			new MButton("Add", new BMessage(MSG_ADD_ITEM), handler),
			new MButton("Remove", new BMessage(MSG_REMOVE_ITEM), handler),
			new MButton("Clear", new BMessage(MSG_CLEAR_TREE), handler),
			NULL
		),
		NULL
	);
	window->AddChild(topview);
	window->Show();
	return 0;
}


// TestHandler

// constructor
TestHandler::TestHandler(ColumnTreeView* treeView)
	: BHandler(),
	  fTreeView(treeView),
	  fTreeModel(treeView->GetModel())
{
}

// MessageReceived
void
TestHandler::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_ADD_ITEM:
		{
			printf("MSG_ADD_ITEM\n");
			int32 selected = fTreeView->CurrentSelection();
			ColumnTreeItem* selectedItem = fTreeView->ItemAt(selected);
			TextColumnTreeItem* item = new TextColumnTreeItem(20);
			BString text = "item";
			text << rand() % 100 << "_c";
			item->SetText((BString(text) << 0).String(), 0);
			item->SetText((BString(text) << 1).String(), 1);
			item->SetText((BString(text) << 2).String(), 2);
			fTreeView->AddSubItem(selectedItem, item);
			fTreeView->ExpandItem(item);
			break;
		}
		case MSG_REMOVE_ITEM:
			printf("MSG_REMOVE_ITEM\n");
			break;
		case MSG_CLEAR_TREE:
			printf("MSG_CLEAR_TREE\n");
			break;
	}
}

