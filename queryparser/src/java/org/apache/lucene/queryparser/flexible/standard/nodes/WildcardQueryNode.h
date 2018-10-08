#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/nodes/FieldQueryNode.h"

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
namespace org::apache::lucene::queryparser::flexible::standard::nodes
{

using FieldQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldQueryNode;
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;

/**
 * A {@link WildcardQueryNode} represents wildcard query This does not apply to
 * phrases. Examples: a*b*c Fl?w? m?ke*g
 */
class WildcardQueryNode : public FieldQueryNode
{
  GET_CLASS_NAME(WildcardQueryNode)

  /**
   * @param field
   *          - field name
   * @param text
   *          - value that contains one or more wild card characters (? or *)
   * @param begin
   *          - position in the query string
   * @param end
   *          - position in the query string
   */
public:
  WildcardQueryNode(std::shared_ptr<std::wstring> field,
                    std::shared_ptr<std::wstring> text, int begin, int end);

  WildcardQueryNode(std::shared_ptr<FieldQueryNode> fqn);

  std::shared_ptr<std::wstring>
  toQueryString(std::shared_ptr<EscapeQuerySyntax> escaper) override;

  virtual std::wstring toString();

  std::shared_ptr<WildcardQueryNode>
  cloneTree()  override;

protected:
  std::shared_ptr<WildcardQueryNode> shared_from_this()
  {
    return std::static_pointer_cast<WildcardQueryNode>(
        org.apache.lucene.queryparser.flexible.core.nodes
            .FieldQueryNode::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/standard/nodes/
