using namespace std;

#include "RegexpQueryNodeProcessor.h"

namespace org::apache::lucene::queryparser::flexible::standard::processors
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;
using ConfigurationKeys = org::apache::lucene::queryparser::flexible::standard::
    config::StandardQueryConfigHandler::ConfigurationKeys;
using RegexpQueryNode = org::apache::lucene::queryparser::flexible::standard::
    nodes::RegexpQueryNode;

shared_ptr<QueryNode> RegexpQueryNodeProcessor::preProcessNode(
    shared_ptr<QueryNode> node) 
{
  return node;
}

shared_ptr<QueryNode> RegexpQueryNodeProcessor::postProcessNode(
    shared_ptr<QueryNode> node) 
{
  if (std::dynamic_pointer_cast<RegexpQueryNode>(node) != nullptr) {
    shared_ptr<RegexpQueryNode> regexpNode =
        std::static_pointer_cast<RegexpQueryNode>(node);
    shared_ptr<Analyzer> analyzer =
        getQueryConfigHandler()->get(ConfigurationKeys::ANALYZER);
    if (analyzer != nullptr) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring text = regexpNode->getText()->toString();
      // because we call utf8ToString, this will only work with the default
      // TermToBytesRefAttribute
      text = analyzer->normalize(regexpNode->getFieldAsString(), text)
                 ->utf8ToString();
      regexpNode->setText(text);
    }
  }
  return node;
}

deque<std::shared_ptr<QueryNode>> RegexpQueryNodeProcessor::setChildrenOrder(
    deque<std::shared_ptr<QueryNode>> &children) 
{
  return children;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::processors