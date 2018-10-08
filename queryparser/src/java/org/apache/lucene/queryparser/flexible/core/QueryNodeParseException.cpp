using namespace std;

#include "QueryNodeParseException.h"

namespace org::apache::lucene::queryparser::flexible::core
{
using Message = org::apache::lucene::queryparser::flexible::messages::Message;
using MessageImpl =
    org::apache::lucene::queryparser::flexible::messages::MessageImpl;
using QueryParserMessages = org::apache::lucene::queryparser::flexible::core::
    messages::QueryParserMessages;
using SyntaxParser =
    org::apache::lucene::queryparser::flexible::core::parser::SyntaxParser;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;

QueryNodeParseException::QueryNodeParseException(shared_ptr<Message> message)
    : QueryNodeException(message)
{
}

QueryNodeParseException::QueryNodeParseException(runtime_error throwable)
    : QueryNodeException(throwable)
{
}

QueryNodeParseException::QueryNodeParseException(shared_ptr<Message> message,
                                                 runtime_error throwable)
    : QueryNodeException(message, throwable)
{
}

void QueryNodeParseException::setQuery(shared_ptr<std::wstring> query)
{
  this->query = query;
  this->message = make_shared<MessageImpl>(
      QueryParserMessages::INVALID_SYNTAX_CANNOT_PARSE, query, L"");
}

shared_ptr<std::wstring> QueryNodeParseException::getQuery()
{
  return this->query;
}

void QueryNodeParseException::setErrorToken(const wstring &errorToken)
{
  this->errorToken = errorToken;
}

wstring QueryNodeParseException::getErrorToken() { return this->errorToken; }

void QueryNodeParseException::setNonLocalizedMessage(
    shared_ptr<Message> message)
{
  this->message = message;
}

int QueryNodeParseException::getBeginLine() { return this->beginLine; }

int QueryNodeParseException::getBeginColumn() { return this->beginColumn; }

void QueryNodeParseException::setBeginLine(int beginLine)
{
  this->beginLine = beginLine;
}

void QueryNodeParseException::setBeginColumn(int beginColumn)
{
  this->beginColumn = beginColumn;
}
} // namespace org::apache::lucene::queryparser::flexible::core