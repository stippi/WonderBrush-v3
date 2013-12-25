/*
 * Copyright 2006, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "FileSaver.h"

// constructor
FileSaver::FileSaver(const entry_ref& ref)
	: fRef(ref)
{
}

// destructor
FileSaver::~FileSaver()
{
}

// SetRef
void
FileSaver::SetRef(const entry_ref& ref)
{
	fRef = ref;
}

