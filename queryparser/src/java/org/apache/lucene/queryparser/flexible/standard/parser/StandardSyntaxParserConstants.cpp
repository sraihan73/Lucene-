using namespace std;

#include "StandardSyntaxParserConstants.h"

namespace org::apache::lucene::queryparser::flexible::standard::parser
{

std::deque<wstring> const StandardSyntaxParserConstants::tokenImage = {
    L"<EOF>",           L"<_NUM_CHAR>",
    L"<_ESCAPED_CHAR>", L"<_TERM_START_CHAR>",
    L"<_TERM_CHAR>",    L"<_WHITESPACE>",
    L"<_QUOTED_CHAR>",  L"<token of kind 7>",
    L"<AND>",           L"<OR>",
    L"<NOT>",           L"\"+\"",
    L"\"-\"",           L"\"(\"",
    L"\")\"",           L"\":\"",
    L"\"=\"",           L"\"<\"",
    L"\"<=\"",          L"\">\"",
    L"\">=\"",          L"\"^\"",
    L"<QUOTED>",        L"<TERM>",
    L"<FUZZY_SLOP>",    L"<REGEXPTERM>",
    L"\"[\"",           L"\"{\"",
    L"<NUMBER>",        L"\"TO\"",
    L"\"]\"",           L"\"}\"",
    L"<RANGE_QUOTED>",  L"<RANGE_GOOP>"};
}