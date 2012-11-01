/*
 * Copyright 2006-2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */


#include "CurrentColor.h"

#include <stdio.h>

#include <OS.h>

#include "ui_defines.h"


CurrentColor::CurrentColor()
	: Notifier()
	, fColor(kBlack)
{
}


CurrentColor::~CurrentColor()
{
}


void
CurrentColor::SetColor(const rgb_color& color)
{
	if (fColor == color)
		return;

	fColor = color;
	Notify();
}

