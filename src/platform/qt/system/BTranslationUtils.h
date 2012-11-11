#ifndef PLATFORM_QT_B_TRANSLATION_UTILS_H
#define PLATFORM_QT_B_TRANSLATION_UTILS_H


#include <stddef.h>


class BBitmap;
class BTranslatorRoster;


class BTranslationUtils {
public:

	static	BBitmap*			GetBitmap(const char* name,
									BTranslatorRoster* roster = NULL);

};


#endif // PLATFORM_QT_B_TRANSLATION_UTILS_H
