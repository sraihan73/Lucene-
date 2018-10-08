using namespace std;

#include "SpanQueryBuilder.h"

namespace org::apache::lucene::queryparser::xml::builders
{
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using QueryBuilder = org::apache::lucene::queryparser::xml::QueryBuilder;
using org::w3c::dom::Element;
} // namespace org::apache::lucene::queryparser::xml::builders