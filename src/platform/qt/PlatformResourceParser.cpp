#include "PlatformResourceParser.h"

#include <ctype.h>

#include <Resources.h>

#include <QFile>


static const QByteArray kResourceString("resource");
static const QByteArray kVectorIconTypeString("VICN");
static const QByteArray kArrayString("array");

static const char kTokenInvalid				= '0';
static const char kTokenQuotedString		= '"';
static const char kTokenSingleQuotedString	= '\'';
static const char kTokenUnquotedString		= '_';
static const char kTokenResource			= 'r';
static const char kTokenDollar				= '$';
static const char kTokenHash				= '#';
static const char kTokenOpeningParenthesis	= '(';
static const char kTokenClosingParenthesis	= ')';
static const char kTokenOpeningBrace		= '{';
static const char kTokenClosingBrace		= '}';
static const char kTokenSemicolon			= ';';
static const char kTokenComma				= ',';


struct PlatformResourceParser::Token {
	char		fType;
	QByteArray	fString;

	Token()
		:
		fType(kTokenInvalid),
		fString()
	{
	}

	Token(char type, const QByteArray& string)
		:
		fType(type),
		fString(string)
	{
	}

	bool IsValid() const
	{
		return fType != kTokenInvalid;
	}

	bool operator==(char type) const
	{
		return fType == type;
	}

	bool operator!=(char type) const
	{
		return fType != type;
	}

	bool operator==(const Token& other) const
	{
		return fType == other.fType && fString == other.fString;
	}

	bool operator!=(const Token& other) const
	{
		return !(*this == other);
	}
};


struct PlatformResourceParser::Exception {
	Exception(const char* message)
		:
		fMessage(QString::fromUtf8(message))
	{
	}

	Exception(const QString& message)
		:
		fMessage(message)
	{
	}

	const QString& Message() const
	{
		return fMessage;
	}

private:
	QString	fMessage;
};


struct PlatformResourceParser::Tokenizer {
	Tokenizer(QFile& file)
		:
		fFile(file),
		fLine(),
		fLineOffset(0),
		fEof(false)
	{
	}

	bool HasMoreTokens()
	{
		Token token = NextToken();
		if (!token.IsValid())
			return false;
		PutToken(token);
		return true;
	}

	Token NextToken()
	{
		if (fPutToken.IsValid()) {
			Token token = fPutToken;
			fPutToken = Token();
			return token;
		}

		if (fEof)
			return Token();

		for (;;) {
			_SkipWhiteSpace();

			while (_IsEndOfLine()) {
				if (!_NextLine())
					return Token();
				_SkipWhiteSpace();
			}

			char tokenStartChar = fLine.at(fLineOffset++);
			switch (tokenStartChar) {
				case '"':
				{
					QByteArray string;
					while (!_IsEndOfLine()) {
						char c = fLine.at(fLineOffset++);
						if (c == '"') {
							return Token(kTokenQuotedString, string);
						}
						if (c == '\\') {
							if (_IsEndOfLine()) {
								// unterminated string
								throw Exception("unterminated string");
							}
							string.append(fLine.at(fLineOffset++));
						} else
							string.append(c);
					}

					throw Exception("unterminated string");
				}

				case '\'':
				{
					QByteArray string;
					while (!_IsEndOfLine()) {
						char c = fLine.at(fLineOffset++);
						if (c == '\'') {
							return Token(kTokenSingleQuotedString, string);
						}
						string.append(c);
					}

					throw Exception("unterminated string");
				}

				case '$':
				case '#':
				case '(':
				case ')':
				case '{':
				case '}':
				case ';':
				case ',':
				{
					return Token(tokenStartChar, QByteArray(1, tokenStartChar));
				}

				case '/':
					if (!_IsEndOfLine() && fLine.at(fLineOffset) == '/') {
						// comment -- skip rest of line
						fLine.clear();
						fLineOffset = 0;
						continue;
					}
					// fall through
				default:
				{
					QByteArray string;
					string.append(tokenStartChar);
					while (!_IsEndOfLine()) {
						char c = fLine.at(fLineOffset++);
						if (_IsUnquotedStringDelimiter(c)) {
							fLineOffset--;
							break;
						}
						string.append(c);
					}

					char type = kTokenUnquotedString;
					if (string == kResourceString)
						type = kTokenResource;

					return Token(type, string);
				}
			}
		}
	}

