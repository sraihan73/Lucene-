#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class Codec;
}

namespace org::apache::lucene::codecs
{
class PointsFormat;
}
namespace org::apache::lucene::codecs
{
class PointsWriter;
}
namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::codecs
{
class PointsReader;
}
namespace org::apache::lucene::index
{
class SegmentReadState;
}
namespace org::apache::lucene::spatial3d::geom
{
class GeoPoint;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::spatial3d::geom
{
class GeoShape;
}
namespace org::apache::lucene::util
{
class FixedBitSet;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::geo
{
class Polygon;
}
namespace org::apache::lucene::spatial3d::geom
{
class PlanetModel;
}
namespace org::apache::lucene::spatial3d::geom
{
class GeoPolygon;
}
namespace org::apache::lucene::index
{
class PointValues;
}
namespace org::apache::lucene::index
{
class IntersectVisitor;
}
namespace org::apache::lucene::spatial3d
{
class Cell;
}
namespace org::apache::lucene::index
{
class IndexReader;
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

using Codec = org::apache::lucene::codecs::Codec;
using Polygon = org::apache::lucene::geo::Polygon;
using IndexReader = org::apache::lucene::index::IndexReader;
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using Query = org::apache::lucene::search::Query;
using GeoPoint = org::apache::lucene::spatial3d::geom::GeoPoint;
using GeoPolygon = org::apache::lucene::spatial3d::geom::GeoPolygon;
using GeoShape = org::apache::lucene::spatial3d::geom::GeoShape;
using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestGeo3DPoint : public LuceneTestCase
{
  GET_CLASS_NAME(TestGeo3DPoint)

private:
  static std::shared_ptr<Codec> getCodec();

private:
  class FilterCodecAnonymousInnerClass : public FilterCodec
  {
    GET_CLASS_NAME(FilterCodecAnonymousInnerClass)
  private:
    int maxPointsInLeafNode = 0;
    double maxMBSortInHeap = 0;

  public:
    FilterCodecAnonymousInnerClass(std::shared_ptr<Codec> getDefault,
                                   int maxPointsInLeafNode,
                                   double maxMBSortInHeap);

    std::shared_ptr<PointsFormat> pointsFormat() override;

  private:
    class PointsFormatAnonymousInnerClass : public PointsFormat
    {
      GET_CLASS_NAME(PointsFormatAnonymousInnerClass)
    private:
      std::shared_ptr<FilterCodecAnonymousInnerClass> outerInstance;

    public:
      PointsFormatAnonymousInnerClass(
          std::shared_ptr<FilterCodecAnonymousInnerClass> outerInstance);

      std::shared_ptr<PointsWriter>
      fieldsWriter(std::shared_ptr<SegmentWriteState> writeState) throw(
          IOException) override;

      std::shared_ptr<PointsReader>
      fieldsReader(std::shared_ptr<SegmentReadState> readState) throw(
          IOException) override;

    protected:
      std::shared_ptr<PointsFormatAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<PointsFormatAnonymousInnerClass>(
            org.apache.lucene.codecs.PointsFormat::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<FilterCodecAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterCodecAnonymousInnerClass>(
          org.apache.lucene.codecs.FilterCodec::shared_from_this());
    }
  };

public:
  virtual void testBasic() ;

private:
  static double toRadians(double degrees);

private:
  class Cell : public std::enable_shared_from_this<Cell>
  {
    GET_CLASS_NAME(Cell)
  public:
    static int nextCellID;

    const std::shared_ptr<Cell> parent;
    const int cellID;
    const int xMinEnc, xMaxEnc;
    const int yMinEnc, yMaxEnc;
    const int zMinEnc, zMaxEnc;
    const int splitCount;

    Cell(std::shared_ptr<Cell> parent, int xMinEnc, int xMaxEnc, int yMinEnc,
         int yMaxEnc, int zMinEnc, int zMaxEnc, int splitCount);

    /** Returns true if the quantized point lies within this cell, inclusive on
     * all bounds. */
    virtual bool contains(std::shared_ptr<GeoPoint> point);

    virtual std::wstring toString();
  };

private:
  static double quantize(double xyzValue);

  static std::shared_ptr<GeoPoint> quantize(std::shared_ptr<GeoPoint> point);

  /** Tests consistency of GeoArea.getRelationship vs GeoShape.isWithin */
public:
  virtual void testGeo3DRelations() ;

  virtual void testRandomTiny() ;

  virtual void testRandomMedium() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testRandomBig() throws Exception
  virtual void testRandomBig() ;

private:
  void doTestRandom(int count) ;

public:
  virtual void testPolygonOrdering();

private:
  static constexpr double MEAN_EARTH_RADIUS_METERS = PlanetModel::WGS84_MEAN;

  static std::shared_ptr<Query> random3DQuery(const std::wstring &field);

  // Poached from Geo3dRptTest.randomShape:
  static std::shared_ptr<GeoShape> randomShape();

  static void verify(std::deque<double> &lats,
                     std::deque<double> &lons) ;

private:
  class SimpleCollectorAnonymousInnerClass : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<FixedBitSet> hits;

  public:
    SimpleCollectorAnonymousInnerClass(std::shared_ptr<FixedBitSet> hits);

  private:
    int docBase = 0;

  public:
    bool needsScores() override;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    void collect(int doc) override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass>(
          org.apache.lucene.search.SimpleCollector::shared_from_this());
    }
  };

public:
  virtual void testToString();

