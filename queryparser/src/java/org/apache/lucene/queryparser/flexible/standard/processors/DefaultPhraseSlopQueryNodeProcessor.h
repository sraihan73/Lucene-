#pragma once
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <deque>

// C++ NOTE: Forward class declarations:
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

using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;

/**
 * This processor verifies if {@link ConfigurationKeys#PHRASE_SLOP}
 * is defined in the {@link QueryConfigHandler}. If it is, it looks for every
 * {@link TokenizedPhraseQueryNode} and {@link MultiPhraseQueryNode} that does
 * not have any {@link SlopQueryNode} applied to it and creates an
 * {@link SlopQueryNode} and apply to it. The new {@link SlopQueryNode} has the
 * same slop value defined in the configuration.
 *
 * @see SlopQueryNode
 * @see ConfigurationKeys#PHRASE_SLOP
 */
class DefaultPhraseSlopQueryNodeProcessor : public QueryNodeProcessorImpl
{
  GET_CLASS_NAME(DefaultPhraseSlopQueryNodeProcessor)

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  bool processChildren_ = true;

  int defaultPhraseSlop = 0;

public:
  DefaultPhraseSlopQueryNodeProcessor();

  std::shared_ptr<QueryNode> process(
      std::shared_ptr<QueryNode> queryTree)  override;

protected:
  std::shared_ptr<QueryNode> postProcessNode(
      std::shared_ptr<QueryNode> node)  override;

  std::shared_ptr<QueryNode> preProcessNode(
      std::shared_ptr<QueryNode> node)  override;

  void processChildren(std::shared_ptr<QueryNode> queryTree) throw(
      QueryNodeException) override;

  std::deque<std::shared_ptr<QueryNode>>
  setChildrenOrder(std::deque<std::shared_ptr<QueryNode>> &children) throw(
      QueryNodeException) override;

protected:
  std::shared_ptr<DefaultPhraseSlopQueryNodeProcessor> shared_from_this()
  {
    return std::static_pointer_cast<DefaultPhraseSlopQueryNodeProcessor>(
        org.apache.lucene.queryparser.flexible.core.processors
            .QueryNodeProcessorImpl::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/standard/processors/
