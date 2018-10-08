#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"

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
namespace org::apache::lucene::index
{

using BKDWriter = org::apache::lucene::util::bkd::BKDWriter;

/**
 * Access to indexed numeric values.
 * <p>
 * Points represent numeric values and are indexed differently than ordinary
text. Instead of an inverted index,
 * points are indexed with datastructures such as <a
href="https://en.wikipedia.org/wiki/K-d_tree">KD-trees</a>.
 * These structures are optimized for operations such as <i>range</i>,
<i>distance</i>, <i>nearest-neighbor</i>,
 * and <i>point-in-polygon</i> queries.
 * <h1>Basic Point Types</h1>
 * <table summary="Basic point types in Java and Lucene">
 *   <tr><th>Java type</th><th>Lucene class</th></tr>
 *   <tr><td>{@code int}</td><td>{@link IntPoint}</td></tr>
GET_CLASS_NAME(</th></tr>)
 *   <tr><td>{@code long}</td><td>{@link LongPoint}</td></tr>
 *   <tr><td>{@code float}</td><td>{@link FloatPoint}</td></tr>
 *   <tr><td>{@code double}</td><td>{@link DoublePoint}</td></tr>
 *   <tr><td>{@code byte[]}</td><td>{@link BinaryPoint}</td></tr>
 *   <tr><td>{@link int64_t}</td><td><a
href="{@docRoot}/../sandbox/org/apache/lucene/document/BigIntegerPoint.html">BigIntegerPoint</a>*</td></tr>
 *   <tr><td>{@link InetAddress}</td><td><a
href="{@docRoot}/../misc/org/apache/lucene/document/InetAddressPoint.html">InetAddressPoint</a>*</td></tr>
 * </table>
 * * in the <i>lucene-sandbox</i> jar<br>
 * <p>
 * Basic Lucene point types behave like their java peers: for example {@link
IntPoint} represents a signed 32-bit
 * {@link Integer}, supporting values ranging from {@link Integer#MIN_VALUE} to
{@link Integer#MAX_VALUE}, ordered
 * consistent with {@link Integer#compareTo(Integer)}. In addition to indexing
support, point classes also contain
 * static methods (such as {@link IntPoint#newRangeQuery(std::wstring, int, int)}) for
creating common queries. For example: GET_CLASS_NAME(es)
 * <pre class="prettyprint">
 *   // add year 1970 to document
 *   document.add(new IntPoint("year", 1970));
 *   // index document
 *   writer.addDocument(document);
 *   ...
 *   // issue range query of 1960-1980
 *   Query query = IntPoint.newRangeQuery("year", 1960, 1980);
 *   TopDocs docs = searcher.search(query, ...);
 * </pre>
 * <h1>Geospatial Point Types</h1>
 * Although basic point types such as {@link DoublePoint} support points in
multi-dimensional space too, Lucene has
 * specialized classes for location data. These classes are optimized for
location data: they are more space-efficient and
 * support special operations such as <i>distance</i> and <i>polygon</i>
queries. There are currently two implementations:
 * <br>
 * <ol>
 *   <li><a
href="{@docRoot}/../sandbox/org/apache/lucene/document/LatLonPoint.html">LatLonPoint</a>
in <i>lucene-sandbox</i>: indexes {@code (latitude,longitude)} as {@code (x,y)}
in two-dimensional space.
 *   <li><a
href="{@docRoot}/../spatial3d/org/apache/lucene/spatial3d/Geo3DPoint.html">Geo3DPoint</a>*
in <i>lucene-spatial3d</i>: indexes {@code (latitude,longitude)} as {@code
(x,y,z)} in three-dimensional space.
 * </ol>
 * * does <b>not</b> support altitude, 3D here means "uses three dimensions
under-the-hood"<br>
 * <h1>Advanced usage</h1>
 * Custom structures can be created on top of single- or multi- dimensional
basic types, on top of
 * {@link BinaryPoint} for more flexibility, or via custom {@link Field}
subclasses.
 *
 *  @lucene.experimental */
class PointValues : public std::enable_shared_from_this<PointValues>
{
  GET_CLASS_NAME(PointValues)

  /** Maximum number of bytes for each dimension */
public:
  static constexpr int MAX_NUM_BYTES = 16;

