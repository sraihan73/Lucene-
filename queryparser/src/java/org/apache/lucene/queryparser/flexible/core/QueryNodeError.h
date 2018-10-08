#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queryparser::flexible::messages
{
class Message;
}

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
using NLSException =
    org::apache::lucene::queryparser::flexible::messages::NLSException;

/**
 * Error class with NLS support
 *
 * @see org.apache.lucene.queryparser.flexible.messages.NLS
 * @see org.apache.lucene.queryparser.flexible.messages.Message
 */
class QueryNodeError : public Error, public NLSException
{
  GET_CLASS_NAME(QueryNodeError)
private:
  std::shared_ptr<Message> message;

  /**
   * @param message
   *          - NLS Message Object
   */
public:
  QueryNodeError(std::shared_ptr<Message> message);

  /**
   * @param throwable
   *          - @see java.lang.Error
   */
  QueryNodeError(std::runtime_error throwable);

  /**
   * @param message
   *          - NLS Message Object
   * @param throwable
   *          - @see java.lang.Error
   */
  QueryNodeError(std::shared_ptr<Message> message,
                 std::runtime_error throwable);

  /*
   * (non-Javadoc)
   *
   * @see org.apache.lucene.messages.NLSException#getMessageObject()
   */
  std::shared_ptr<Message> getMessageObject() override;

protected:
  std::shared_ptr<QueryNodeError> shared_from_this()
  {
    return std::static_pointer_cast<QueryNodeError>(Error::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::flexible::core
