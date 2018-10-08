using namespace std;

#include "OpenRangeQueryNodeProcessor.h"

namespace org::apache::lucene::queryparser::flexible::standard::processors
{
using TermRangeQuery = org::apache::lucene::search::TermRangeQuery;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using FieldQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;
using UnescapedCharSequence = org::apache::lucene::queryparser::flexible::core::
    util::UnescapedCharSequence;
using TermRangeQueryNode = org::apache::lucene::queryparser::flexible::
    standard::nodes::TermRangeQueryNode;
const wstring OpenRangeQueryNodeProcessor::OPEN_RANGE_TOKEN = L"*";

OpenRangeQueryNodeProcessor::OpenRangeQueryNodeProcessor() {}

shared_ptr<QueryNode> OpenRangeQueryNodeProcessor::postProcessNode(
    shared_ptr<QueryNode> node) 
{

  if (std::dynamic_pointer_cast<TermRangeQueryNode>(node) != nullptr) {
    shared_ptr<TermRangeQueryNode> rangeNode =
        std::static_pointer_cast<TermRangeQueryNode>(node);
    shared_ptr<FieldQueryNode> lowerNode = rangeNode->getLowerBound();
    shared_ptr<FieldQueryNode> upperNode = rangeNode->getUpperBound();
    shared_ptr<std::wstring> lowerText = lowerNode->getText();
    shared_ptr<std::wstring> upperText = upperNode->getText();

    if (OPEN_RANGE_TOKEN == upperNode->getTextAsString() &&
        (!(std::dynamic_pointer_cast<UnescapedCharSequence>(upperText) !=
           nullptr) ||
         !(std::static_pointer_cast<UnescapedCharSequence>(upperText))
              ->wasEscaped(0))) {
      upperText = L"";
    }

    if (OPEN_RANGE_TOKEN == lowerNode->getTextAsString() &&
        (!(std::dynamic_pointer_cast<UnescapedCharSequence>(lowerText) !=
           nullptr) ||
         !(std::static_pointer_cast<UnescapedCharSequence>(lowerText))
              ->wasEscaped(0))) {
      lowerText = L"";
    }

    lowerNode->setText(lowerText);
    upperNode->setText(upperText);
  }

  return node;
}

shared_ptr<QueryNode> OpenRangeQueryNodeProcessor::preProcessNode(
    shared_ptr<QueryNode> node) 
{
  return node;
}

deque<std::shared_ptr<QueryNode>>
OpenRangeQueryNodeProcessor::setChildrenOrder(
    deque<std::shared_ptr<QueryNode>> &children) 
{
  return children;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::processors