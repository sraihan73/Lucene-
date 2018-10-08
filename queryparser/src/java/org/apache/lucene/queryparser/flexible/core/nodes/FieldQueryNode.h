#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
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
 * A {@link FieldQueryNode} represents a element that contains field/text tuple
 */
class FieldQueryNode
    : public QueryNodeImpl,
      public FieldValuePairQueryNode<std::shared_ptr<std::wstring>>,
      public TextableQueryNode
{
  GET_CLASS_NAME(FieldQueryNode)

  /**
   * The term's field
   */
protected:
  std::shared_ptr<std::wstring> field;

  /**
   * The term's text.
   */
  std::shared_ptr<std::wstring> text;

  /**
   * The term's begin position.
   */
  int begin = 0;

  /**
   * The term's end position.
   */
  int end = 0;

  /**
   * The term's position increment.
   */
  int positionIncrement = 0;

  /**
   * @param field
   *          - field name
   * @param text
   *          - value
   * @param begin
   *          - position in the query string
   * @param end
   *          - position in the query string
   */
public:
  FieldQueryNode(std::shared_ptr<std::wstring> field,
                 std::shared_ptr<std::wstring> text, int begin, int end);

protected:
  virtual std::shared_ptr<std::wstring>
  getTermEscaped(std::shared_ptr<EscapeQuerySyntax> escaper);

  virtual std::shared_ptr<std::wstring>
  getTermEscapeQuoted(std::shared_ptr<EscapeQuerySyntax> escaper);

public:
  std::shared_ptr<std::wstring>
  toQueryString(std::shared_ptr<EscapeQuerySyntax> escaper) override;

  virtual std::wstring toString();

  /**
   * @return the term
   */
  virtual std::wstring getTextAsString();

  /**
   * returns null if the field was not specified in the query string
   *
   * @return the field
   */
  virtual std::wstring getFieldAsString();

  virtual int getBegin();

  virtual void setBegin(int begin);

  virtual int getEnd();

  virtual void setEnd(int end);

  std::shared_ptr<std::wstring> getField() override;

  void setField(std::shared_ptr<std::wstring> field) override;

  virtual int getPositionIncrement();

  virtual void setPositionIncrement(int pi);

  /**
   * Returns the term.
   *
   * @return The "original" form of the term.
   */
  std::shared_ptr<std::wstring> getText() override;

  /**
   * @param text
   *          the text to set
   */
  void setText(std::shared_ptr<std::wstring> text) override;

  std::shared_ptr<FieldQueryNode>
  cloneTree()  override;

  std::shared_ptr<std::wstring> getValue() override;

  void setValue(std::shared_ptr<std::wstring> value) override;

protected:
  std::shared_ptr<FieldQueryNode> shared_from_this()
  {
    return std::static_pointer_cast<FieldQueryNode>(
        QueryNodeImpl::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/core/nodes/
