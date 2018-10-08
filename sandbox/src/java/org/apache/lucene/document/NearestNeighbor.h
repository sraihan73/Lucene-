#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <string>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/bkd/BKDReader.h"

#include  "core/src/java/org/apache/lucene/util/bkd/IndexTree.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/document/NearestHit.h"
#include  "core/src/java/org/apache/lucene/index/PointValues.h"

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

using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using Bits = org::apache::lucene::util::Bits;
using IndexTree = org::apache::lucene::util::bkd::BKDReader::IndexTree;
using BKDReader = org::apache::lucene::util::bkd::BKDReader;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLatitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLongitude;

/**
 * KNN search on top of 2D lat/lon indexed points.
 *
 * @lucene.experimental
 */
class NearestNeighbor : public std::enable_shared_from_this<NearestNeighbor>
{
  GET_CLASS_NAME(NearestNeighbor)

public:
  class Cell : public std::enable_shared_from_this<Cell>,
               public Comparable<std::shared_ptr<Cell>>
  {
    GET_CLASS_NAME(Cell)
  public:
    const int readerIndex;
    std::deque<char> const minPacked;
    std::deque<char> const maxPacked;
    const std::shared_ptr<IndexTree> index;

    /** The closest possible distance of all points in this cell */
    const double distanceMeters;

    Cell(std::shared_ptr<IndexTree> index, int readerIndex,
         std::deque<char> &minPacked, std::deque<char> &maxPacked,
         double distanceMeters);

    virtual int compareTo(std::shared_ptr<Cell> other);

    virtual std::wstring toString();
  };

private:
  class NearestVisitor : public std::enable_shared_from_this<NearestVisitor>,
                         public IntersectVisitor
  {
    GET_CLASS_NAME(NearestVisitor)

  public:
    int curDocBase = 0;
    std::shared_ptr<Bits> curLiveDocs;
    const int topN;
    const std::shared_ptr<PriorityQueue<std::shared_ptr<NearestHit>>> hitQueue;
    const double pointLat;
    const double pointLon;

  private:
    int setBottomCounter = 0;

    double minLon = -std::numeric_limits<double>::infinity();
    double maxLon = std::numeric_limits<double>::infinity();
    double minLat = -std::numeric_limits<double>::infinity();
    double maxLat = std::numeric_limits<double>::infinity();

    // second set of longitude ranges to check (for cross-dateline case)
    double minLon2 = std::numeric_limits<double>::infinity();

  public:
    NearestVisitor(
        std::shared_ptr<PriorityQueue<std::shared_ptr<NearestHit>>> hitQueue,
        int topN, double pointLat, double pointLon);

    void visit(int docID) override;

  private:
    void maybeUpdateBBox();

  public:
    void visit(int docID, std::deque<char> &packedValue) override;

    Relation compare(std::deque<char> &minPackedValue,
                     std::deque<char> &maxPackedValue) override;
  };

  /** Holds one hit from {@link LatLonPoint#nearest} */
public:
  class NearestHit : public std::enable_shared_from_this<NearestHit>
  {
    GET_CLASS_NAME(NearestHit)
  public:
    int docID = 0;
    double distanceMeters = 0;

    virtual std::wstring toString();
  };

  // TODO: can we somehow share more with, or simply directly use, the
  // LatLonPointDistanceComparator?  It's really doing the same thing as our
  // hitQueue...

public:
  static std::deque<std::shared_ptr<NearestHit>>
  nearest(double pointLat, double pointLon,
          std::deque<std::shared_ptr<BKDReader>> &readers,
          std::deque<std::shared_ptr<Bits>> &liveDocs,
          std::deque<int> &docBases, int const n) ;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<NearestHit>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  public:
    ComparatorAnonymousInnerClass();

    int compare(std::shared_ptr<NearestHit> a, std::shared_ptr<NearestHit> b);
  };

  // NOTE: incoming args never cross the dateline, since they are a BKD cell
private:
  static double approxBestDistance(std::deque<char> &minPackedValue,
                                   std::deque<char> &maxPackedValue,
                                   double pointLat, double pointLon);

  // NOTE: incoming args never cross the dateline, since they are a BKD cell
  static double approxBestDistance(double minLat, double maxLat, double minLon,
                                   double maxLon, double pointLat,
                                   double pointLon);
};

} // #include  "core/src/java/org/apache/lucene/document/
