/*
 * Copyright 2006-2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef EXPORTER_H
#define EXPORTER_H

#include <Entry.h>
#include <OS.h>

#include "Document.h"

class BPositionIO;
class LayerSnapshot;

class Exporter {
public:
								Exporter();
	virtual						~Exporter();

			status_t			Export(const DocumentRef& document,
									const entry_ref& ref);

	virtual	status_t			Export(const DocumentRef& document,
									BPositionIO* stream) = 0;

	virtual	const char*			MIMEType() = 0;

			void				SetSelfDestroy(bool selfDestroy);

			void				WaitForExportThread();

private:
	static	int32				_ExportThreadEntry(void* cookie);
			int32				_ExportThread();
			status_t			_Export(const DocumentRef& document,
									const entry_ref* docRef);

private:
			DocumentRef			fDocument;
			entry_ref			fRef;
			thread_id			fExportThread;
			bool				fSelfDestroy;
};

#endif // EXPORTER_H
