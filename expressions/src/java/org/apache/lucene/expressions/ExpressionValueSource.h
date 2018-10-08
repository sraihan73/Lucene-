#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class DoubleValuesSource;
}

namespace org::apache::lucene::expressions
{
class Expression;
}
namespace org::apache::lucene::expressions
{
class Bindings;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
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

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

/**
 * A {@link DoubleValuesSource} which evaluates a {@link Expression} given the
 * context of an {@link Bindings}.
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings({"rawtypes", "unchecked"}) final class
// ExpressionValueSource extends org.apache.lucene.search.DoubleValuesSource
class ExpressionValueSource final : public DoubleValuesSource
{
public:
  std::deque<std::shared_ptr<DoubleValuesSource>> const variables;
  const std::shared_ptr<Expression> expression;
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool needsScores_;

  ExpressionValueSource(std::shared_ptr<Bindings> bindings,
                        std::shared_ptr<Expression> expression);

  ExpressionValueSource(
      std::deque<std::shared_ptr<DoubleValuesSource>> &variables,
      std::shared_ptr<Expression> expression, bool needsScores);

  std::shared_ptr<DoubleValues>
  getValues(std::shared_ptr<LeafReaderContext> readerContext,
            std::shared_ptr<DoubleValues> scores)  override;

private:
  static std::shared_ptr<DoubleValues>
  zeroWhenUnpositioned(std::shared_ptr<DoubleValues> in_);

private:
  class DoubleValuesAnonymousInnerClass : public DoubleValues
  {
    GET_CLASS_NAME(DoubleValuesAnonymousInnerClass)
  private:
    std::shared_ptr<DoubleValues> in_;

  public:
    DoubleValuesAnonymousInnerClass(std::shared_ptr<DoubleValues> in_);

    bool positioned = false;

    double doubleValue()  override;

    bool advanceExact(int doc)  override;

  protected:
    std::shared_ptr<DoubleValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DoubleValuesAnonymousInnerClass>(
          org.apache.lucene.search.DoubleValues::shared_from_this());
    }
  };

public:
  virtual std::wstring toString();

  virtual int hashCode();

  bool equals(std::any obj) override;

  bool needsScores() override;

  bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  std::shared_ptr<Explanation>
  explain(std::shared_ptr<LeafReaderContext> ctx, int docId,
          std::shared_ptr<Explanation> scoreExplanation) 
      override;

  std::shared_ptr<DoubleValuesSource>
  rewrite(std::shared_ptr<IndexSearcher> searcher)  override;

protected:
  std::shared_ptr<ExpressionValueSource> shared_from_this()
  {
    return std::static_pointer_cast<ExpressionValueSource>(
        org.apache.lucene.search.DoubleValuesSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::expressions
