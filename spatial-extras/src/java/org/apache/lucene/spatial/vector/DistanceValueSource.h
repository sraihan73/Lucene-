#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial::deque
{
class PointVectorStrategy;
}

namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class DoubleValues;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
}
namespace org::apache::lucene::search
{
class DoubleValuesSource;
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
namespace org::apache::lucene::spatial::deque
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using org::locationtech::spatial4j::shape::Point;

/**
 * A DoubleValuesSource that returns the distance for a {@link
 * PointVectorStrategy}.
 *
 * @lucene.internal
 */
class DistanceValueSource : public DoubleValuesSource
{
  GET_CLASS_NAME(DistanceValueSource)

private:
  std::shared_ptr<PointVectorStrategy> strategy;
  const std::shared_ptr<Point> from;
  const double multiplier;
  const double nullValue;

  /**
   * Constructor.
   */
public:
  DistanceValueSource(std::shared_ptr<PointVectorStrategy> strategy,
                      std::shared_ptr<Point> from, double multiplier);

  /**
   * Returns the ValueSource description.
   */
  virtual std::wstring toString();

  /**
   * Returns the FunctionValues used by the function query.
   */
  std::shared_ptr<DoubleValues>
  getValues(std::shared_ptr<LeafReaderContext> readerContext,
            std::shared_ptr<DoubleValues> scores)  override;

private:
  class DoubleValuesAnonymousInnerClass : public DoubleValues
  {
    GET_CLASS_NAME(DoubleValuesAnonymousInnerClass)
  private:
    std::shared_ptr<DistanceValueSource> outerInstance;

    std::shared_ptr<NumericDocValues> ptX;
    std::shared_ptr<NumericDocValues> ptY;

  public:
    DoubleValuesAnonymousInnerClass(
        std::shared_ptr<DistanceValueSource> outerInstance,
        std::shared_ptr<NumericDocValues> ptX,
        std::shared_ptr<NumericDocValues> ptY);

  private:
    const std::shared_ptr<Point> outerInstance->from;
    const std::shared_ptr<DistanceCalculator> calculator;

  public:
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
  bool needsScores() override;

  bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  std::shared_ptr<DoubleValuesSource>
  rewrite(std::shared_ptr<IndexSearcher> searcher)  override;

  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<DistanceValueSource> shared_from_this()
  {
    return std::static_pointer_cast<DistanceValueSource>(
        org.apache.lucene.search.DoubleValuesSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::deque
