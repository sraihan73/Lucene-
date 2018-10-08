#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/flexible/messages/Message.h"

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::queryparser::flexible::core
{

using Message = org::apache::lucene::queryparser::flexible::messages::Message;
using MessageImpl =
    org::apache::lucene::queryparser::flexible::messages::MessageImpl;
using NLSException =
    org::apache::lucene::queryparser::flexible::messages::NLSException;
using QueryParserMessages = org::apache::lucene::queryparser::flexible::core::
    messages::QueryParserMessages;

/**
 * <p>
 * This exception should be thrown if something wrong happens when dealing with
 * {@link QueryNode}s.
 * </p>
 * <p>
 * It also supports NLS messages.
 * </p>
 *
 * @see Message
 * @see NLS
 * @see NLSException
 * @see QueryNode
 */
class QueryNodeException : public std::runtime_error, public NLSException
{
  GET_CLASS_NAME(QueryNodeException)

protected:
  std::shared_ptr<Message> message =
      std::make_shared<MessageImpl>(QueryParserMessages::EMPTY_MESSAGE);

public:
  QueryNodeException(std::shared_ptr<Message> message);

  QueryNodeException(std::runtime_error throwable);

  QueryNodeException(std::shared_ptr<Message> message,
                     std::runtime_error throwable);

  std::shared_ptr<Message> getMessageObject() override;

  std::wstring getMessage() override;

  std::wstring getLocalizedMessage() override;

  virtual std::wstring getLocalizedMessage(std::shared_ptr<Locale> locale);

  virtual std::wstring toString();

protected:
  std::shared_ptr<QueryNodeException> shared_from_this()
  {
    return std::static_pointer_cast<QueryNodeException>(
        Exception::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/core/
