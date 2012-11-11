#include "BTranslationUtils.h"

#include "BBitmap.h"


/*static*/ BBitmap*
BTranslationUtils::GetBitmap(const char* name, BTranslatorRoster* /*roster*/)
{
	QImage image(QString::fromUtf8(name));
	if (image.isNull())
		return NULL;

	BBitmap* bitmap = new(std::nothrow) BBitmap(image);
	if (bitmap == NULL || !bitmap->IsValid()) {
		delete bitmap;
		return NULL;
	}

	return bitmap;
}