	Token ExpectToken(char type)
	{
		Token token = NextToken();
		if (token != type) {
			throw Exception(QString::fromUtf8("expected token type '%1', got "
				"'%2'").arg(type).arg(QString::fromUtf8(token.fString.data())));
		}
		return token;
	}

	bool CheckToken(char type, Token* _token = NULL)
	{
		Token token = NextToken();
		if (token == type) {
			if (_token != NULL)
				*_token = token;
			return true;
		}

		PutToken(token);
		return false;
	}

	bool CheckToken(const Token& expected)
	{
		Token token = NextToken();
		if (token == expected)
			return true;

		PutToken(token);
		return false;
	}

	void PutToken(const Token& token)
	{
		fPutToken = token;
	}

private:
	bool _IsEndOfLine() const
	{
		return fLineOffset >= fLine.length();
	}

	bool _NextLine()
	{
		fFile.unsetError();
		fLine = fFile.readLine();
		if (fLine.isEmpty()
			&& (fFile.atEnd() || fFile.error() != QFile::NoError)) {
			fEof = true;
			return false;
		}

		fLineOffset = 0;
		return true;
	}

	void _SkipWhiteSpace()
	{
		while (fLineOffset < fLine.length() && isspace(fLine.at(fLineOffset)))
			fLineOffset++;
	}

	bool _IsUnquotedStringDelimiter(char c)
	{
		if (isspace(c))
			return true;

		switch (c) {
			case '$':
			case '#':
			case '(':
			case ')':
			case '{':
			case '}':
			case ';':
			case ',':
				return true;
			default:
				return false;
		}
	}

private:
	QFile&		fFile;
	QByteArray	fLine;
	int			fLineOffset;
	bool		fEof;
	Token		fPutToken;
};


PlatformResourceParser::PlatformResourceParser()
{
}


status_t
PlatformResourceParser::ParseAppResources(BResources& resources)
{
	QFile rdefFile(QString::fromUtf8(":/resources/rdef"));
	if (!rdefFile.open(QFile::ReadOnly))
		return B_ENTRY_NOT_FOUND;

	Tokenizer tokenizer(rdefFile);
	fTokenizer = &tokenizer;

	try {
		fResources = &resources;
		fResources->Unset();
		_Parse();
	} catch (Exception& exception) {
		qDebug("caught exception: %s", exception.Message().toUtf8().data());
		return B_ERROR;
	}

	return B_OK;
}


void
PlatformResourceParser::_Parse()
{
	while (fTokenizer->HasMoreTokens())
		_ParseResource();
}


void
PlatformResourceParser::_ParseResource()
{
	fTokenizer->ExpectToken(kTokenResource);
	Token token = fTokenizer->NextToken();
	if (token == kTokenOpeningParenthesis) {
		// a numbered and named resource
		Token resourceId = fTokenizer->ExpectToken(kTokenUnquotedString);
		fTokenizer->ExpectToken(kTokenComma);
		Token resourceName = fTokenizer->ExpectToken(kTokenQuotedString);
		fTokenizer->ExpectToken(kTokenClosingParenthesis);

		if (fTokenizer->CheckToken(kTokenHash)
			&& fTokenizer->CheckToken(Token(kTokenSingleQuotedString,
				kVectorIconTypeString))
			&& fTokenizer->CheckToken(Token(kTokenUnquotedString, kArrayString))
			&& fTokenizer->CheckToken(kTokenOpeningBrace)) {
			type_code type = B_VECTOR_ICON_TYPE;

			QByteArray resourceData;
			for (;;) {
				token = fTokenizer->NextToken();
				if (token == kTokenClosingBrace)
					break;
				fTokenizer->PutToken(token);

				fTokenizer->ExpectToken(kTokenDollar);
				token = fTokenizer->ExpectToken(kTokenQuotedString);

				QByteArray lineData = QByteArray::fromHex(token.fString);
				resourceData.append(lineData);
			}

			fResources->AddResource(type, resourceId.fString.toInt(),
				resourceData, resourceName.fString);
		}
	} else
		fTokenizer->PutToken(token);

	// skip all tokens until the ';'
	for (;;) {
		token = fTokenizer->NextToken();
		if (token == kTokenSemicolon)
			break;
	}
}
