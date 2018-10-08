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

/**
 * This should be thrown when an exception happens during the query parsing from
 * string to the query node tree.
 *
 * @see QueryNodeException
 * @see SyntaxParser
 * @see QueryNode
 */
class QueryNodeParseException : public QueryNodeException
{
  GET_CLASS_NAME(QueryNodeParseException)

private:
  std::shared_ptr<std::wstring> query;

  int beginColumn = -1;

  int beginLine = -1;

  std::wstring errorToken = L"";

public:
  QueryNodeParseException(std::shared_ptr<Message> message);

  QueryNodeParseException(std::runtime_error throwable);

  QueryNodeParseException(std::shared_ptr<Message> message,
                          std::runtime_error throwable);

  virtual void setQuery(std::shared_ptr<std::wstring> query);

  virtual std::shared_ptr<std::wstring> getQuery();

  /**
   * @param errorToken
   *          the errorToken in the query
   */
protected:
  virtual void setErrorToken(const std::wstring &errorToken);

public:
  virtual std::wstring getErrorToken();

  virtual void setNonLocalizedMessage(std::shared_ptr<Message> message);

  /**
   * For EndOfLine and EndOfFile ("&lt;EOF&gt;") parsing problems the last char
   * in the string is returned For the case where the parser is not able to
   * figure out the line and column number -1 will be returned
   *
   * @return line where the problem was found
   */
  virtual int getBeginLine();

  /**
   * For EndOfLine and EndOfFile ("&lt;EOF&gt;") parsing problems the last char
   * in the string is returned For the case where the parser is not able to
   * figure out the line and column number -1 will be returned
   *
   * @return column of the first char where the problem was found
   */
  virtual int getBeginColumn();

  /**
   * @param beginLine
   *          the beginLine to set
   */
protected:
  virtual void setBeginLine(int beginLine);

  /**
   * @param beginColumn
   *          the beginColumn to set
   */
  virtual void setBeginColumn(int beginColumn);

protected:
  std::shared_ptr<QueryNodeParseException> shared_from_this()
  {
    return std::static_pointer_cast<QueryNodeParseException>(
        QueryNodeException::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/core/
