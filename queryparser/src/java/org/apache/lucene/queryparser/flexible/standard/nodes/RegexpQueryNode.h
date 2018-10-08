#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::queryparser::flexible::core::parser
{
class EscapeQuerySyntax;
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
namespace org::apache::lucene::queryparser::flexible::standard::nodes
{

using FieldableNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldableNode;
using QueryNodeImpl =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNodeImpl;
using TextableQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::TextableQueryNode;
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * A {@link RegexpQueryNode} represents {@link RegexpQuery} query Examples:
 * /[a-z]|[0-9]/
 */
class RegexpQueryNode : public QueryNodeImpl,
                        public TextableQueryNode,
                        public FieldableNode
{
  GET_CLASS_NAME(RegexpQueryNode)
private:
  std::shared_ptr<std::wstring> text;
  std::shared_ptr<std::wstring> field;
  /**
   * @param field
   *          - field name
   * @param text
   *          - value that contains a regular expression
   * @param begin
   *          - position in the query string
   * @param end
   *          - position in the query string
   */
public:
  RegexpQueryNode(std::shared_ptr<std::wstring> field,
                  std::shared_ptr<std::wstring> text, int begin, int end);

  virtual std::shared_ptr<BytesRef> textToBytesRef();

  virtual std::wstring toString();

  std::shared_ptr<RegexpQueryNode>
  cloneTree()  override;

  std::shared_ptr<std::wstring> getText() override;

  void setText(std::shared_ptr<std::wstring> text) override;

  std::shared_ptr<std::wstring> getField() override;

  virtual std::wstring getFieldAsString();

  void setField(std::shared_ptr<std::wstring> field) override;

  std::shared_ptr<std::wstring>
  toQueryString(std::shared_ptr<EscapeQuerySyntax> escapeSyntaxParser) override;

protected:
  std::shared_ptr<RegexpQueryNode> shared_from_this()
  {
    return std::static_pointer_cast<RegexpQueryNode>(
        org.apache.lucene.queryparser.flexible.core.nodes
            .QueryNodeImpl::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::flexible::standard::nodes