  /** Maximum number of dimensions */
  static constexpr int MAX_DIMENSIONS = BKDWriter::MAX_DIMS;

  /** Return the cumulated number of points across all leaves of the given
   * {@link IndexReader}. Leaves that do not have points for the given field
   * are ignored.
   *  @see PointValues#size() */
  static int64_t size(std::shared_ptr<IndexReader> reader,
                        const std::wstring &field) ;

  /** Return the cumulated number of docs that have points across all leaves
   * of the given {@link IndexReader}. Leaves that do not have points for the
   * given field are ignored.
   *  @see PointValues#getDocCount() */
  static int getDocCount(std::shared_ptr<IndexReader> reader,
                         const std::wstring &field) ;

  /** Return the minimum packed values across all leaves of the given
   * {@link IndexReader}. Leaves that do not have points for the given field
   * are ignored.
   *  @see PointValues#getMinPackedValue() */
  static std::deque<char>
  getMinPackedValue(std::shared_ptr<IndexReader> reader,
                    const std::wstring &field) ;

  /** Return the maximum packed values across all leaves of the given
   * {@link IndexReader}. Leaves that do not have points for the given field
   * are ignored.
   *  @see PointValues#getMaxPackedValue() */
  static std::deque<char>
  getMaxPackedValue(std::shared_ptr<IndexReader> reader,
                    const std::wstring &field) ;

  /** Default constructor */
protected:
  PointValues();

  /** Used by {@link #intersect} to check how each recursive cell corresponds to
   * the query. */
public:
  enum class Relation {
    GET_CLASS_NAME(Relation)
    /** Return this if the cell is fully contained by the query */
    CELL_INSIDE_QUERY,
    /** Return this if the cell and query do not overlap */
    CELL_OUTSIDE_QUERY,
    /** Return this if the cell partially overlaps the query */
    CELL_CROSSES_QUERY
  };

  /** We recurse the BKD tree, using a provided instance of this to guide the
   * recursion.
   *
   * @lucene.experimental */
public:
  class IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitor)
    /** Called for all documents in a leaf cell that's fully contained by the
     * query.  The consumer should blindly accept the docID. */
  public:
    virtual void visit(int docID) = 0;

    /** Called for all documents in a leaf cell that crosses the query.  The
     * consumer should scrutinize the packedValue to decide whether to accept
     * it.  In the 1D case, values are visited in increasing order, and in the
     * case of ties, in increasing docID order. */
    virtual void visit(int docID, std::deque<char> &packedValue) = 0;

    /** Called for non-leaf cells to test how the cell relates to the query, to
     *  determine how to further recurse down the tree. */
    virtual Relation compare(std::deque<char> &minPackedValue,
                             std::deque<char> &maxPackedValue) = 0;

    /** Notifies the caller that this many documents are about to be visited */
    // C++ TODO: There is no equivalent in C++ to Java default interface
    // methods:
    //        default void grow(int count)
    //    {
    //    };
  };

  /** Finds all documents and points matching the provided visitor.
   *  This method does not enforce live documents, so it's up to the caller
   *  to test whether each document is deleted, if necessary. */
public:
  virtual void intersect(std::shared_ptr<IntersectVisitor> visitor) = 0;

  /** Estimate the number of points that would be visited by {@link #intersect}
   * with the given {@link IntersectVisitor}. This should run many times faster
   * than {@link #intersect(IntersectVisitor)}.
   * @see DocIdSetIterator#cost */
  virtual int64_t
  estimatePointCount(std::shared_ptr<IntersectVisitor> visitor) = 0;

  /** Returns minimum value for each dimension, packed, or null if {@link #size}
   * is <code>0</code> */
  virtual std::deque<char> getMinPackedValue() = 0;

  /** Returns maximum value for each dimension, packed, or null if {@link #size}
   * is <code>0</code> */
  virtual std::deque<char> getMaxPackedValue() = 0;

  /** Returns how many dimensions were indexed */
  virtual int getNumDimensions() = 0;

  /** Returns the number of bytes per dimension */
  virtual int getBytesPerDimension() = 0;

  /** Returns the total number of indexed points across all documents. */
  virtual int64_t size() = 0;

  /** Returns the total number of documents that have indexed at least one
   * point. */
  virtual int getDocCount() = 0;
};

} // #include  "core/src/java/org/apache/lucene/index/
