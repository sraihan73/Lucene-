#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

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
 * A {@link NoTokenFoundQueryNode} is used if a term is convert into no tokens
 * by the tokenizer/lemmatizer/analyzer (null).
 */
class NoTokenFoundQueryNode : public DeletedQueryNode
{
  GET_CLASS_NAME(NoTokenFoundQueryNode)

public:
  NoTokenFoundQueryNode();

  std::shared_ptr<std::wstring>
  toQueryString(std::shared_ptr<EscapeQuerySyntax> escaper) override;

  virtual std::wstring toString();

  std::shared_ptr<QueryNode>
  cloneTree()  override;

protected:
  std::shared_ptr<NoTokenFoundQueryNode> shared_from_this()
  {
    return std::static_pointer_cast<NoTokenFoundQueryNode>(
        DeletedQueryNode::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::flexible::core::nodes
