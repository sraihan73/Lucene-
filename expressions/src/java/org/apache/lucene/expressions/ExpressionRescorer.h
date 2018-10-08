#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::expressions
{
class Expression;
}

namespace org::apache::lucene::expressions
{
class Bindings;
}
namespace org::apache::lucene::search
{
class DoubleValues;
}
namespace org::apache::lucene::search
{
class Explanation;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
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
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using SortRescorer = org::apache::lucene::search::SortRescorer;

/**
 * A {@link Rescorer} that uses an expression to re-score
 * first pass hits.  Functionally this is the same as {@link
 * SortRescorer} (if you build the {@link Sort} using {@link
 * Expression#getSortField}), except for the explain method
 * which gives more detail by showing the value of each
 * variable.
 *
 * @lucene.experimental
 */

class ExpressionRescorer : public SortRescorer
{
  GET_CLASS_NAME(ExpressionRescorer)

private:
  const std::shared_ptr<Expression> expression;
  const std::shared_ptr<Bindings> bindings;

  /** Uses the provided {@link Expression} to assign second
   *  pass scores. */
public:
  ExpressionRescorer(std::shared_ptr<Expression> expression,
                     std::shared_ptr<Bindings> bindings);

private:
  static std::shared_ptr<DoubleValues> scores(int doc, float score);

private:
  class DoubleValuesAnonymousInnerClass : public DoubleValues
  {
    GET_CLASS_NAME(DoubleValuesAnonymousInnerClass)
  private:
    int doc = 0;
    float score = 0;

  public:
    DoubleValuesAnonymousInnerClass(int doc, float score);

    double doubleValue()  override;

    bool advanceExact(int target)  override;

  protected:
    std::shared_ptr<DoubleValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DoubleValuesAnonymousInnerClass>(
          org.apache.lucene.search.DoubleValues::shared_from_this());
    }
  };

public:
  std::shared_ptr<Explanation>
  explain(std::shared_ptr<IndexSearcher> searcher,
          std::shared_ptr<Explanation> firstPassExplanation,
          int docID)  override;

protected:
  std::shared_ptr<ExpressionRescorer> shared_from_this()
  {
    return std::static_pointer_cast<ExpressionRescorer>(
        org.apache.lucene.search.SortRescorer::shared_from_this());
  }
};

} // namespace org::apache::lucene::expressions
