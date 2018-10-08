#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class DoubleValuesSource;
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

namespace org::apache::lucene::spatial::util
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

/**
 * Transforms a DoubleValuesSource using the formula v = k / (v + k)
 */
class ReciprocalDoubleValuesSource : public DoubleValuesSource
{
  GET_CLASS_NAME(ReciprocalDoubleValuesSource)

private:
  const double distToEdge;
  const std::shared_ptr<DoubleValuesSource> input;

  /**
   * Creates a ReciprocalDoubleValuesSource
   * @param distToEdge  the value k in v = k / (v + k)
   * @param input       the input DoubleValuesSource to transform
   */
public:
  ReciprocalDoubleValuesSource(double distToEdge,
                               std::shared_ptr<DoubleValuesSource> input);

  std::shared_ptr<DoubleValues>
  getValues(std::shared_ptr<LeafReaderContext> ctx,
            std::shared_ptr<DoubleValues> scores)  override;

private:
  class DoubleValuesAnonymousInnerClass : public DoubleValues
  {
    GET_CLASS_NAME(DoubleValuesAnonymousInnerClass)
  private:
    std::shared_ptr<ReciprocalDoubleValuesSource> outerInstance;

    std::shared_ptr<DoubleValues> in_;

  public:
    DoubleValuesAnonymousInnerClass(
        std::shared_ptr<ReciprocalDoubleValuesSource> outerInstance,
        std::shared_ptr<DoubleValues> in_);

    double doubleValue()  override;

    bool advanceExact(int doc)  override;

  protected:
    std::shared_ptr<DoubleValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DoubleValuesAnonymousInnerClass>(
          org.apache.lucene.search.DoubleValues::shared_from_this());
    }
  };

private:
  double recip(double in_);

public:
  bool needsScores() override;

  bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  std::shared_ptr<Explanation>
  explain(std::shared_ptr<LeafReaderContext> ctx, int docId,
          std::shared_ptr<Explanation> scoreExplanation) 
      override;

  std::shared_ptr<DoubleValuesSource>
  rewrite(std::shared_ptr<IndexSearcher> searcher)  override;

  virtual bool equals(std::any o);

  virtual int hashCode();

  virtual std::wstring toString();

protected:
  std::shared_ptr<ReciprocalDoubleValuesSource> shared_from_this()
  {
    return std::static_pointer_cast<ReciprocalDoubleValuesSource>(
        org.apache.lucene.search.DoubleValuesSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::util