  virtual void testShapeQueryToString();

private:
  static std::shared_ptr<Directory> getDirectory();

public:
  virtual void testEquals();

  virtual void testComplexPolygons();

protected:
  static double MINIMUM_EDGE_ANGLE;
  static double MINIMUM_ARC_ANGLE;

  /** Cook up a random Polygon that makes sense, with possible nested polygon
   * within. This is part of testing more complex polygons with nested holes.
   * Picking random points doesn't do it because it's almost impossible to come
   * up with nested ones of the proper clockwise/counterclockwise rotation that
   * way.
   */
  static std::shared_ptr<Polygon> makePoly(std::shared_ptr<PlanetModel> pm,
                                           std::shared_ptr<GeoPoint> pole,
                                           bool const clockwiseDesired,
                                           bool const createHoles);

  static std::deque<std::shared_ptr<GeoPoint>>
  convertToPoints(std::shared_ptr<PlanetModel> pm,
                  std::shared_ptr<GeoPoint> pole, std::deque<double> &angles,
                  std::deque<double> &arcDistances);

  static std::shared_ptr<GeoPoint>
  createPoint(std::shared_ptr<PlanetModel> pm, double const angle,
              double const arcDistance, double const sinLatitude,
              double const cosLatitude, double const sinLongitude,
              double const cosLongitude);

  static bool verifyPolygon(std::shared_ptr<PlanetModel> pm,
                            std::shared_ptr<Polygon> polygon,
                            std::shared_ptr<GeoPolygon> outsidePolygon);

  static int legalIndex(int index, int size);

public:
  virtual void testEncodeDecodeCeil() ;

  /** make sure values always go down: this is important for edge case
   * consistency */
  virtual void testEncodeDecodeRoundsDown() ;

  // poached from TestGeoEncodingUtils.testLatitudeQuantization:

  /**
   * step through some integers, ensuring they decode to their expected double
   * values. double values start at -planetMax and increase by Geo3DUtil.DECODE
   * for each integer. check edge cases within the double range and random
   * doubles within the range too.
   */
  virtual void testQuantization() ;

  virtual void testEncodeDecodeIsStable() ;

  /** Clips the incoming value to the allowed min/max range before encoding,
   * instead of throwing an exception. */
private:
  static int encodeValueLenient(double x);

private:
  class ExplainingVisitor
      : public std::enable_shared_from_this<ExplainingVisitor>,
        public IntersectVisitor
  {
    GET_CLASS_NAME(ExplainingVisitor)

  public:
    const std::shared_ptr<GeoShape> shape;
    const std::shared_ptr<GeoPoint> targetDocPoint;
    const std::shared_ptr<GeoPoint> scaledDocPoint;
    const std::shared_ptr<IntersectVisitor> in_;
    const std::deque<std::shared_ptr<Cell>> stack =
        std::deque<std::shared_ptr<Cell>>();

  private:
    std::deque<std::shared_ptr<Cell>> stackToTargetDoc;

  public:
    const int targetDocID;
    const int numDims;
    const int bytesPerDim;

  private:
    int targetStackUpto = 0;

  public:
    const std::shared_ptr<StringBuilder> b;

    // In the first phase, we always return CROSSES to do a full scan of the BKD
    // tree to see which leaf block the document lives in
    bool firstPhase = true;

    ExplainingVisitor(std::shared_ptr<GeoShape> shape,
                      std::shared_ptr<GeoPoint> targetDocPoint,
                      std::shared_ptr<GeoPoint> scaledDocPoint,
                      std::shared_ptr<IntersectVisitor> in_, int targetDocID,
                      int numDims, int bytesPerDim,
                      std::shared_ptr<StringBuilder> b);

    virtual void startSecondPhase();

    void visit(int docID)  override;

    void visit(int docID,
               std::deque<char> &packedValue)  override;

    Relation compare(std::deque<char> &minPackedValue,
                     std::deque<char> &maxPackedValue) override;

  private:
    class Cell : public std::enable_shared_from_this<Cell>
    {
      GET_CLASS_NAME(Cell)
    private:
      std::shared_ptr<TestGeo3DPoint::ExplainingVisitor> outerInstance;

      std::deque<char> const minPackedValue;
      std::deque<char> const maxPackedValue;

    public:
      Cell(std::shared_ptr<TestGeo3DPoint::ExplainingVisitor> outerInstance,
           std::deque<char> &minPackedValue,
           std::deque<char> &maxPackedValue);

      /** Returns true if this cell fully contains the other one */
      virtual bool contains(std::shared_ptr<Cell> other);

      virtual std::wstring toString();

      bool equals(std::any other) override;

      virtual int hashCode();
    };
  };

public:
  static std::wstring explain(const std::wstring &fieldName,
                              std::shared_ptr<GeoShape> shape,
                              std::shared_ptr<GeoPoint> targetDocPoint,
                              std::shared_ptr<GeoPoint> scaledDocPoint,
                              std::shared_ptr<IndexReader> reader,
                              int docID) ;

protected:
  std::shared_ptr<TestGeo3DPoint> shared_from_this()
  {
    return std::static_pointer_cast<TestGeo3DPoint>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial3d
