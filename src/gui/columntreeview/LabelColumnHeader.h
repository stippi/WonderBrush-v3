// LabelColumnHeader.h

#ifndef LABEL_COLUMN_HEADER_H
#define LABEL_COLUMN_HEADER_H

#include <String.h>

#include "ColumnHeader.h"

class LabelColumnHeader : public ColumnHeader {
 public:
								LabelColumnHeader(const char* label);
	virtual						~LabelColumnHeader();

	virtual	void				Draw(BView* view, BRect frame,
									 BRect updateRect, uint32 flags,
									 const column_header_colors* colors);

			void				SetLabel(const char* label);
			const char*			Label() const;

 private:
			BString				fLabel;
};

#endif	// LABEL_COLUMN_HEADER_H

