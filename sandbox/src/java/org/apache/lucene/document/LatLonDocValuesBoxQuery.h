#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class IndexSearcher;
}

namespace org::apache::lucene::search
{
class Weight;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::index
{
class SortedNumericDocValues;
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
namespace org::apache::lucene::document
{

using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Weight = org::apache::lucene::search::Weight;

/** Distance query for {@link LatLonDocValuesField}. */
class LatLonDocValuesBoxQuery final : public Query
{
  GET_CLASS_NAME(LatLonDocValuesBoxQuery)

private:
  const std::wstring field;
  const int minLatitude, maxLatitude, minLongitude, maxLongitude;
  const bool crossesDateline;

public:
  LatLonDocValuesBoxQuery(const std::wstring &field, double minLatitude,
                          double maxLatitude, double minLongitude,
                          double maxLongitude);

  std::wstring toString(const std::wstring &field) override;

  bool equals(std::any obj) override;

  virtual int hashCode();

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
  {
    GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
  private:
    std::shared_ptr<LatLonDocValuesBoxQuery> outerInstance;

    float boost = 0;

  public:
    ConstantScoreWeightAnonymousInnerClass(
        std::shared_ptr<LatLonDocValuesBoxQuery> outerInstance, float boost);

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class TwoPhaseIteratorAnonymousInnerClass : public TwoPhaseIterator
    {
      GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass)
    private:
      std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance;

      std::shared_ptr<SortedNumericDocValues> values;

    public:
      TwoPhaseIteratorAnonymousInnerClass(
          std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
          std::shared_ptr<SortedNumericDocValues> values);

      bool matches()  override;

      float matchCost() override;

    protected:
      std::shared_ptr<TwoPhaseIteratorAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<TwoPhaseIteratorAnonymousInnerClass>(
            org.apache.lucene.search.TwoPhaseIterator::shared_from_this());
      }
    };

  public:
    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ConstantScoreWeightAnonymousInnerClass>(
          org.apache.lucene.search.ConstantScoreWeight::shared_from_this());
    }
  };

protected:
  std::shared_ptr<LatLonDocValuesBoxQuery> shared_from_this()
  {
    return std::static_pointer_cast<LatLonDocValuesBoxQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::document
