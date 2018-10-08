using namespace std;

#include "QueryParserHelper.h"

namespace org::apache::lucene::queryparser::flexible::core
{
using QueryBuilder =
    org::apache::lucene::queryparser::flexible::core::builders::QueryBuilder;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using SyntaxParser =
    org::apache::lucene::queryparser::flexible::core::parser::SyntaxParser;
using QueryNodeProcessor = org::apache::lucene::queryparser::flexible::core::
    processors::QueryNodeProcessor;

QueryParserHelper::QueryParserHelper(
    shared_ptr<QueryConfigHandler> queryConfigHandler,
    shared_ptr<SyntaxParser> syntaxParser,
    shared_ptr<QueryNodeProcessor> processor, shared_ptr<QueryBuilder> builder)
{
  this->syntaxParser = syntaxParser;
  this->config = queryConfigHandler;
  this->processor = processor;
  this->builder = builder;

  if (processor != nullptr) {
    processor->setQueryConfigHandler(queryConfigHandler);
  }
}

shared_ptr<QueryNodeProcessor> QueryParserHelper::getQueryNodeProcessor()
{
  return processor;
}

void QueryParserHelper::setQueryNodeProcessor(
    shared_ptr<QueryNodeProcessor> processor)
{
  this->processor = processor;
  this->processor->setQueryConfigHandler(getQueryConfigHandler());
}

void QueryParserHelper::setSyntaxParser(shared_ptr<SyntaxParser> syntaxParser)
{

  if (syntaxParser == nullptr) {
    throw invalid_argument(L"textParser should not be null!");
  }

  this->syntaxParser = syntaxParser;
}

void QueryParserHelper::setQueryBuilder(shared_ptr<QueryBuilder> queryBuilder)
{

  if (queryBuilder == nullptr) {
    throw invalid_argument(L"queryBuilder should not be null!");
  }

  this->builder = queryBuilder;
}

shared_ptr<QueryConfigHandler> QueryParserHelper::getQueryConfigHandler()
{
  return config;
}

shared_ptr<QueryBuilder> QueryParserHelper::getQueryBuilder()
{
  return this->builder;
}

shared_ptr<SyntaxParser> QueryParserHelper::getSyntaxParser()
{
  return this->syntaxParser;
}

void QueryParserHelper::setQueryConfigHandler(
    shared_ptr<QueryConfigHandler> config)
{
  this->config = config;
  shared_ptr<QueryNodeProcessor> processor = getQueryNodeProcessor();

  if (processor != nullptr) {
    processor->setQueryConfigHandler(config);
  }
}

any QueryParserHelper::parse(
    const wstring &query, const wstring &defaultField) 
{
  shared_ptr<QueryNode> queryTree =
      getSyntaxParser()->parse(query, defaultField);

  shared_ptr<QueryNodeProcessor> processor = getQueryNodeProcessor();

  if (processor != nullptr) {
    queryTree = processor->process(queryTree);
  }

  return getQueryBuilder()->build(queryTree);
}
} // namespace org::apache::lucene::queryparser::flexible::core