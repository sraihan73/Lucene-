using namespace std;

#include "ExtensionQuery.h"

namespace org::apache::lucene::queryparser::ext
{
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;

ExtensionQuery::ExtensionQuery(shared_ptr<QueryParser> topLevelParser,
                               const wstring &field,
                               const wstring &rawQueryString)
    : field(field), rawQueryString(rawQueryString),
      topLevelParser(topLevelParser)
{
}

wstring ExtensionQuery::getField() { return field; }

wstring ExtensionQuery::getRawQueryString() { return rawQueryString; }

shared_ptr<QueryParser> ExtensionQuery::getTopLevelParser()
{
  return topLevelParser;
}
} // namespace org::apache::lucene::queryparser::ext