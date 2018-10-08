#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::geo
{
class Edge;
}

namespace org::apache::lucene::geo
{
class Polygon;
}
namespace org::apache::lucene::index
{
class PointValues;
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
namespace org::apache::lucene::geo
{

using Polygon = org::apache::lucene::geo::Polygon;
using Relation = org::apache::lucene::index::PointValues::Relation;

/**
 * 2D polygon implementation represented as a balanced interval tree of edges.
 * <p>
 * Construction takes {@code O(n log n)} time for sorting and tree construction.
 * {@link #contains contains()} and {@link #relate relate()} are {@code O(n)},
 * but for most practical polygons are much faster than brute force. <p> Loosely
 * based on the algorithm described in <a
 * href="http://www-ma2.upc.es/geoc/Schirra-pointPolygon.pdf">
 * http://www-ma2.upc.es/geoc/Schirra-pointPolygon.pdf</a>.
 * @lucene.internal
 */
// Both Polygon.contains() and Polygon.crossesSlowly() loop all edges, and first
// check that the edge is within a range. we just organize the edges to do the
// same computations on the same subset of edges more efficiently.
class Polygon2D final : public std::enable_shared_from_this<Polygon2D>
{
  GET_CLASS_NAME(Polygon2D)
  /** minimum latitude of this polygon's bounding box area */
public:
  const double minLat;
  /** maximum latitude of this polygon's bounding box area */
  const double maxLat;
  /** minimum longitude of this polygon's bounding box area */
  const double minLon;
  /** maximum longitude of this polygon's bounding box area */
  const double maxLon;

  // each component/hole is a node in an augmented 2d kd-tree: we alternate
  // splitting between latitude/longitude, and pull up max values for both
  // dimensions to each parent node (regardless of split).

  /** maximum latitude of this component or any of its children */
private:
  double maxY = 0;
  /** maximum longitude of this component or any of its children */
  double maxX = 0;
  /** which dimension was this node split on */
  // TODO: its implicit based on level, but bool keeps code simple
  bool splitX = false;

  // child components, or null
  std::shared_ptr<Polygon2D> left;
  std::shared_ptr<Polygon2D> right;

  /** tree of holes, or null */
  const std::shared_ptr<Polygon2D> holes;

  /** root node of edge tree */
  const std::shared_ptr<Edge> tree;

  Polygon2D(std::shared_ptr<Polygon> polygon, std::shared_ptr<Polygon2D> holes);

  /**
   * Returns true if the point is contained within this polygon.
   * <p>
   * See <a
   * href="https://www.ecse.rpi.edu/~wrf/Research/Short_Notes/pnpoly.html">
   * https://www.ecse.rpi.edu/~wrf/Research/Short_Notes/pnpoly.html</a> for more
   * information.
   */
public:
  bool contains(double latitude, double longitude);

  /** Returns true if the point is contained within this polygon component. */
private:
  bool componentContains(double latitude, double longitude);

  /** Returns relation to the provided rectangle */
public:
  Relation relate(double minLat, double maxLat, double minLon, double maxLon);

  /** Returns relation to the provided rectangle for this component */
private:
  Relation componentRelate(double minLat, double maxLat, double minLon,
                           double maxLon);

  // returns 0, 4, or something in between
  int numberOfCorners(double minLat, double maxLat, double minLon,
                      double maxLon);

  /** Creates tree from sorted components (with range low and high inclusive) */
  static std::shared_ptr<Polygon2D>
  createTree(std::deque<std::shared_ptr<Polygon2D>> &components, int low,
             int high, bool splitX);

  /** Builds a Polygon2D from multipolygon */
public:
  static std::shared_ptr<Polygon2D> create(std::deque<Polygon> &polygons);

  /**
   * Internal tree node: represents polygon edge from lat1,lon1 to lat2,lon2.
   * The sort value is {@code low}, which is the minimum latitude of the edge.
   * {@code max} stores the maximum latitude of this edge or any children.
   */
public:
  class Edge final : public std::enable_shared_from_this<Edge>
  {
    GET_CLASS_NAME(Edge)
    // lat-lon pair (in original order) of the two vertices
  public:
    const double lat1, lat2;
    const double lon1, lon2;
    /** min of this edge */
    const double low;
    /** max latitude of this edge or any children */
    double max = 0;

    /** left child edge, or null */
    std::shared_ptr<Edge> left;
    /** right child edge, or null */
    std::shared_ptr<Edge> right;

    Edge(double lat1, double lon1, double lat2, double lon2, double low,
         double max);

    /**
     * Returns true if the point crosses this edge subtree an odd number of
     * times <p> See <a
     * href="https://www.ecse.rpi.edu/~wrf/Research/Short_Notes/pnpoly.html">
     * https://www.ecse.rpi.edu/~wrf/Research/Short_Notes/pnpoly.html</a> for
     * more information.
     */
    // ported to java from
    // https://www.ecse.rpi.edu/~wrf/Research/Short_Notes/pnpoly.html original
    // code under the BSD license
    // (https://www.ecse.rpi.edu/~wrf/Research/Short_Notes/pnpoly.html#License%20to%20Use)
    //
    // Copyright (c) 1970-2003, Wm. Randolph Franklin
    //
    // Permission is hereby granted, free of charge, to any person obtaining a
    // copy of this software and associated documentation files (the
    // "Software"), to deal in the Software without restriction, including
    // without limitation the rights to use, copy, modify, merge, publish,
    // distribute, sublicense, and/or sell copies of the Software, and to permit
    // persons to whom the Software is furnished to do so, subject to the
    // following conditions:
    //
    // 1. Redistributions of source code must retain the above copyright
    //    notice, this deque of conditions and the following disclaimers.
    // 2. Redistributions in binary form must reproduce the above copyright
    //    notice in the documentation and/or other materials provided with
    //    the distribution.
    // 3. The name of W. Randolph Franklin may not be used to endorse or
    //    promote products derived from this Software without specific
    //    prior written permission.
    //
    // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
    // OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
    // NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
    // DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
    // OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
    // USE OR OTHER DEALINGS IN THE SOFTWARE.
    bool contains(double latitude, double longitude);

    /** Returns true if the box crosses any edge in this edge subtree */
    bool crosses(double minLat, double maxLat, double minLon, double maxLon);
  };

  /**
   * Creates an edge interval tree from a set of polygon vertices.
   * @return root node of the tree.
   */
private:
  static std::shared_ptr<Edge> createTree(std::deque<double> &polyLats,
                                          std::deque<double> &polyLons);

  /** Creates tree from sorted edges (with range low and high inclusive) */
  static std::shared_ptr<Edge>
  createTree(std::deque<std::shared_ptr<Edge>> &edges, int low, int high);

  /**
   * Returns a positive value if points a, b, and c are arranged in
   * counter-clockwise order, negative value if clockwise, zero if collinear.
   */
  // see the "Orient2D" method described here:
  // http://www.cs.berkeley.edu/~jrs/meshpapers/robnotes.pdf
  // https://www.cs.cmu.edu/~quake/robust.html
  // Note that this one does not yet have the floating point tricks to be exact!
  static int orient(double ax, double ay, double bx, double by, double cx,
                    double cy);
};

} // namespace org::apache::lucene::geo
