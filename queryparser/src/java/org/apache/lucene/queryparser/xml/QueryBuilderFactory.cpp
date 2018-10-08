using namespace std;

#include "QueryBuilderFactory.h"

namespace org::apache::lucene::queryparser::xml
{
using Query = org::apache::lucene::search::Query;
using org::w3c::dom::Element;

shared_ptr<Query>
QueryBuilderFactory::getQuery(shared_ptr<Element> n) 
{
  shared_ptr<QueryBuilder> builder = builders[n->getNodeName()];
  if (builder == nullptr) {
    throw make_shared<ParserException>(
        L"No QueryObjectBuilder defined for node " + n->getNodeName());
  }
  return builder->getQuery(n);
}

void QueryBuilderFactory::addBuilder(const wstring &nodeName,
                                     shared_ptr<QueryBuilder> builder)
{
  builders.emplace(nodeName, builder);
}

shared_ptr<QueryBuilder>
QueryBuilderFactory::getQueryBuilder(const wstring &nodeName)
{
  return builders[nodeName];
}
} // namespace org::apache::lucene::queryparser::xml