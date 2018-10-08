using namespace std;

#include "FuzzyQueryNodeProcessor.h"

namespace org::apache::lucene::queryparser::flexible::standard::processors
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;
using FuzzyQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FuzzyQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;
using FuzzyConfig =
    org::apache::lucene::queryparser::flexible::standard::config::FuzzyConfig;
using ConfigurationKeys = org::apache::lucene::queryparser::flexible::standard::
    config::StandardQueryConfigHandler::ConfigurationKeys;
using FuzzyQuery = org::apache::lucene::search::FuzzyQuery;

shared_ptr<QueryNode> FuzzyQueryNodeProcessor::postProcessNode(
    shared_ptr<QueryNode> node) 
{

  return node;
}

shared_ptr<QueryNode> FuzzyQueryNodeProcessor::preProcessNode(
    shared_ptr<QueryNode> node) 
{

  if (std::dynamic_pointer_cast<FuzzyQueryNode>(node) != nullptr) {
    shared_ptr<FuzzyQueryNode> fuzzyNode =
        std::static_pointer_cast<FuzzyQueryNode>(node);
    shared_ptr<QueryConfigHandler> config = getQueryConfigHandler();

    shared_ptr<Analyzer> analyzer =
        getQueryConfigHandler()->get(ConfigurationKeys::ANALYZER);
    if (analyzer != nullptr) {
      // because we call utf8ToString, this will only work with the default
      // TermToBytesRefAttribute
      wstring text = fuzzyNode->getTextAsString();
      text = analyzer->normalize(fuzzyNode->getFieldAsString(), text)
                 ->utf8ToString();
      fuzzyNode->setText(text);
    }

    shared_ptr<FuzzyConfig> fuzzyConfig = nullptr;

    if ((fuzzyConfig = config->get(ConfigurationKeys::FUZZY_CONFIG)) !=
        nullptr) {
      fuzzyNode->setPrefixLength(fuzzyConfig->getPrefixLength());

      if (fuzzyNode->getSimilarity() < 0) {
        fuzzyNode->setSimilarity(fuzzyConfig->getMinSimilarity());
      }

    } else if (fuzzyNode->getSimilarity() < 0) {
      throw invalid_argument(L"No FUZZY_CONFIG set in the config");
    }
  }

  return node;
}

deque<std::shared_ptr<QueryNode>> FuzzyQueryNodeProcessor::setChildrenOrder(
    deque<std::shared_ptr<QueryNode>> &children) 
{

  return children;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::processors