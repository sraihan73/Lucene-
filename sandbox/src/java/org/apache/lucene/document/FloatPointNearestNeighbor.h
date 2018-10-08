#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::bkd
{
class BKDReader;
}

namespace org::apache::lucene::util::bkd
{
class IndexTree;
}
namespace org::apache::lucene::util
{
class Bits;
}
namespace org::apache::lucene::document
{
class NearestHit;
}
namespace org::apache::lucene::index
{
class PointValues;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class TopFieldDocs;
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

using PointValues = org::apache::lucene::index::PointValues;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TopFieldDocs = org::apache::lucene::search::TopFieldDocs;
using Bits = org::apache::lucene::util::Bits;
using BKDReader = org::apache::lucene::util::bkd::BKDReader;

/**
 * KNN search on top of N dimensional indexed float points.
 *
 * @lucene.experimental
 */
class FloatPointNearestNeighbor
    : public std::enable_shared_from_this<FloatPointNearestNeighbor>
{
  GET_CLASS_NAME(FloatPointNearestNeighbor)

public:
  class Cell : public std::enable_shared_from_this<Cell>,
               public Comparable<std::shared_ptr<Cell>>
  {
    GET_CLASS_NAME(Cell)
  public:
    const int readerIndex;
    std::deque<char> const minPacked;
    std::deque<char> const maxPacked;
    const std::shared_ptr<BKDReader::IndexTree> index;

    /** The closest possible distance^2 of all points in this cell */
    const double distanceSquared;

    Cell(std::shared_ptr<BKDReader::IndexTree> index, int readerIndex,
         std::deque<char> &minPacked, std::deque<char> &maxPacked,
         double distanceSquared);

    virtual int compareTo(std::shared_ptr<Cell> other);

    virtual std::wstring toString();
  };

private:
  class NearestVisitor : public std::enable_shared_from_this<NearestVisitor>,
                         public PointValues::IntersectVisitor
  {
    GET_CLASS_NAME(NearestVisitor)
  public:
    int curDocBase = 0;
    std::shared_ptr<Bits> curLiveDocs;
    const int topN;
    const std::shared_ptr<PriorityQueue<std::shared_ptr<NearestHit>>> hitQueue;
    std::deque<float> const origin;

  private:
    int dims = 0;
    int updateMinMaxCounter = 0;
    std::deque<float> min;
    std::deque<float> max;

  public:
    NearestVisitor(
        std::shared_ptr<PriorityQueue<std::shared_ptr<NearestHit>>> hitQueue,
        int topN, std::deque<float> &origin);

    void visit(int docID) override;

  private:
    static constexpr int MANTISSA_BITS = 23;

    /**
     * Returns the minimum value that will change the given distance when added
     * to it.
     *
     * This value is calculated from the distance exponent reduced by (at most)
     * 23, the number of bits in a float mantissa. This is necessary when the
     * result of subtracting/adding the distance in a single dimension has an
     * exponent that differs significantly from that of the distance value.
     * Without this fudge factor (i.e. only subtracting/adding the distance),
     * cells and values can be inappropriately judged as outside the search
     * radius.
     */
    float getMinDelta(float distance);

    void maybeUpdateMinMax();

  public:
    void visit(int docID, std::deque<char> &packedValue) override;

    PointValues::Relation compare(std::deque<char> &minPackedValue,
                                  std::deque<char> &maxPackedValue) override;
  };

  /** Holds one hit from {@link FloatPointNearestNeighbor#nearest} */
public:
  class NearestHit : public std::enable_shared_from_this<NearestHit>
  {
    GET_CLASS_NAME(NearestHit)
  public:
    int docID = 0;
    double distanceSquared = 0;

    virtual std::wstring toString();
  };

private:
  static std::deque<std::shared_ptr<NearestHit>>
  nearest(std::deque<std::shared_ptr<BKDReader>> &readers,
          std::deque<std::shared_ptr<Bits>> &liveDocs,
          std::deque<int> &docBases, int const topN,
          std::deque<float> &origin) ;

  static double approxBestDistanceSquared(std::deque<char> &minPackedValue,
                                          std::deque<char> &maxPackedValue,
                                          std::deque<float> &value);

public:
  static double euclideanDistanceSquared(std::deque<float> &a,
                                         std::deque<float> &b);

  static std::shared_ptr<TopFieldDocs>
  nearest(std::shared_ptr<IndexSearcher> searcher, const std::wstring &field,
          int topN, std::deque<float> &origin) ;
};

} // namespace org::apache::lucene::document
