#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queryparser::flexible::core::parser
{
class EscapeQuerySyntax;
}

namespace org::apache::lucene::queryparser::flexible::core::nodes
{
class QueryNode;
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
namespace org::apache::lucene::queryparser::flexible::core::nodes
{

using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;

/**
 * A {@link PathQueryNode} is used to store queries like
 * /company/USA/California /product/shoes/brown. QueryText are objects that
 * contain the text, begin position and end position in the query.
 * <p>
 * Example how the text parser creates these objects:
 * </p>
 * <pre class="prettyprint">
 * List values = ArrayList();
 * values.add(new PathQueryNode.QueryText("company", 1, 7));
 * values.add(new PathQueryNode.QueryText("USA", 9, 12));
 * values.add(new PathQueryNode.QueryText("California", 14, 23));
 * QueryNode q = new PathQueryNode(values);
 * </pre>
 */
class PathQueryNode : public QueryNodeImpl
{
  GET_CLASS_NAME(PathQueryNode)

  /**
   * Term text with a beginning and end position
   */
public:
  class QueryText : public std::enable_shared_from_this<QueryText>,
                    public Cloneable
  {
    GET_CLASS_NAME(QueryText)
  public:
    std::shared_ptr<std::wstring> value = nullptr;
    /**
     * != null The term's begin position.
     */
    int begin = 0;

    /**
     * The term's end position.
     */
    int end = 0;

    /**
     * @param value
     *          - text value
     * @param begin
     *          - position in the query string
     * @param end
     *          - position in the query string
     */
    // C++ TODO: No base class can be determined:
    QueryText(std::shared_ptr<std::wstring> value, int begin,
              int end); // super();

    std::shared_ptr<QueryText>
    clone()  override;

    /**
     * @return the value
     */
    virtual std::shared_ptr<std::wstring> getValue();

    /**
     * @return the begin
     */
    virtual int getBegin();

    /**
     * @return the end
     */
    virtual int getEnd();

    virtual std::wstring toString();
  };

private:
  std::deque<std::shared_ptr<QueryText>> values;

  /**
   * @param pathElements
   *          - List of QueryText objects
   */
public:
  PathQueryNode(std::deque<std::shared_ptr<QueryText>> &pathElements);

  /**
   * Returns the a List with all QueryText elements
   *
   * @return QueryText List size
   */
  virtual std::deque<std::shared_ptr<QueryText>> getPathElements();

  /**
   * Returns the a List with all QueryText elements
   */
  virtual void
  setPathElements(std::deque<std::shared_ptr<QueryText>> &elements);

  /**
   * Returns the a specific QueryText element
   *
   * @return QueryText List size
   */
  virtual std::shared_ptr<QueryText> getPathElement(int index);

  /**
   * Returns the std::wstring value of a specific QueryText element
   *
   * @return the std::wstring for a specific QueryText element
   */
  virtual std::shared_ptr<std::wstring> getFirstPathElement();

  /**
   * Returns a List QueryText element from position startIndex
   *
   * @return a List QueryText element from position startIndex
   */
  virtual std::deque<std::shared_ptr<QueryText>>
  getPathElements(int startIndex);

private:
  std::shared_ptr<std::wstring> getPathString();

public:
  std::shared_ptr<std::wstring>
  toQueryString(std::shared_ptr<EscapeQuerySyntax> escaper) override;

  virtual std::wstring toString();

  std::shared_ptr<QueryNode>
  cloneTree()  override;

protected:
  std::shared_ptr<PathQueryNode> shared_from_this()
  {
    return std::static_pointer_cast<PathQueryNode>(
        QueryNodeImpl::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::flexible::core::nodes
