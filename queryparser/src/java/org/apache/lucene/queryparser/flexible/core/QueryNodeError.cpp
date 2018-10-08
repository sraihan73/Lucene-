using namespace std;

#include "QueryNodeError.h"

namespace org::apache::lucene::queryparser::flexible::core
{
using Message = org::apache::lucene::queryparser::flexible::messages::Message;
using NLSException =
    org::apache::lucene::queryparser::flexible::messages::NLSException;

QueryNodeError::QueryNodeError(shared_ptr<Message> message)
    : Error(message->getKey())
{

  this->message = message;
}

QueryNodeError::QueryNodeError(runtime_error throwable) : Error(throwable) {}

QueryNodeError::QueryNodeError(shared_ptr<Message> message,
                               runtime_error throwable)
    : Error(message->getKey(), throwable)
{

  this->message = message;
}

shared_ptr<Message> QueryNodeError::getMessageObject() { return this->message; }
} // namespace org::apache::lucene::queryparser::flexible::core