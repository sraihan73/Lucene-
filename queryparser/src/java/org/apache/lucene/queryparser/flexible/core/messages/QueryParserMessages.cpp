using namespace std;

#include "QueryParserMessages.h"

namespace org::apache::lucene::queryparser::flexible::core::messages
{
using NLS = org::apache::lucene::queryparser::flexible::messages::NLS;
const wstring QueryParserMessages::BUNDLE_NAME =
    QueryParserMessages::typeid->getName();

QueryParserMessages::QueryParserMessages()
{
  // Do not instantiate
}

QueryParserMessages::StaticConstructor::StaticConstructor()
{
  // register all string ids with NLS class and initialize static string
  // values
  NLS::initializeMessages(BUNDLE_NAME, QueryParserMessages::typeid);
}

QueryParserMessages::StaticConstructor QueryParserMessages::staticConstructor;
wstring QueryParserMessages::INVALID_SYNTAX;
wstring QueryParserMessages::INVALID_SYNTAX_CANNOT_PARSE;
wstring QueryParserMessages::INVALID_SYNTAX_FUZZY_LIMITS;
wstring QueryParserMessages::INVALID_SYNTAX_FUZZY_EDITS;
wstring QueryParserMessages::INVALID_SYNTAX_ESCAPE_UNICODE_TRUNCATION;
wstring QueryParserMessages::INVALID_SYNTAX_ESCAPE_CHARACTER;
wstring QueryParserMessages::INVALID_SYNTAX_ESCAPE_NONE_HEX_UNICODE;
wstring QueryParserMessages::NODE_ACTION_NOT_SUPPORTED;
wstring QueryParserMessages::PARAMETER_VALUE_NOT_SUPPORTED;
wstring QueryParserMessages::LUCENE_QUERY_CONVERSION_ERROR;
wstring QueryParserMessages::EMPTY_MESSAGE;
wstring QueryParserMessages::WILDCARD_NOT_SUPPORTED;
wstring QueryParserMessages::TOO_MANY_BOOLEAN_CLAUSES;
wstring QueryParserMessages::LEADING_WILDCARD_NOT_ALLOWED;
wstring QueryParserMessages::COULD_NOT_PARSE_NUMBER;
wstring QueryParserMessages::NUMBER_CLASS_NOT_SUPPORTED_BY_NUMERIC_RANGE_QUERY;
wstring QueryParserMessages::UNSUPPORTED_NUMERIC_DATA_TYPE;
wstring QueryParserMessages::NUMERIC_CANNOT_BE_EMPTY;
} // namespace org::apache::lucene::queryparser::flexible::core::messages