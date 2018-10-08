#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <deque>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/QueryNodeException.h"
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
namespace org::apache::lucene::queryparser::flexible::standard::processors
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;
using Operator = org::apache::lucene::queryparser::flexible::standard::config::
    StandardQueryConfigHandler::Operator;

/**
 * This processor verifies if {@link ConfigurationKeys#ANALYZER}
 * is defined in the {@link QueryConfigHandler}. If it is and the analyzer is
 * not <code>null</code>, it looks for every {@link FieldQueryNode} that is not
 * {@link WildcardQueryNode}, {@link FuzzyQueryNode} or
 * {@link RangeQueryNode} contained in the query node tree, then it applies
 * the analyzer to that {@link FieldQueryNode} object. <br>
 * <br>
 * If the analyzer return only one term, the returned term is set to the
 * {@link FieldQueryNode} and it's returned. <br>
 * <br>
 * If the analyzer return more than one term, a {@link TokenizedPhraseQueryNode}
 * or {@link MultiPhraseQueryNode} is created, whether there is one or more
 * terms at the same position, and it's returned. <br>
 * <br>
 * If no term is returned by the analyzer a {@link NoTokenFoundQueryNode} object
 * is returned.
 *
 * @see ConfigurationKeys#ANALYZER
 * @see Analyzer
 * @see TokenStream
 */
class AnalyzerQueryNodeProcessor : public QueryNodeProcessorImpl
{
  GET_CLASS_NAME(AnalyzerQueryNodeProcessor)

private:
  std::shared_ptr<Analyzer> analyzer;

  bool positionIncrementsEnabled = false;

  Operator defaultOperator = static_cast<Operator>(0);

public:
  AnalyzerQueryNodeProcessor();

  std::shared_ptr<QueryNode> process(
      std::shared_ptr<QueryNode> queryTree)  override;

protected:
  std::shared_ptr<QueryNode> postProcessNode(
      std::shared_ptr<QueryNode> node)  override;

  std::shared_ptr<QueryNode> preProcessNode(
      std::shared_ptr<QueryNode> node)  override;

  std::deque<std::shared_ptr<QueryNode>>
  setChildrenOrder(std::deque<std::shared_ptr<QueryNode>> &children) throw(
      QueryNodeException) override;

protected:
  std::shared_ptr<AnalyzerQueryNodeProcessor> shared_from_this()
  {
    return std::static_pointer_cast<AnalyzerQueryNodeProcessor>(
        org.apache.lucene.queryparser.flexible.core.processors
            .QueryNodeProcessorImpl::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/standard/processors/
