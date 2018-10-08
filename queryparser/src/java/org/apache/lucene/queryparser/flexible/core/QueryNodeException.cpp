using namespace std;

#include "QueryNodeException.h"

namespace org::apache::lucene::queryparser::flexible::core
{
using Message = org::apache::lucene::queryparser::flexible::messages::Message;
using MessageImpl =
    org::apache::lucene::queryparser::flexible::messages::MessageImpl;
using NLS = org::apache::lucene::queryparser::flexible::messages::NLS;
using NLSException =
    org::apache::lucene::queryparser::flexible::messages::NLSException;
using QueryParserMessages = org::apache::lucene::queryparser::flexible::core::
    messages::QueryParserMessages;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;

QueryNodeException::QueryNodeException(shared_ptr<Message> message)
    : Exception(message->getKey())
{

  this->message = message;
}

QueryNodeException::QueryNodeException(runtime_error throwable)
    : Exception(throwable)
{
}

QueryNodeException::QueryNodeException(shared_ptr<Message> message,
                                       runtime_error throwable)
    : Exception(message->getKey(), throwable)
{

  this->message = message;
}

shared_ptr<Message> QueryNodeException::getMessageObject()
{
  return this->message;
}

wstring QueryNodeException::getMessage() { return getLocalizedMessage(); }

wstring QueryNodeException::getLocalizedMessage()
{
  return getLocalizedMessage(Locale::getDefault());
}

wstring QueryNodeException::getLocalizedMessage(shared_ptr<Locale> locale)
{
  return this->message->getLocalizedMessage(locale);
}

wstring QueryNodeException::toString()
{
  return this->message->getKey() + L": " + getLocalizedMessage();
}
} // namespace org::apache::lucene::queryparser::flexible::core