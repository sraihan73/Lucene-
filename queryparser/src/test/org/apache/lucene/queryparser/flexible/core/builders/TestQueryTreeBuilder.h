#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/QueryNodeException.h"

#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/parser/EscapeQuerySyntax.h"
#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/nodes/QueryNode.h"

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
namespace org::apache::lucene::queryparser::flexible::core::builders
{

using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeImpl =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNodeImpl;
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestQueryTreeBuilder : public LuceneTestCase
{
  GET_CLASS_NAME(TestQueryTreeBuilder)

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSetFieldBuilder() throws
  // org.apache.lucene.queryparser.flexible.core.QueryNodeException
  virtual void testSetFieldBuilder() ;

private:
  class DummyQueryNodeInterface : public QueryNode
  {
    GET_CLASS_NAME(DummyQueryNodeInterface)
  };

private:
  class AbstractDummyQueryNode : public QueryNodeImpl,
                                 public DummyQueryNodeInterface
  {
    GET_CLASS_NAME(AbstractDummyQueryNode)

  protected:
    std::shared_ptr<AbstractDummyQueryNode> shared_from_this()
    {
      return std::static_pointer_cast<AbstractDummyQueryNode>(
          org.apache.lucene.queryparser.flexible.core.nodes
              .QueryNodeImpl::shared_from_this());
    }
  };

private:
  class DummyQueryNode : public AbstractDummyQueryNode
  {
    GET_CLASS_NAME(DummyQueryNode)

  public:
    std::shared_ptr<std::wstring> toQueryString(
        std::shared_ptr<EscapeQuerySyntax> escapeSyntaxParser) override;

  protected:
    std::shared_ptr<DummyQueryNode> shared_from_this()
    {
      return std::static_pointer_cast<DummyQueryNode>(
          AbstractDummyQueryNode::shared_from_this());
    }
  };

private:
  class DummyBuilder : public std::enable_shared_from_this<DummyBuilder>,
                       public QueryBuilder
  {
    GET_CLASS_NAME(DummyBuilder)

  public:
    std::any build(std::shared_ptr<QueryNode> queryNode) throw(
        QueryNodeException) override;
  };

protected:
  std::shared_ptr<TestQueryTreeBuilder> shared_from_this()
  {
    return std::static_pointer_cast<TestQueryTreeBuilder>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/core/builders/
