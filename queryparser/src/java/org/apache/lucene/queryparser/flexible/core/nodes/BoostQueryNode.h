#pragma once
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/nodes/QueryNode.h"

#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/parser/EscapeQuerySyntax.h"

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
 * A {@link BoostQueryNode} boosts the QueryNode tree which is under this node.
 * So, it must only and always have one child.
 *
 * The boost value may vary from 0.0 to 1.0.
 *
 */
class BoostQueryNode : public QueryNodeImpl
{
  GET_CLASS_NAME(BoostQueryNode)

private:
  float value = 0;

  /**
   * Constructs a boost node
   *
   * @param query
   *          the query to be boosted
   * @param value
   *          the boost value, it may vary from 0.0 to 1.0
   */
public:
  BoostQueryNode(std::shared_ptr<QueryNode> query, float value);

  /**
   * Returns the single child which this node boosts.
   *
   * @return the single child which this node boosts
   */
  virtual std::shared_ptr<QueryNode> getChild();

  /**
   * Returns the boost value. It may vary from 0.0 to 1.0.
   *
   * @return the boost value
   */
  virtual float getValue();

  /**
   * Returns the boost value parsed to a string.
   *
   * @return the parsed value
   */
private:
  std::shared_ptr<std::wstring> getValueString();

public:
  virtual std::wstring toString();

  std::shared_ptr<std::wstring>
  toQueryString(std::shared_ptr<EscapeQuerySyntax> escapeSyntaxParser) override;

  std::shared_ptr<QueryNode>
  cloneTree()  override;

protected:
  std::shared_ptr<BoostQueryNode> shared_from_this()
  {
    return std::static_pointer_cast<BoostQueryNode>(
        QueryNodeImpl::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/core/nodes/
