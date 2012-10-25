#ifndef PLATFORMRESOURCEPARSER_H
#define PLATFORMRESOURCEPARSER_H


#include <SupportDefs.h>


class BResources;


class PlatformResourceParser {
public:
								PlatformResourceParser();

			status_t			ParseAppResources(BResources& resources);

private:
			struct Token;
			struct Exception;
			struct Tokenizer;

private:
			void				_Parse();
			void				_ParseResource();

private:
			Tokenizer*			fTokenizer;
			BResources*			fResources;
};


#endif // PLATFORMRESOURCEPARSER_H
