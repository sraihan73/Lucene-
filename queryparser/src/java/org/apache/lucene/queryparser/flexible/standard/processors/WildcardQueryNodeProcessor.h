#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::queryparser::flexible::core
{
class QueryNodeException;
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
namespace org::apache::lucene::queryparser::flexible::standard::processors
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;

/**
 * The {@link StandardSyntaxParser} creates {@link PrefixWildcardQueryNode}
 * nodes which have values containing the prefixed wildcard. However, Lucene
 * {@link PrefixQuery} cannot contain the prefixed wildcard. So, this processor
 * basically removed the prefixed wildcard from the
 * {@link PrefixWildcardQueryNode} value.
 *
 * @see PrefixQuery
 * @see PrefixWildcardQueryNode
 */
class WildcardQueryNodeProcessor : public QueryNodeProcessorImpl
{
  GET_CLASS_NAME(WildcardQueryNodeProcessor)

private:
  static const std::shared_ptr<Pattern> WILDCARD_PATTERN;

  // because we call utf8ToString, this will only work with the default
  // TermToBytesRefAttribute
  static std::wstring analyzeWildcard(std::shared_ptr<Analyzer> a,
                                      const std::wstring &field,
                                      const std::wstring &wildcard);

public:
  WildcardQueryNodeProcessor();

protected:
  std::shared_ptr<QueryNode> postProcessNode(
      std::shared_ptr<QueryNode> node)  override;

private:
  bool isWildcard(std::shared_ptr<std::wstring> text);

  bool isPrefixWildcard(std::shared_ptr<std::wstring> text);

protected:
  std::shared_ptr<QueryNode> preProcessNode(
      std::shared_ptr<QueryNode> node)  override;

  std::deque<std::shared_ptr<QueryNode>>
  setChildrenOrder(std::deque<std::shared_ptr<QueryNode>> &children) throw(
      QueryNodeException) override;

protected:
  std::shared_ptr<WildcardQueryNodeProcessor> shared_from_this()
  {
    return std::static_pointer_cast<WildcardQueryNodeProcessor>(
        org.apache.lucene.queryparser.flexible.core.processors
            .QueryNodeProcessorImpl::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::flexible::standard::processors
