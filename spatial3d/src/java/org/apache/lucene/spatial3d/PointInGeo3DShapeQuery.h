#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial3d::geom
{
class GeoShape;
}

namespace org::apache::lucene::spatial3d::geom
{
class XYZBounds;
}
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
namespace org::apache::lucene::spatial3d
{

using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Weight = org::apache::lucene::search::Weight;
using GeoShape = org::apache::lucene::spatial3d::geom::GeoShape;
using XYZBounds = org::apache::lucene::spatial3d::geom::XYZBounds;

/** Finds all previously indexed points that fall within the specified polygon.
 *
 * <p>The field must be indexed using {@link Geo3DPoint}.
 *
 * @lucene.experimental */

class PointInGeo3DShapeQuery final : public Query
{
  GET_CLASS_NAME(PointInGeo3DShapeQuery)
public:
  const std::wstring field;
  const std::shared_ptr<GeoShape> shape;
  const std::shared_ptr<XYZBounds> shapeBounds;

  /** The lats/lons must be clockwise or counter-clockwise. */
  PointInGeo3DShapeQuery(const std::wstring &field,
                         std::shared_ptr<GeoShape> shape);

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
  {
    GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
  private:
    std::shared_ptr<PointInGeo3DShapeQuery> outerInstance;

  public:
    ConstantScoreWeightAnonymousInnerClass(
        std::shared_ptr<PointInGeo3DShapeQuery> outerInstance, float boost);

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ConstantScoreWeightAnonymousInnerClass>(
          org.apache.lucene.search.ConstantScoreWeight::shared_from_this());
    }
  };

public:
  std::wstring getField();

  std::shared_ptr<GeoShape> getShape();

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<PointInGeo3DShapeQuery> other);

public:
  virtual int hashCode();

  std::wstring toString(const std::wstring &field) override;

protected:
  std::shared_ptr<PointInGeo3DShapeQuery> shared_from_this()
  {
    return std::static_pointer_cast<PointInGeo3DShapeQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial3d
