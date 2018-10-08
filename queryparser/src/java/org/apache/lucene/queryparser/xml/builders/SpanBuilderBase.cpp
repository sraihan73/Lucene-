using namespace std;

#include "SpanBuilderBase.h"

namespace org::apache::lucene::queryparser::xml::builders
{
using Query = org::apache::lucene::search::Query;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using org::w3c::dom::Element;

shared_ptr<Query>
SpanBuilderBase::getQuery(shared_ptr<Element> e) 
{
  return getSpanQuery(e);
}
} // namespace org::apache::lucene::queryparser::xml::builders