#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class DoubleValues;
}

namespace org::apache::lucene::search
{
class DoubleValuesSource;
}
namespace org::apache::lucene::expressions
{
class Bindings;
}
namespace org::apache::lucene::search
{
class SortField;
}
namespace org::apache::lucene::search
{
class Rescorer;
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

using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Rescorer = org::apache::lucene::search::Rescorer;
using SortField = org::apache::lucene::search::SortField;

/**
 * Base class that computes the value of an expression for a document.
 * <p>
 * Example that sorts based on an expression:
 * <pre class="prettyprint">
 *   // compile an expression:
 *   Expression expr = JavascriptCompiler.compile("sqrt(_score) +
 * ln(popularity)");
 *
 *   // SimpleBindings just maps variables to SortField instances
 *   SimpleBindings bindings = new SimpleBindings();
 *   bindings.add(new SortField("_score", SortField.Type.SCORE));
 *   bindings.add(new SortField("popularity", SortField.Type.INT));
 *
 *   // create a sort field and sort by it (reverse order)
 *   Sort sort = new Sort(expr.getSortField(bindings, true));
 *   Query query = new TermQuery(new Term("body", "contents"));
 *   searcher.search(query, 10, sort);
 * </pre>
 * <p>
 * Example that modifies the scores produced by the query:
 * <pre class="prettyprint">
 *   // compile an expression:
 *   Expression expr = JavascriptCompiler.compile("sqrt(_score) +
 * ln(popularity)");
 *
 *   // SimpleBindings just maps variables to SortField instances
 *   SimpleBindings bindings = new SimpleBindings();
 *   bindings.add(new SortField("_score", SortField.Type.SCORE));
 *   bindings.add(new SortField("popularity", SortField.Type.INT));
 *
 *   // create a query that matches based on body:contents but
 *   // scores using expr
 *   Query query = new FunctionScoreQuery(
 *       new TermQuery(new Term("body", "contents")),
 *       expr.getDoubleValuesSource(bindings));
 *   searcher.search(query, 10);
 * </pre>
 * @see JavascriptCompiler#compile
 * @lucene.experimental
 */
class Expression : public std::enable_shared_from_this<Expression>
{
  GET_CLASS_NAME(Expression)

  /** The original source text */
public:
  const std::wstring sourceText;

  /** Named variables referred to by this expression */
  std::deque<std::wstring> const variables;

  /**
   * Creates a new {@code Expression}.
   *
   * @param sourceText Source text for the expression: e.g. {@code
   * ln(popularity)}
   * @param variables Names of external variables referred to by the expression
   */
protected:
  Expression(const std::wstring &sourceText,
             std::deque<std::wstring> &variables);

  /**
   * Evaluates the expression for the current document.
   *
   * @param functionValues {@link DoubleValues} for each element of {@link
   * #variables}.
   * @return The computed value of the expression for the given document.
   */
public:
  virtual double
  evaluate(std::deque<std::shared_ptr<DoubleValues>> &functionValues) = 0;

  /**
   * Get a DoubleValuesSource which can compute the value of this expression in
   * the context of the given bindings.
   * @param bindings Bindings to use for external values in this expression
   * @return A DoubleValuesSource which will evaluate this expression when used
   */
  virtual std::shared_ptr<DoubleValuesSource>
  getDoubleValuesSource(std::shared_ptr<Bindings> bindings);

  /** Get a sort field which can be used to rank documents by this expression.
   */
  virtual std::shared_ptr<SortField>
  getSortField(std::shared_ptr<Bindings> bindings, bool reverse);

  /** Get a {@link Rescorer}, to rescore first-pass hits
   *  using this expression. */
  virtual std::shared_ptr<Rescorer>
  getRescorer(std::shared_ptr<Bindings> bindings);
};

} // namespace org::apache::lucene::expressions
