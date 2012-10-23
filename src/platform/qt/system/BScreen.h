/*
 * Copyright 2007-2009, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef BSCREEN_H
#define BSCREEN_H


#include <GraphicsDefs.h>
#include <Rect.h>


class BWindow;


class BScreen
{
public:
								BScreen(screen_id id = B_MAIN_SCREEN_ID);
								BScreen(BWindow* window);
								~BScreen();

			bool				IsValid();

			BRect				Frame();

private:
			int					fScreenIndex;
};


#endif // BSCREEN_H
