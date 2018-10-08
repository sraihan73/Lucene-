#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/config/QueryConfigHandler.h"

#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/nodes/QueryNode.h"
#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/QueryNodeException.h"
#include  "core/src/java/org/apache/lucene/queryparser/flexible/core/nodes/ModifierQueryNode.h"

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
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;
using ModifierQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::ModifierQueryNode;
using Modifier = org::apache::lucene::queryparser::flexible::core::nodes::
    ModifierQueryNode::Modifier;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QueryNodeProcessor = org::apache::lucene::queryparser::flexible::core::
    processors::QueryNodeProcessor;

/**
 * <p>
 * This processor is used to apply the correct {@link ModifierQueryNode} to
 * {@link BooleanQueryNode}s children. This is a variant of
 * {@link BooleanModifiersQueryNodeProcessor} which ignores precedence.
 * </p>
 * <p>
 * The {@link StandardSyntaxParser} knows the rules of precedence, but lucene
 * does not. e.g. <code>(A AND B OR C AND D)</code> ist treated like
 * <code>(+A +B +C +D)</code>.
 * </p>
 * <p>
 * This processor walks through the query node tree looking for
 * {@link BooleanQueryNode}s. If an {@link AndQueryNode} is found, every child,
 * which is not a {@link ModifierQueryNode} or the {@link ModifierQueryNode} is
 * {@link Modifier#MOD_NONE}, becomes a {@link Modifier#MOD_REQ}. For default
 * {@link BooleanQueryNode}, it checks the default operator is
 * {@link Operator#AND}, if it is, the same operation when an
 * {@link AndQueryNode} is found is applied to it. Each {@link BooleanQueryNode}
 * which direct parent is also a {@link BooleanQueryNode} is removed (to ignore
 * the rules of precedence).
 * </p>
 *
 * @see ConfigurationKeys#DEFAULT_OPERATOR
 * @see BooleanModifiersQueryNodeProcessor
 */
class BooleanQuery2ModifierNodeProcessor
    : public std::enable_shared_from_this<BooleanQuery2ModifierNodeProcessor>,
      public QueryNodeProcessor
{
  GET_CLASS_NAME(BooleanQuery2ModifierNodeProcessor)
public:
  static const std::wstring TAG_REMOVE;
  static const std::wstring TAG_MODIFIER;
  static const std::wstring TAG_BOOLEAN_ROOT;

  std::shared_ptr<QueryConfigHandler> queryConfigHandler;

private:
  const std::deque<std::shared_ptr<QueryNode>> childrenBuffer =
      std::deque<std::shared_ptr<QueryNode>>();

  std::optional<bool> usingAnd = false;

public:
  BooleanQuery2ModifierNodeProcessor();

  std::shared_ptr<QueryNode> process(
      std::shared_ptr<QueryNode> queryTree)  override;

protected:
  virtual void processChildren(std::shared_ptr<QueryNode> queryTree) throw(
      QueryNodeException);

private:
  std::shared_ptr<QueryNode> processIteration(
      std::shared_ptr<QueryNode> queryTree) ;

protected:
  virtual void
  fillChildrenBufferAndApplyModifiery(std::shared_ptr<QueryNode> parent);

  virtual std::shared_ptr<QueryNode>
  postProcessNode(std::shared_ptr<QueryNode> node) ;

  virtual std::shared_ptr<QueryNode>
  preProcessNode(std::shared_ptr<QueryNode> node) ;

  virtual bool isDefaultBooleanQueryNode(std::shared_ptr<QueryNode> toTest);

private:
  std::shared_ptr<QueryNode> applyModifier(std::shared_ptr<QueryNode> node,
                                           ModifierQueryNode::Modifier mod);

protected:
  virtual void tagModifierButDoNotOverride(std::shared_ptr<QueryNode> node,
                                           ModifierQueryNode::Modifier mod);

public:
  void setQueryConfigHandler(
      std::shared_ptr<QueryConfigHandler> queryConfigHandler) override;

  std::shared_ptr<QueryConfigHandler> getQueryConfigHandler() override;
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/standard/processors/
