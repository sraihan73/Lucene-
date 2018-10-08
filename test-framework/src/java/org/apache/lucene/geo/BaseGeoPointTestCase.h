#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/geo/Rectangle.h"

#include  "core/src/java/org/apache/lucene/geo/Polygon.h"
#include  "core/src/java/org/apache/lucene/util/FixedBitSet.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/document/Document.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/codecs/Codec.h"
#include  "core/src/java/org/apache/lucene/codecs/PointsFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/PointsWriter.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/codecs/PointsReader.h"
#include  "core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include  "core/src/java/org/apache/lucene/search/TopDocs.h"

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
namespace org::apache::lucene::geo
{

using Document = org::apache::lucene::document::Document;
using Query = org::apache::lucene::search::Query;
using TopDocs = org::apache::lucene::search::TopDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Abstract class to do basic tests for a geospatial impl (high level
 * fields and queries)
 * NOTE: This test focuses on geospatial (distance queries, polygon
 * queries, etc) indexing and search, not any underlying storage
 * format or encoding: it merely supplies two hooks for the encoding
 * so that tests can be exact. The [stretch] goal is for this test to be
 * so thorough in testing a new geo impl that if this
 * test passes, then all Lucene/Solr tests should also pass.  Ie,
 * if there is some bug in a given geo impl that this
 * test fails to catch then this test needs to be improved! */
class BaseGeoPointTestCase : public LuceneTestCase
{
  GET_CLASS_NAME(BaseGeoPointTestCase)

protected:
  static const std::wstring FIELD_NAME;

  // TODO: remove these hooks once all subclasses can pass with new random!

  virtual double nextLongitude();

  virtual double nextLatitude();

  virtual std::shared_ptr<Rectangle> nextBox();

  virtual std::shared_ptr<Polygon> nextPolygon();

  /** Whether this impl supports polygons. */
  virtual bool supportsPolygons();

  /** Valid values that should not cause exception */
public:
  virtual void testIndexExtremeValues();

  /** Invalid values */
  virtual void testIndexOutOfRangeValues();

  /** NaN: illegal */
  virtual void testIndexNaNValues();

  /** Inf: illegal */
  virtual void testIndexInfValues();

  /** Add a single point and search for it in a box */
  // NOTE: we don't currently supply an exact search, only ranges, because of
  // the lossiness...
  virtual void testBoxBasics() ;

  /** null field name not allowed */
  virtual void testBoxNull();

  // box should not accept invalid lat/lon
  virtual void testBoxInvalidCoordinates() ;

  /** test we can search for a point */
  virtual void testDistanceBasics() ;

  /** null field name not allowed */
  virtual void testDistanceNull();

  /** distance query should not accept invalid lat/lon as origin */
  virtual void testDistanceIllegal() ;

  /** negative distance queries are not allowed */
  virtual void testDistanceNegative();

  /** NaN distance queries are not allowed */
  virtual void testDistanceNaN();

  /** Inf distance queries are not allowed */
  virtual void testDistanceInf();

  /** test we can search for a polygon */
  virtual void testPolygonBasics() ;

  /** test we can search for a polygon with a hole (but still includes the doc)
   */
  virtual void testPolygonHole() ;

  /** test we can search for a polygon with a hole (that excludes the doc) */
  virtual void testPolygonHoleExcludes() ;

  /** test we can search for a multi-polygon */
  virtual void testMultiPolygonBasics() ;

  /** null field name not allowed */
  virtual void testPolygonNullField();

  // A particularly tricky adversary for BKD tree:
  virtual void testSamePointManyTimes() ;

  virtual void testAllLatEqual() ;

  virtual void testAllLonEqual() ;

  virtual void testMultiValued() ;

private:
  class SimpleCollectorAnonymousInnerClass : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<BaseGeoPointTestCase> outerInstance;

    std::shared_ptr<FixedBitSet> hits;

  public:
    SimpleCollectorAnonymousInnerClass(
        std::shared_ptr<BaseGeoPointTestCase> outerInstance,
        std::shared_ptr<FixedBitSet> hits);

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
  virtual void testRandomTiny() ;

  virtual void testRandomMedium() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testRandomBig() throws Exception
  virtual void testRandomBig() ;

private:
  void doTestRandom(int count) ;

  /** Override this to quantize randomly generated lat, so the test won't fail
   * due to quantization errors, which are 1) annoying to debug, and 2) should
   * never affect "real" usage terribly. */
protected:
  virtual double quantizeLat(double lat);

  /** Override this to quantize randomly generated lon, so the test won't fail
   * due to quantization errors, which are 1) annoying to debug, and 2) should
   * never affect "real" usage terribly. */
  virtual double quantizeLon(double lon);

  virtual void addPointToDoc(const std::wstring &field,
                             std::shared_ptr<Document> doc, double lat,
                             double lon) = 0;

