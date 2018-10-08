#pragma once
#include "stringbuilder.h"
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
 * A {@link ProximityQueryNode} represents a query where the terms should meet
 * specific distance conditions. (a b c) WITHIN [SENTENCE|PARAGRAPH|NUMBER]
 * [INORDER] ("a" "b" "c") WITHIN [SENTENCE|PARAGRAPH|NUMBER] [INORDER]
 *
 * TODO: Add this to the future standard Lucene parser/processor/builder
 */
class ProximityQueryNode : public BooleanQueryNode
{
  GET_CLASS_NAME(ProximityQueryNode)

  /**
   * Distance condition: PARAGRAPH, SENTENCE, or NUMBER
   */
public:
  class Type final
  {
    GET_CLASS_NAME(Type)
    // C++ TODO: Enum value-specific class bodies are not converted by Java to
    // C++ Converter:
    //        PARAGRAPH
    //        {
    //            std::wstring toQueryString()
    //            {
    //                return "WITHIN PARAGRAPH";
    //            }
    //        },
    // C++ TODO: Enum value-specific class bodies are not converted by Java to
    // C++ Converter:
    //        SENTENCE
    //        {
    //            std::wstring toQueryString()
    //            {
    //                return "WITHIN SENTENCE";
    //            }
    //        },
    // C++ TODO: Enum value-specific class bodies are not converted by Java to
    // C++ Converter:
    //        NUMBER
    //        {
    //            std::wstring toQueryString()
    //            {
    //                return "WITHIN";
    //            }
    //        };

  private:
    static std::deque<Type> valueList;

    class StaticConstructor
    {
      GET_CLASS_NAME(StaticConstructor)
    public:
      StaticConstructor();
    };

    static StaticConstructor staticConstructor;

  public:
    enum class InnerEnum { GET_CLASS_NAME(InnerEnum) };

    const InnerEnum innerEnumValue;

  private:
    const std::wstring nameValue;
    const int ordinalValue;
    static int nextOrdinal;

    Type(const std::wstring &name, InnerEnum innerEnum);

  public:
    virtual std::shared_ptr<std::wstring> toQueryString() = 0;

  public:
    bool operator==(const Type &other);

    bool operator!=(const Type &other);

    static std::deque<Type> values();

    int ordinal();

    std::wstring toString();

    static Type valueOf(const std::wstring &name);
  };

  /** utility class containing the distance condition and number */
public:
  class ProximityType : public std::enable_shared_from_this<ProximityType>
  {
    GET_CLASS_NAME(ProximityType)
  public:
    int pDistance = 0;

    Type pType = nullptr;

    ProximityType(Type type);

    ProximityType(Type type, int distance);
  };

private:
  Type proximityType = Type::SENTENCE;
  int distance = -1;
  bool inorder = false;
  std::shared_ptr<std::wstring> field = nullptr;

  /**
   * @param clauses
   *          - QueryNode children
   * @param field
   *          - field name
   * @param type
   *          - type of proximity query
   * @param distance
   *          - positive integer that specifies the distance
   * @param inorder
   *          - true, if the tokens should be matched in the order of the
   *          clauses
   */
public:
  ProximityQueryNode(std::deque<std::shared_ptr<QueryNode>> &clauses,
                     std::shared_ptr<std::wstring> field, Type type,
                     int distance, bool inorder);

  /**
   * @param clauses
   *          - QueryNode children
   * @param field
   *          - field name
   * @param type
   *          - type of proximity query
   * @param inorder
   *          - true, if the tokens should be matched in the order of the
   *          clauses
   */
  ProximityQueryNode(std::deque<std::shared_ptr<QueryNode>> &clauses,
                     std::shared_ptr<std::wstring> field, Type type,
                     bool inorder);

private:
  static void clearFields(std::deque<std::shared_ptr<QueryNode>> &nodes,
                          std::shared_ptr<std::wstring> field);

public:
  virtual Type getProximityType();

  virtual std::wstring toString();

  std::shared_ptr<std::wstring>
  toQueryString(std::shared_ptr<EscapeQuerySyntax> escapeSyntaxParser) override;

  std::shared_ptr<QueryNode>
  cloneTree()  override;

  /**
   * @return the distance
   */
  virtual int getDistance();

  /**
   * returns null if the field was not specified in the query string
   *
   * @return the field
   */
  virtual std::shared_ptr<std::wstring> getField();

  /**
   * returns null if the field was not specified in the query string
   *
   * @return the field
   */
  virtual std::wstring getFieldAsString();

  /**
   * @param field
   *          the field to set
   */
  virtual void setField(std::shared_ptr<std::wstring> field);

  /**
   * @return terms must be matched in the specified order
   */
  virtual bool isInOrder();

protected:
  std::shared_ptr<ProximityQueryNode> shared_from_this()
  {
    return std::static_pointer_cast<ProximityQueryNode>(
        BooleanQueryNode::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/core/nodes/
