#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::geo
{
class Polygon;
}

namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class Weight;
}
namespace org::apache::lucene::geo
{
class Polygon2D;
}
namespace org::apache::lucene::geo
{
class GeoEncodingUtils;
}
namespace org::apache::lucene::geo
{
class PolygonPredicate;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::util
{
class DocIdSetBuilder;
}
namespace org::apache::lucene::util
{
class BulkAdder;
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
namespace org::apache::lucene::document
{

using Polygon = org::apache::lucene::geo::Polygon;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Weight = org::apache::lucene::search::Weight;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLatitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLongitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitude;

/** Finds all previously indexed points that fall within the specified polygons.
 *
 *  <p>The field must be indexed with using {@link
 * org.apache.lucene.document.LatLonPoint} added per document.
 *
 *  @lucene.experimental */

class LatLonPointInPolygonQuery final : public Query
{
  GET_CLASS_NAME(LatLonPointInPolygonQuery)
public:
  const std::wstring field;
  std::deque<std::shared_ptr<Polygon>> const polygons;

  LatLonPointInPolygonQuery(const std::wstring &field,
                            std::deque<std::shared_ptr<Polygon>> &polygons);

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
  {
    GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
  private:
    std::shared_ptr<LatLonPointInPolygonQuery> outerInstance;

    std::deque<char> minLat;
    std::deque<char> maxLat;
    std::deque<char> minLon;
    std::deque<char> maxLon;
    std::shared_ptr<Polygon2D> tree;
    std::shared_ptr<GeoEncodingUtils::PolygonPredicate> polygonPredicate;

  public:
    ConstantScoreWeightAnonymousInnerClass(
        std::shared_ptr<LatLonPointInPolygonQuery> outerInstance, float boost,
        std::deque<char> &minLat, std::deque<char> &maxLat,
        std::deque<char> &minLon, std::deque<char> &maxLon,
        std::shared_ptr<Polygon2D> tree,
        std::shared_ptr<GeoEncodingUtils::PolygonPredicate> polygonPredicate);

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class IntersectVisitorAnonymousInnerClass
        : public std::enable_shared_from_this<
              IntersectVisitorAnonymousInnerClass>,
          public PointValues::IntersectVisitor
    {
      GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass)
    private:
      std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance;

      std::shared_ptr<DocIdSetBuilder> result;

    public:
      IntersectVisitorAnonymousInnerClass(
          std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
          std::shared_ptr<DocIdSetBuilder> result);

      std::shared_ptr<DocIdSetBuilder::BulkAdder> adder;

      void grow(int count);

      void visit(int docID) override;

      void visit(int docID, std::deque<char> &packedValue) override;

      PointValues::Relation compare(std::deque<char> &minPackedValue,
                                    std::deque<char> &maxPackedValue);
    };

  public:
    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ConstantScoreWeightAnonymousInnerClass>(
          org.apache.lucene.search.ConstantScoreWeight::shared_from_this());
    }
  };

  /** Returns the query field */
public:
  std::wstring getField();

  /** Returns a copy of the internal polygon array */
  std::deque<std::shared_ptr<Polygon>> getPolygons();

  virtual int hashCode();

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<LatLonPointInPolygonQuery> other);

public:
  std::wstring toString(const std::wstring &field) override;

protected:
  std::shared_ptr<LatLonPointInPolygonQuery> shared_from_this()
  {
    return std::static_pointer_cast<LatLonPointInPolygonQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::document
