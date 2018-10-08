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
namespace org::apache::lucene::queryparser::flexible::standard::nodes
{

using FieldValuePairQueryNode = org::apache::lucene::queryparser::flexible::
    core::nodes::FieldValuePairQueryNode;
using QueryNodeImpl =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNodeImpl;
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;

/**
 * This query node represents a field query that holds a point value. It is
 * similar to {@link FieldQueryNode}, however the {@link #getValue()} returns a
 * {@link Number}.
 *
 * @see PointsConfig
 */
class PointQueryNode : public QueryNodeImpl,
                       public FieldValuePairQueryNode<std::shared_ptr<Number>>
{
  GET_CLASS_NAME(PointQueryNode)

private:
  std::shared_ptr<NumberFormat> numberFormat;

  std::shared_ptr<std::wstring> field;

  std::shared_ptr<Number> value;

  /**
   * Creates a {@link PointQueryNode} object using the given field,
   * {@link Number} value and {@link NumberFormat} used to convert the value to
   * {@link std::wstring}.
   *
   * @param field the field associated with this query node
   * @param value the value hold by this node
   * @param numberFormat the {@link NumberFormat} used to convert the value to
   * {@link std::wstring}
   */
public:
  PointQueryNode(std::shared_ptr<std::wstring> field,
                 std::shared_ptr<Number> value,
                 std::shared_ptr<NumberFormat> numberFormat);

  /**
   * Returns the field associated with this node.
   *
   * @return the field associated with this node
   */
  std::shared_ptr<std::wstring> getField() override;

  /**
   * Sets the field associated with this node.
   *
   * @param fieldName the field associated with this node
   */
  void setField(std::shared_ptr<std::wstring> fieldName) override;

  /**
   * This method is used to get the value converted to {@link std::wstring} and
   * escaped using the given {@link EscapeQuerySyntax}.
   *
   * @param escaper the {@link EscapeQuerySyntax} used to escape the value
   * {@link std::wstring}
   *
   * @return the value converte to {@link std::wstring} and escaped
   */
protected:
  virtual std::shared_ptr<std::wstring>
  getTermEscaped(std::shared_ptr<EscapeQuerySyntax> escaper);

public:
  std::shared_ptr<std::wstring>
  toQueryString(std::shared_ptr<EscapeQuerySyntax> escapeSyntaxParser) override;

  /**
   * Sets the {@link NumberFormat} used to convert the value to {@link std::wstring}.
   *
   * @param format the {@link NumberFormat} used to convert the value to {@link
   * std::wstring}
   */
  virtual void setNumberFormat(std::shared_ptr<NumberFormat> format);

  /**
   * Returns the {@link NumberFormat} used to convert the value to {@link
   * std::wstring}.
   *
   * @return the {@link NumberFormat} used to convert the value to {@link
   * std::wstring}
   */
  virtual std::shared_ptr<NumberFormat> getNumberFormat();

  /**
   * Returns the numeric value as {@link Number}.
   *
   * @return the numeric value
   */
  std::shared_ptr<Number> getValue() override;

  /**
   * Sets the numeric value.
   *
   * @param value the numeric value
   */
  void setValue(std::shared_ptr<Number> value) override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<PointQueryNode> shared_from_this()
  {
    return std::static_pointer_cast<PointQueryNode>(
        org.apache.lucene.queryparser.flexible.core.nodes
            .QueryNodeImpl::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/standard/nodes/
