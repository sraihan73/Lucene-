#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
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
namespace org::apache::lucene::queryparser::flexible::standard::nodes
{

using FieldableNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldableNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeImpl =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNodeImpl;
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;

/**
 * A {@link MultiPhraseQueryNode} indicates that its children should be used to
 * build a {@link MultiPhraseQuery} instead of {@link PhraseQuery}.
 */
class MultiPhraseQueryNode : public QueryNodeImpl, public FieldableNode
{
  GET_CLASS_NAME(MultiPhraseQueryNode)

public:
  MultiPhraseQueryNode();

  virtual std::wstring toString();

  std::shared_ptr<std::wstring>
  toQueryString(std::shared_ptr<EscapeQuerySyntax> escapeSyntaxParser) override;

  std::shared_ptr<QueryNode>
  cloneTree()  override;

  std::shared_ptr<std::wstring> getField() override;

  void setField(std::shared_ptr<std::wstring> fieldName) override;

protected:
  std::shared_ptr<MultiPhraseQueryNode> shared_from_this()
  {
    return std::static_pointer_cast<MultiPhraseQueryNode>(
        org.apache.lucene.queryparser.flexible.core.nodes
            .QueryNodeImpl::shared_from_this());
  }
}; // end class MultitermQueryNode

} // namespace org::apache::lucene::queryparser::flexible::standard::nodes
