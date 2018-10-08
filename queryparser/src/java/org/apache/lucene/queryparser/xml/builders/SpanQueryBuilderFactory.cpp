using namespace std;

#include "SpanQueryBuilderFactory.h"

namespace org::apache::lucene::queryparser::xml::builders
{
using Query = org::apache::lucene::search::Query;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using org::w3c::dom::Element;

shared_ptr<Query>
SpanQueryBuilderFactory::getQuery(shared_ptr<Element> e) 
{
  return getSpanQuery(e);
}

void SpanQueryBuilderFactory::addBuilder(const wstring &nodeName,
                                         shared_ptr<SpanQueryBuilder> builder)
{
  builders.emplace(nodeName, builder);
}

shared_ptr<SpanQuery> SpanQueryBuilderFactory::getSpanQuery(
    shared_ptr<Element> e) 
{
  shared_ptr<SpanQueryBuilder> builder = builders[e->getNodeName()];
  if (builder == nullptr) {
    throw make_shared<ParserException>(
        L"No SpanQueryObjectBuilder defined for node " + e->getNodeName());
  }
  return builder->getSpanQuery(e);
}
} // namespace org::apache::lucene::queryparser::xml::builders