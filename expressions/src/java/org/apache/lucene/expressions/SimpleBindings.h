#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class SortField;
}

namespace org::apache::lucene::search
{
class DoubleValuesSource;
}
namespace org::apache::lucene::expressions
{
class Expression;
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
namespace org::apache::lucene::expressions
{

using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using SortField = org::apache::lucene::search::SortField;

/**
 * Simple class that binds expression variable names to {@link SortField}s
 * or other {@link Expression}s.
GET_CLASS_NAME(that)
 * <p>
 * Example usage:
 * <pre class="prettyprint">
 *   SimpleBindings bindings = new SimpleBindings();
 *   // document's text relevance score
 *   bindings.add(new SortField("_score", SortField.Type.SCORE));
 *   // integer NumericDocValues field
 *   bindings.add(new SortField("popularity", SortField.Type.INT));
 *   // another expression
 *   bindings.add("recency", myRecencyExpression);
 *
 *   // create a sort field in reverse order
 *   Sort sort = new Sort(expr.getSortField(bindings, true));
 * </pre>
 *
 * @lucene.experimental
 */
class SimpleBindings final : public Bindings
{
  GET_CLASS_NAME(SimpleBindings)
public:
  const std::unordered_map<std::wstring, std::any> map_obj =
      std::unordered_map<std::wstring, std::any>();

  /** Creates a new empty Bindings */
  SimpleBindings();

  /**
   * Adds a SortField to the bindings.
   * <p>
   * This can be used to reference a DocValuesField, a field from
   * FieldCache, the document's score, etc.
   */
  void add(std::shared_ptr<SortField> sortField);

  /**
   * Bind a {@link DoubleValuesSource} directly to the given name.
   */
  void add(const std::wstring &name,
           std::shared_ptr<DoubleValuesSource> source);

  /**
   * Adds an Expression to the bindings.
   * <p>
   * This can be used to reference expressions from other expressions.
   */
  void add(const std::wstring &name, std::shared_ptr<Expression> expression);

  std::shared_ptr<DoubleValuesSource>
  getDoubleValuesSource(const std::wstring &name) override;

  /**
   * Traverses the graph of bindings, checking there are no cycles or missing
   * references
   * @throws IllegalArgumentException if the bindings is inconsistent
   */
  void validate();

protected:
  std::shared_ptr<SimpleBindings> shared_from_this()
  {
    return std::static_pointer_cast<SimpleBindings>(
        Bindings::shared_from_this());
  }
};

} // namespace org::apache::lucene::expressions
