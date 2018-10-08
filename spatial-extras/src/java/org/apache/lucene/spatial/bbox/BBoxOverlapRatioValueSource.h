#pragma once
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial
{
class ShapeValuesSource;
}

namespace org::apache::lucene::search
{
class Explanation;
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
namespace org::apache::lucene::spatial::bbox
{

using Explanation = org::apache::lucene::search::Explanation;
using ShapeValuesSource = org::apache::lucene::spatial::ShapeValuesSource;
using org::locationtech::spatial4j::shape::Rectangle;

/**
 * The algorithm is implemented as envelope on envelope (rect on rect) overlays
 * rather than complex polygon on complex polygon overlays. <p> Spatial
 * relevance scoring algorithm: <DL> <DT>queryArea</DT> <DD>the area of the
 * input query envelope</DD> <DT>targetArea</DT> <DD>the area of the target
 * envelope (per Lucene document)</DD> <DT>intersectionArea</DT> <DD>the area of
 * the intersection between the query and target envelopes</DD>
 *   <DT>queryTargetProportion</DT> <DD>A 0-1 factor that divides the score
 * proportion between query and target. 0.5 is evenly.</DD>
 *
 *   <DT>queryRatio</DT> <DD>intersectionArea / queryArea; (see note)</DD>
 *   <DT>targetRatio</DT> <DD>intersectionArea / targetArea; (see note)</DD>
 *   <DT>queryFactor</DT> <DD>queryRatio * queryTargetProportion;</DD>
 *   <DT>targetFactor</DT> <DD>targetRatio * (1 - queryTargetProportion);</DD>
 *   <DT>score</DT> <DD>queryFactor + targetFactor;</DD>
 * </DL>
 * Additionally, note that an optional minimum side length {@code minSideLength}
 * may be used whenever an area is calculated (queryArea, targetArea,
 * intersectionArea). This allows for points or horizontal/vertical lines to be
 * used as the query shape and in such case the descending order should have
 * smallest boxes up front. Without this, a point or line query shape typically
 * scores everything with the same value since there is 0 area. <p> Note: The
 * actual computation of queryRatio and targetRatio is more complicated so that
 * it considers points and lines. Lines have the ratio of overlap, and points
 * are either 1.0 or 0.0 depending on whether it intersects or not. <p>
 * Originally based on Geoportal's
 * <a
 * href="http://geoportal.svn.sourceforge.net/svnroot/geoportal/Geoportal/trunk/src/com/esri/gpt/catalog/lucene/SpatialRankingValueSource.java">
 *   SpatialRankingValueSource</a> but modified quite a bit. GeoPortal's
 * algorithm will yield a score of 0 if either a line or point is compared, and
 * it doesn't output a 0-1 normalized score (it multiplies the factors), and it
 * doesn't support minSideLength, and it had dateline bugs.
 *
 * @lucene.experimental
 */
class BBoxOverlapRatioValueSource : public BBoxSimilarityValueSource
{
  GET_CLASS_NAME(BBoxOverlapRatioValueSource)

private:
  const bool isGeo; //-180/+180 degrees  (not part of identity; attached to
                    //parent strategy/field)

  const std::shared_ptr<Rectangle> queryExtent;
  const double queryArea; // not part of identity

  const double minSideLength;

  const double queryTargetProportion;

  // TODO option to compute geodetic area

  /**
   *
   * @param rectValueSource mandatory; source of rectangles
   * @param isGeo True if ctx.isGeo() and thus dateline issues should be
   * attended to
   * @param queryExtent mandatory; the query rectangle
   * @param queryTargetProportion see class javadocs. Between 0 and 1.
   * @param minSideLength see class javadocs. 0.0 will effectively disable.
   */
public:
  BBoxOverlapRatioValueSource(
      std::shared_ptr<ShapeValuesSource> rectValueSource, bool isGeo,
      std::shared_ptr<Rectangle> queryExtent, double queryTargetProportion,
      double minSideLength);

  /** Construct with 75% weighting towards target (roughly GeoPortal's default),
   * geo degrees assumed, no minimum side length. */
  BBoxOverlapRatioValueSource(
      std::shared_ptr<ShapeValuesSource> rectValueSource,
      std::shared_ptr<Rectangle> queryExtent);

  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::wstring similarityDescription() override;

  double score(std::shared_ptr<Rectangle> target,
               std::shared_ptr<AtomicReference<std::shared_ptr<Explanation>>>
                   exp) override;

  /** Calculates the area while applying the minimum side length. */
private:
  double calcArea(double width, double height);

protected:
  std::shared_ptr<BBoxOverlapRatioValueSource> shared_from_this()
  {
    return std::static_pointer_cast<BBoxOverlapRatioValueSource>(
        BBoxSimilarityValueSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::bbox
