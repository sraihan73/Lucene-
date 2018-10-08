using namespace std;

#include "MatchAllDocsQueryBuilder.h"

namespace org::apache::lucene::queryparser::xml::builders
{
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Query = org::apache::lucene::search::Query;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using QueryBuilder = org::apache::lucene::queryparser::xml::QueryBuilder;
using org::w3c::dom::Element;

shared_ptr<Query>
MatchAllDocsQueryBuilder::getQuery(shared_ptr<Element> e) 
{
  return make_shared<MatchAllDocsQuery>();
}
} // namespace org::apache::lucene::queryparser::xml::builders