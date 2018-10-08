using namespace std;

#include "FuzzyQueryNodeBuilder.h"

namespace org::apache::lucene::queryparser::flexible::standard::builders
{
using Term = org::apache::lucene::index::Term;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using FuzzyQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FuzzyQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using FuzzyQuery = org::apache::lucene::search::FuzzyQuery;

FuzzyQueryNodeBuilder::FuzzyQueryNodeBuilder()
{
  // empty constructor
}

shared_ptr<FuzzyQuery> FuzzyQueryNodeBuilder::build(
    shared_ptr<QueryNode> queryNode) 
{
  shared_ptr<FuzzyQueryNode> fuzzyNode =
      std::static_pointer_cast<FuzzyQueryNode>(queryNode);
  wstring text = fuzzyNode->getTextAsString();

  int numEdits = FuzzyQuery::floatToEdits(
      fuzzyNode->getSimilarity(), text.codePointCount(0, text.length()));

  return make_shared<FuzzyQuery>(
      make_shared<Term>(fuzzyNode->getFieldAsString(),
                        fuzzyNode->getTextAsString()),
      numEdits, fuzzyNode->getPrefixLength());
}
} // namespace org::apache::lucene::queryparser::flexible::standard::builders