#pragma once
#include "stringhelper.h"
#include <memory>
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
 * A {@link ModifierQueryNode} indicates the modifier value (+,-,?,NONE) for
 * each term on the query string. For example "+t1 -t2 t3" will have a tree of:
 * <blockquote>
 * &lt;BooleanQueryNode&gt; &lt;ModifierQueryNode modifier="MOD_REQ"&gt;
 * &lt;t1/&gt; &lt;/ModifierQueryNode&gt; &lt;ModifierQueryNode
 * modifier="MOD_NOT"&gt; &lt;t2/&gt; &lt;/ModifierQueryNode&gt; &lt;t3/&gt;
 * &lt;/BooleanQueryNode&gt;
 * </blockquote>
 */
class ModifierQueryNode : public QueryNodeImpl
{
  GET_CLASS_NAME(ModifierQueryNode)

  /**
   * Modifier type: such as required (REQ), prohibited (NOT)
   */
public:
  class Modifier final
  {
    GET_CLASS_NAME(Modifier)
  public:
    static Modifier MOD_NONE;
    static Modifier MOD_NOT;
    static Modifier MOD_REQ;

  private:
    static std::deque<Modifier> valueList;

    class StaticConstructor
    {
      GET_CLASS_NAME(StaticConstructor)
    public:
      StaticConstructor();
    };

    static StaticConstructor staticConstructor;

  public:
    enum class InnerEnum {
      GET_CLASS_NAME(InnerEnum) MOD_NONE,
      MOD_NOT,
      MOD_REQ
    };

    const InnerEnum innerEnumValue;

  private:
    const std::wstring nameValue;
    const int ordinalValue;
    static int nextOrdinal;

    Modifier(const std::wstring &name, InnerEnum innerEnum);

  public:
    std::wstring ToString() override;

    virtual std::wstring toDigitString();

    virtual std::wstring toLargeString();

  public:
    bool operator==(const Modifier &other);

    bool operator!=(const Modifier &other);

    static std::deque<Modifier> values();

    int ordinal();

    static Modifier valueOf(const std::wstring &name);
  };

private:
  Modifier modifier = Modifier::MOD_NONE;

  /**
   * Used to store the modifier value on the original query string
   *
   * @param query
   *          - QueryNode subtree
   * @param mod
   *          - Modifier Value
   */
public:
  ModifierQueryNode(std::shared_ptr<QueryNode> query, Modifier mod);

  virtual std::shared_ptr<QueryNode> getChild();

  virtual Modifier getModifier();

  virtual std::wstring toString();

  std::shared_ptr<std::wstring>
  toQueryString(std::shared_ptr<EscapeQuerySyntax> escapeSyntaxParser) override;

  std::shared_ptr<QueryNode>
  cloneTree()  override;

  virtual void setChild(std::shared_ptr<QueryNode> child);

protected:
  std::shared_ptr<ModifierQueryNode> shared_from_this()
  {
    return std::static_pointer_cast<ModifierQueryNode>(
        QueryNodeImpl::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/core/nodes/
