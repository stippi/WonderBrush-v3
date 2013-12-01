/*
 * Copyright 2006-2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef MESSAGE_EXPORTER_H
#define MESSAGE_EXPORTER_H


#include "Exporter.h"

class BMessage;
class BPositionIO;

class MessageExporter : public Exporter {
public:
								MessageExporter();
	virtual						~MessageExporter();

	virtual	status_t			Export(const LayerSnapshot* rootSnapshot,
									BPositionIO* stream);

	virtual	const char*			MIMEType();

private:
};

#endif // MESSAGE_EXPORTER_H