  virtual std::shared_ptr<Query> newRectQuery(const std::wstring &field,
                                              double minLat, double maxLat,
                                              double minLon, double maxLon) = 0;

  virtual std::shared_ptr<Query> newDistanceQuery(const std::wstring &field,
                                                  double centerLat,
                                                  double centerLon,
                                                  double radiusMeters) = 0;

  virtual std::shared_ptr<Query>
  newPolygonQuery(const std::wstring &field, std::deque<Polygon> &polygon) = 0;

public:
  static bool rectContainsPoint(std::shared_ptr<Rectangle> rect,
                                double pointLat, double pointLon);

private:
  void verify(std::deque<double> &lats,
              std::deque<double> &lons) ;

protected:
  virtual void
  verifyRandomRectangles(std::deque<double> &lats,
                         std::deque<double> &lons) ;

private:
  class SimpleCollectorAnonymousInnerClass2 : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass2)
  private:
    std::shared_ptr<BaseGeoPointTestCase> outerInstance;

    std::shared_ptr<FixedBitSet> hits;

  public:
    SimpleCollectorAnonymousInnerClass2(
        std::shared_ptr<BaseGeoPointTestCase> outerInstance,
        std::shared_ptr<FixedBitSet> hits);

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
    std::shared_ptr<SimpleCollectorAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass2>(
          org.apache.lucene.search.SimpleCollector::shared_from_this());
    }
  };

protected:
  virtual void
  verifyRandomDistances(std::deque<double> &lats,
                        std::deque<double> &lons) ;

private:
  class SimpleCollectorAnonymousInnerClass3 : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass3)
  private:
    std::shared_ptr<BaseGeoPointTestCase> outerInstance;

    std::shared_ptr<FixedBitSet> hits;

  public:
    SimpleCollectorAnonymousInnerClass3(
        std::shared_ptr<BaseGeoPointTestCase> outerInstance,
        std::shared_ptr<FixedBitSet> hits);

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
    std::shared_ptr<SimpleCollectorAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass3>(
          org.apache.lucene.search.SimpleCollector::shared_from_this());
    }
  };

protected:
  virtual void
  verifyRandomPolygons(std::deque<double> &lats,
                       std::deque<double> &lons) ;

private:
  class SimpleCollectorAnonymousInnerClass4 : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass4)
  private:
    std::shared_ptr<BaseGeoPointTestCase> outerInstance;

    std::shared_ptr<FixedBitSet> hits;

  public:
    SimpleCollectorAnonymousInnerClass4(
        std::shared_ptr<BaseGeoPointTestCase> outerInstance,
        std::shared_ptr<FixedBitSet> hits);

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
    std::shared_ptr<SimpleCollectorAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass4>(
          org.apache.lucene.search.SimpleCollector::shared_from_this());
    }
  };

public:
  virtual void testRectBoundariesAreInclusive() ;

  /** Run a few iterations with just 10 docs, hopefully easy to debug */
  virtual void testRandomDistance() ;

  /** Runs with thousands of docs */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testRandomDistanceHuge() throws
  // Exception
  virtual void testRandomDistanceHuge() ;

private:
  void doRandomDistanceTest(int numDocs, int numQueries) ;

private:
  class FilterCodecAnonymousInnerClass : public FilterCodec
  {
    GET_CLASS_NAME(FilterCodecAnonymousInnerClass)
  private:
    std::shared_ptr<BaseGeoPointTestCase> outerInstance;

    int pointsInLeaf = 0;

  public:
    FilterCodecAnonymousInnerClass(
        std::shared_ptr<BaseGeoPointTestCase> outerInstance,
        std::shared_ptr<org::apache::lucene::codecs::Codec> getDefaultCodec,
        int pointsInLeaf);

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
  virtual void testEquals() ;

  /** return topdocs over a small set of points in field "point" */
private:
  std::shared_ptr<TopDocs> searchSmallSet(std::shared_ptr<Query> query,
                                          int size) ;

public:
  virtual void testSmallSetRect() ;

  virtual void testSmallSetDateline() ;

  virtual void testSmallSetMultiValued() ;

  virtual void testSmallSetWholeMap() ;

  virtual void testSmallSetPoly() ;

  virtual void testSmallSetPolyWholeMap() ;

  virtual void testSmallSetDistance() ;

  virtual void testSmallSetTinyDistance() ;

  /** see https://issues.apache.org/jira/browse/LUCENE-6905 */
  virtual void testSmallSetDistanceNotEmpty() ;

  /**
   * Explicitly large
   */
  virtual void testSmallSetHugeDistance() ;

  virtual void testSmallSetDistanceDateline() ;

protected:
  std::shared_ptr<BaseGeoPointTestCase> shared_from_this()
  {
    return std::static_pointer_cast<BaseGeoPointTestCase>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/geo/
