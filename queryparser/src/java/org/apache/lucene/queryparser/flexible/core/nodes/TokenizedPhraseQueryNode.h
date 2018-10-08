#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
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
namespace org::apache::lucene::queryparser::flexible::core::nodes
{

using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;

/**
 * A {@link TokenizedPhraseQueryNode} represents a node created by a code that
 * tokenizes/lemmatizes/analyzes.
 */
class TokenizedPhraseQueryNode : public QueryNodeImpl, public FieldableNode
{
  GET_CLASS_NAME(TokenizedPhraseQueryNode)

public:
  TokenizedPhraseQueryNode();

  virtual std::wstring toString();

  // This text representation is not re-parseable
  std::shared_ptr<std::wstring>
  toQueryString(std::shared_ptr<EscapeQuerySyntax> escapeSyntaxParser) override;

  std::shared_ptr<QueryNode>
  cloneTree()  override;

  std::shared_ptr<std::wstring> getField() override;

  void setField(std::shared_ptr<std::wstring> fieldName) override;

protected:
  std::shared_ptr<TokenizedPhraseQueryNode> shared_from_this()
  {
    return std::static_pointer_cast<TokenizedPhraseQueryNode>(
        QueryNodeImpl::shared_from_this());
  }
}; // end class MultitermQueryNode

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/core/nodes/
