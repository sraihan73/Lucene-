#pragma once
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial::util
{
template <typename T>
class ShapeFieldCacheProvider;
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
namespace org::apache::lucene::spatial::util
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Point;

/**
 * A DoubleValuesSource that returns the spatial distance
 * between an input point and a document's points in
 * {@link ShapeFieldCacheProvider}. The shortest distance is returned if a
 * document has more than one point.
 *
 * @lucene.internal
 */
class ShapeFieldCacheDistanceValueSource : public DoubleValuesSource
{
  GET_CLASS_NAME(ShapeFieldCacheDistanceValueSource)

private:
  const std::shared_ptr<SpatialContext> ctx;
  const std::shared_ptr<Point> from;
  const std::shared_ptr<ShapeFieldCacheProvider<std::shared_ptr<Point>>>
      provider;
  const double multiplier;

public:
  ShapeFieldCacheDistanceValueSource(
      std::shared_ptr<SpatialContext> ctx,
      std::shared_ptr<ShapeFieldCacheProvider<std::shared_ptr<Point>>> provider,
      std::shared_ptr<Point> from, double multiplier);

  virtual std::wstring toString();

  std::shared_ptr<DoubleValues>
  getValues(std::shared_ptr<LeafReaderContext> readerContext,
            std::shared_ptr<DoubleValues> scores)  override;

private:
  class DoubleValuesAnonymousInnerClass : public DoubleValues
  {
    GET_CLASS_NAME(DoubleValuesAnonymousInnerClass)
  private:
    std::shared_ptr<ShapeFieldCacheDistanceValueSource> outerInstance;

    std::shared_ptr<LeafReaderContext> readerContext;

  public:
    DoubleValuesAnonymousInnerClass(
        std::shared_ptr<ShapeFieldCacheDistanceValueSource> outerInstance,
        std::shared_ptr<LeafReaderContext> readerContext);

  private:
    const std::shared_ptr<ShapeFieldCache<std::shared_ptr<Point>>> cache;
    const std::shared_ptr<Point> outerInstance->from;
    const std::shared_ptr<DistanceCalculator> calculator;

    std::deque<std::shared_ptr<Point>> currentVals;

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
  std::shared_ptr<ShapeFieldCacheDistanceValueSource> shared_from_this()
  {
    return std::static_pointer_cast<ShapeFieldCacheDistanceValueSource>(
        org.apache.lucene.search.DoubleValuesSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::util
