using namespace std;

#include "DummyQueryNodeBuilder.h"

namespace org::apache::lucene::queryparser::flexible::standard::builders
{
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryTreeBuilder = org::apache::lucene::queryparser::flexible::core::
    builders::QueryTreeBuilder;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using TermQuery = org::apache::lucene::search::TermQuery;

DummyQueryNodeBuilder::DummyQueryNodeBuilder()
{
  // empty constructor
}

shared_ptr<TermQuery> DummyQueryNodeBuilder::build(
    shared_ptr<QueryNode> queryNode) 
{
  return nullptr;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::builders