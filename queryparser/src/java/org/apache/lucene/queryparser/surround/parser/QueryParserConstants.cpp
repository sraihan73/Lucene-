using namespace std;

#include "QueryParserConstants.h"

namespace org::apache::lucene::queryparser::surround::parser
{

std::deque<wstring> const QueryParserConstants::tokenImage = {
    L"<EOF>",        L"<_NUM_CHAR>", L"<_TERM_CHAR>",  L"<_WHITESPACE>",
    L"\"*\"",        L"\"?\"",       L"<_DISTOP_NUM>", L"<token of kind 7>",
    L"<OR>",         L"<AND>",       L"<NOT>",         L"<W>",
    L"<N>",          L"\"(\"",       L"\")\"",         L"\",\"",
    L"\":\"",        L"\"^\"",       L"<TRUNCQUOTED>", L"<QUOTED>",
    L"<SUFFIXTERM>", L"<TRUNCTERM>", L"<TERM>",        L"<NUMBER>"};
}