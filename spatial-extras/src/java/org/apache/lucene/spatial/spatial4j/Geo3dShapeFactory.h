#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/PlanetModel.h"

#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPoint.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPolygonFactory.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/PolygonDescription.h"

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

using com::google::common::geometry::S2CellId;
using com::google::common::geometry::S2Point;
using S2ShapeFactory =
    org::apache::lucene::spatial::prefix::tree::S2ShapeFactory;
using GeoCompositeAreaShape =
    org::apache::lucene::spatial3d::geom::GeoCompositeAreaShape;
using GeoPoint = org::apache::lucene::spatial3d::geom::GeoPoint;
using GeoPolygonFactory =
    org::apache::lucene::spatial3d::geom::GeoPolygonFactory;
using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::context::SpatialContextFactory;
using org::locationtech::spatial4j::shape::Circle;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::ShapeCollection;

/**
 * Geo3d implementation of {@link S2ShapeFactory}
 *
 * @lucene.experimental
 */
class Geo3dShapeFactory
    : public std::enable_shared_from_this<Geo3dShapeFactory>,
      public S2ShapeFactory
{
  GET_CLASS_NAME(Geo3dShapeFactory)

private:
  const bool normWrapLongitude;
  std::shared_ptr<SpatialContext> context;
  std::shared_ptr<PlanetModel> planetModel;

  /**
   * Default accuracy for circles when not using the unit sphere.
   * It is equivalent to ~10m on the surface of the earth.
   */
  static constexpr double DEFAULT_CIRCLE_ACCURACY = 1e-4;
  double circleAccuracy = DEFAULT_CIRCLE_ACCURACY;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked") public
  // Geo3dShapeFactory(org.locationtech.spatial4j.context.SpatialContext context,
  // org.locationtech.spatial4j.context.SpatialContextFactory factory)
  Geo3dShapeFactory(std::shared_ptr<SpatialContext> context,
                    std::shared_ptr<SpatialContextFactory> factory);

  std::shared_ptr<SpatialContext> getSpatialContext() override;

  /**
   * Set the accuracy for circles in decimal degrees. Note that accuracy has no
   * effect when the planet model is a sphere. In that case, circles are always
   * fully precise.
   *
   * @param circleAccuracy the provided accuracy in decimal degrees.
   */
  virtual void setCircleAccuracy(double circleAccuracy);

  bool isNormWrapLongitude() override;

  double normX(double x) override;

  double normY(double y) override;

  double normZ(double z) override;

  double normDist(double distance) override;

  void verifyX(double x) override;

  void verifyY(double y) override;

  void verifyZ(double v) override;

  std::shared_ptr<Point> pointXY(double x, double y) override;

  std::shared_ptr<Point> pointXYZ(double x, double y, double z) override;

  std::shared_ptr<Rectangle> rect(std::shared_ptr<Point> point,
                                  std::shared_ptr<Point> point1) override;

  std::shared_ptr<Rectangle> rect(double minX, double maxX, double minY,
                                  double maxY) override;

  std::shared_ptr<Circle> circle(double x, double y, double distance) override;

  std::shared_ptr<Circle> circle(std::shared_ptr<Point> point,
                                 double distance) override;

  std::shared_ptr<Shape> lineString(std::deque<std::shared_ptr<Point>> &deque,
                                    double distance) override;

  template <typename S>
  std::shared_ptr<ShapeCollection<S>> multiShape(std::deque<S> &deque);

  std::shared_ptr<LineStringBuilder> lineString() override;

  std::shared_ptr<PolygonBuilder> polygon() override;

  template <typename T>
  std::shared_ptr<MultiShapeBuilder<T>> multiShape(std::type_info<T> &aClass);

  std::shared_ptr<MultiPointBuilder> multiPoint() override;

  std::shared_ptr<MultiLineStringBuilder> multiLineString() override;

  std::shared_ptr<MultiPolygonBuilder> multiPolygon() override;

  std::shared_ptr<Shape>
  getS2CellShape(std::shared_ptr<S2CellId> cellId) override;

private:
  std::shared_ptr<GeoPoint> getGeoPoint(std::shared_ptr<S2Point> point);

  /**
   * Geo3d implementation of {@link
   * org.locationtech.spatial4j.shape.ShapeFactory.PointsBuilder} interface to
   * generate {@link GeoPoint}.
   *
   * @param <T> is normally this object
   */
private:
  template <typename T>
  class Geo3dPointBuilder
      : public std::enable_shared_from_this<Geo3dPointBuilder>,
        public PointsBuilder<T>
  {
    GET_CLASS_NAME(Geo3dPointBuilder)
  private:
    std::shared_ptr<Geo3dShapeFactory> outerInstance;

  public:
    Geo3dPointBuilder(std::shared_ptr<Geo3dShapeFactory> outerInstance)
        : outerInstance(outerInstance)
    {
    }

    std::deque<std::shared_ptr<GeoPoint>> points;

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings("unchecked") @Override public T
    // pointXY(double x, double y)
    T pointXY(double x, double y) override
    {
      std::shared_ptr<GeoPoint> point = std::make_shared<GeoPoint>(
          outerInstance->planetModel, y * DistanceUtils::DEGREES_TO_RADIANS,
          x * DistanceUtils::DEGREES_TO_RADIANS);
      points.push_back(point);
      return static_cast<T>(shared_from_this());
    }

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings("unchecked") @Override public T
    // pointXYZ(double x, double y, double z)
    T pointXYZ(double x, double y, double z) override
    {
      std::shared_ptr<GeoPoint> point = std::make_shared<GeoPoint>(x, y, z);
      if (!std::find(points.begin(), points.end(), point) != points.end()) {
        points.push_back(point);
      }
      return static_cast<T>(shared_from_this());
    }
  };

  /**
   * Geo3d implementation of {@link
   * org.locationtech.spatial4j.shape.ShapeFactory.LineStringBuilder} to
   * generate line strings.
   */
private:
  class Geo3dLineStringBuilder
      : public Geo3dPointBuilder<std::shared_ptr<LineStringBuilder>>,
        public LineStringBuilder
  {
    GET_CLASS_NAME(Geo3dLineStringBuilder)
  private:
    std::shared_ptr<Geo3dShapeFactory> outerInstance;

  public:
    Geo3dLineStringBuilder(std::shared_ptr<Geo3dShapeFactory> outerInstance);

    double distance = 0;

    std::shared_ptr<LineStringBuilder> buffer(double distance) override;

    std::shared_ptr<Shape> build() override;

  protected:
    std::shared_ptr<Geo3dLineStringBuilder> shared_from_this()
    {
      return std::static_pointer_cast<Geo3dLineStringBuilder>(
          Geo3dPointBuilder<LineStringBuilder>::shared_from_this());
    }
  };

  /**
   * Geo3d implementation of {@link
   * org.locationtech.spatial4j.shape.ShapeFactory.PolygonBuilder} to generate
   * polygons.
   */
private:
  class Geo3dPolygonBuilder
      : public Geo3dPointBuilder<std::shared_ptr<PolygonBuilder>>,
        public PolygonBuilder
  {
    GET_CLASS_NAME(Geo3dPolygonBuilder)
  private:
    std::shared_ptr<Geo3dShapeFactory> outerInstance;

  public:
    Geo3dPolygonBuilder(std::shared_ptr<Geo3dShapeFactory> outerInstance);

    std::deque<std::shared_ptr<GeoPolygonFactory::PolygonDescription>>
        polyHoles = std::deque<
            std::shared_ptr<GeoPolygonFactory::PolygonDescription>>();

    std::shared_ptr<HoleBuilder> hole() override;

  public:
    class Geo3dHoleBuilder : public Geo3dPointBuilder<
                                 std::shared_ptr<PolygonBuilder::HoleBuilder>>,
                             public PolygonBuilder::HoleBuilder
    {
      GET_CLASS_NAME(Geo3dHoleBuilder)
    private:
      std::shared_ptr<Geo3dShapeFactory::Geo3dPolygonBuilder> outerInstance;

    public:
      Geo3dHoleBuilder(std::shared_ptr<Geo3dShapeFactory::Geo3dPolygonBuilder>
                           outerInstance);

      std::shared_ptr<PolygonBuilder> endHole() override;

    protected:
      std::shared_ptr<Geo3dHoleBuilder> shared_from_this()
      {
        return std::static_pointer_cast<Geo3dHoleBuilder>(
            Geo3dPointBuilder<PolygonBuilder.HoleBuilder>::shared_from_this());
      }
    };

  public:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings("unchecked") @Override public
    // org.locationtech.spatial4j.shape.Shape build()
    std::shared_ptr<Shape> build() override;

    std::shared_ptr<Shape> buildOrRect() override;

  protected:
    std::shared_ptr<Geo3dPolygonBuilder> shared_from_this()
    {
      return std::static_pointer_cast<Geo3dPolygonBuilder>(
          Geo3dPointBuilder<PolygonBuilder>::shared_from_this());
    }
  };

private:
  class Geo3dMultiPointBuilder
      : public Geo3dPointBuilder<std::shared_ptr<MultiPointBuilder>>,
        public MultiPointBuilder
  {
    GET_CLASS_NAME(Geo3dMultiPointBuilder)
  private:
    std::shared_ptr<Geo3dShapeFactory> outerInstance;

  public:
    Geo3dMultiPointBuilder(std::shared_ptr<Geo3dShapeFactory> outerInstance);

    std::shared_ptr<Shape> build() override;

  protected:
    std::shared_ptr<Geo3dMultiPointBuilder> shared_from_this()
    {
      return std::static_pointer_cast<Geo3dMultiPointBuilder>(
          Geo3dPointBuilder<MultiPointBuilder>::shared_from_this());
    }
  };

  /**
   * Geo3d implementation of {@link
   * org.locationtech.spatial4j.shape.ShapeFactory.MultiLineStringBuilder} to
   * generate multi-lines
   */
private:
  class Geo3dMultiLineBuilder
      : public std::enable_shared_from_this<Geo3dMultiLineBuilder>,
        public MultiLineStringBuilder
  {
    GET_CLASS_NAME(Geo3dMultiLineBuilder)
  private:
    std::shared_ptr<Geo3dShapeFactory> outerInstance;

  public:
    Geo3dMultiLineBuilder(std::shared_ptr<Geo3dShapeFactory> outerInstance);

    std::deque<std::shared_ptr<LineStringBuilder>> builders =
        std::deque<std::shared_ptr<LineStringBuilder>>();

    std::shared_ptr<LineStringBuilder> lineString() override;

    std::shared_ptr<MultiLineStringBuilder>
    add(std::shared_ptr<LineStringBuilder> lineStringBuilder) override;

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings("unchecked") @Override public
    // org.locationtech.spatial4j.shape.Shape build()
    std::shared_ptr<Shape> build() override;
  };

  /**
   * Geo3d implementation of {@link
   * org.locationtech.spatial4j.shape.ShapeFactory.MultiPolygonBuilder} to
   * generate multi-polygons. We have chosen to use a composite shape but it
   * might be possible to use GeoComplexPolygon.
   */
private:
  class Geo3dMultiPolygonBuilder
      : public std::enable_shared_from_this<Geo3dMultiPolygonBuilder>,
        public MultiPolygonBuilder
  {
    GET_CLASS_NAME(Geo3dMultiPolygonBuilder)
  private:
    std::shared_ptr<Geo3dShapeFactory> outerInstance;

  public:
    Geo3dMultiPolygonBuilder(std::shared_ptr<Geo3dShapeFactory> outerInstance);

    std::deque<std::shared_ptr<PolygonBuilder>> builders =
        std::deque<std::shared_ptr<PolygonBuilder>>();

    std::shared_ptr<PolygonBuilder> polygon() override;

    std::shared_ptr<MultiPolygonBuilder>
    add(std::shared_ptr<PolygonBuilder> polygonBuilder) override;

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings("unchecked") @Override public
    // org.locationtech.spatial4j.shape.Shape build()
    std::shared_ptr<Shape> build() override;
  };

  /**
   * Geo3d implementation of {@link
   * org.locationtech.spatial4j.shape.ShapeFactory.MultiShapeBuilder} to
   * generate geometry collections.
   *
   * @param <T> is the type of shapes.
   */
private:
  template <typename T>
  class Geo3dMultiShapeBuilder
      : public std::enable_shared_from_this<Geo3dMultiShapeBuilder>,
        public MultiShapeBuilder<T>
  {
    GET_CLASS_NAME(Geo3dMultiShapeBuilder)
  private:
    std::shared_ptr<Geo3dShapeFactory> outerInstance;

  public:
    Geo3dMultiShapeBuilder(std::shared_ptr<Geo3dShapeFactory> outerInstance)
        : outerInstance(outerInstance)
    {
    }

    static_assert(
        std::is_base_of<org.locationtech.spatial4j.shape.Shape, T>::value,
        L"T must inherit from org.locationtech.spatial4j.shape.Shape");

    std::shared_ptr<GeoCompositeAreaShape> composite =
        std::make_shared<GeoCompositeAreaShape>(outerInstance->planetModel);

    std::shared_ptr<MultiShapeBuilder<T>> add(T shape) override
    {
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: Geo3dShape<?> areaShape = (Geo3dShape<?>) shape;
      std::shared_ptr < Geo3dShape <
          ? >> areaShape = std::static_pointer_cast < Geo3dShape < ? >> (shape);
      composite->addShape(areaShape->shape);
      return shared_from_this();
    }

    std::shared_ptr<Shape> build() override
    {
      return std::make_shared<Geo3dShape<>>(composite, outerInstance->context);
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/spatial/spatial4j/
