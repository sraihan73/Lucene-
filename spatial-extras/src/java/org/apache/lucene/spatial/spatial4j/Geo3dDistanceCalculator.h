#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/PlanetModel.h"

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

namespace org::apache::lucene::spatial::spatial4j
{

using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::distance::DistanceCalculator;
using org::locationtech::spatial4j::shape::Circle;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;

/**
 * Geo3d implementation of {@link DistanceCalculator}
 *
 * @lucene.experimental
 */
class Geo3dDistanceCalculator
    : public std::enable_shared_from_this<Geo3dDistanceCalculator>,
      public DistanceCalculator
{
  GET_CLASS_NAME(Geo3dDistanceCalculator)

protected:
  const std::shared_ptr<PlanetModel> planetModel;

public:
  Geo3dDistanceCalculator(std::shared_ptr<PlanetModel> planetModel);

  double distance(std::shared_ptr<Point> from,
                  std::shared_ptr<Point> to) override;

  double distance(std::shared_ptr<Point> from, double toX, double toY) override;

  bool within(std::shared_ptr<Point> from, double toX, double toY,
              double distance) override;

  std::shared_ptr<Point> pointOnBearing(std::shared_ptr<Point> from,
                                        double distDEG, double bearingDEG,
                                        std::shared_ptr<SpatialContext> ctx,
                                        std::shared_ptr<Point> reuse) override;

  std::shared_ptr<Rectangle>
  calcBoxByDistFromPt(std::shared_ptr<Point> from, double distDEG,
                      std::shared_ptr<SpatialContext> ctx,
                      std::shared_ptr<Rectangle> reuse) override;

  double calcBoxByDistFromPt_yHorizAxisDEG(
      std::shared_ptr<Point> from, double distDEG,
      std::shared_ptr<SpatialContext> ctx) override;

  double area(std::shared_ptr<Rectangle> rect) override;

  double area(std::shared_ptr<Circle> circle) override;
};

} // #include  "core/src/java/org/apache/lucene/spatial/spatial4j/
