#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/geo/GeoEncodingUtils.h"
#include  "core/src/java/org/apache/lucene/geo/DistancePredicate.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/ScorerSupplier.h"
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"
#include  "core/src/java/org/apache/lucene/index/PointValues.h"
#include  "core/src/java/org/apache/lucene/util/DocIdSetBuilder.h"
#include  "core/src/java/org/apache/lucene/index/IntersectVisitor.h"
#include  "core/src/java/org/apache/lucene/util/BulkAdder.h"
#include  "core/src/java/org/apache/lucene/util/FixedBitSet.h"

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

/**
 * Distance query for {@link LatLonPoint}.
 */
class LatLonPointDistanceQuery final : public Query
{
  GET_CLASS_NAME(LatLonPointDistanceQuery)
public:
  const std::wstring field;
  const double latitude;
  const double longitude;
  const double radiusMeters;

  LatLonPointDistanceQuery(const std::wstring &field, double latitude,
                           double longitude, double radiusMeters);

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
  {
    GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
  private:
    std::shared_ptr<LatLonPointDistanceQuery> outerInstance;

    std::deque<char> minLat;
    std::deque<char> maxLat;
    std::deque<char> minLon;
    std::deque<char> maxLon;
    std::deque<char> minLon2;
    double sortKey = 0;
    double axisLat = 0;

  public:
    ConstantScoreWeightAnonymousInnerClass(
        std::shared_ptr<LatLonPointDistanceQuery> outerInstance, float boost,
        std::deque<char> &minLat, std::deque<char> &maxLat,
        std::deque<char> &minLon, std::deque<char> &maxLon,
        std::deque<char> &minLon2, double sortKey, double axisLat);

    const std::shared_ptr<GeoEncodingUtils::DistancePredicate>
        distancePredicate;

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    std::shared_ptr<ScorerSupplier> scorerSupplier(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class ScorerSupplierAnonymousInnerClass : public ScorerSupplier
    {
      GET_CLASS_NAME(ScorerSupplierAnonymousInnerClass)
    private:
      std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance;

      std::shared_ptr<LeafReader> reader;
      std::shared_ptr<PointValues> values;
      std::shared_ptr<DocIdSetBuilder> result;
      std::shared_ptr<PointValues::IntersectVisitor> visitor;
      std::shared_ptr<Weight> weight;

    public:
      ScorerSupplierAnonymousInnerClass(
          std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
          std::shared_ptr<LeafReader> reader,
          std::shared_ptr<PointValues> values,
          std::shared_ptr<DocIdSetBuilder> result,
          std::shared_ptr<PointValues::IntersectVisitor> visitor,
          std::shared_ptr<Weight> weight);

      int64_t cost = 0;

      std::shared_ptr<Scorer>
      get(int64_t leadCost)  override;

      int64_t cost() override;

    protected:
      std::shared_ptr<ScorerSupplierAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ScorerSupplierAnonymousInnerClass>(
            org.apache.lucene.search.ScorerSupplier::shared_from_this());
      }
    };

    /**
     * Create a visitor that collects documents matching the range.
     */
  private:
    std::shared_ptr<PointValues::IntersectVisitor>
    getIntersectVisitor(std::shared_ptr<DocIdSetBuilder> result);

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

      // algorithm: we create a bounding box (two bounding boxes if we cross the
      // dateline).
      // 1. check our bounding box(es) first. if the subtree is entirely outside
      // of those, bail.
      // 2. check if the subtree is disjoint. it may cross the bounding box but
      // not intersect with circle
      // 3. see if the subtree is fully contained. if the subtree is enormous
      // along the x axis, wrapping half way around the world, etc: then this
      // can't work, just go to step 4.
      // 4. recurse naively (subtrees crossing over circle edge)
      PointValues::Relation compare(std::deque<char> &minPackedValue,
                                    std::deque<char> &maxPackedValue);
    };

    /**
     * Create a visitor that clears documents that do NOT match the range.
     */
  private:
    std::shared_ptr<PointValues::IntersectVisitor>
    getInverseIntersectVisitor(std::shared_ptr<FixedBitSet> result,
                               std::deque<int> &cost);

  private:
    class IntersectVisitorAnonymousInnerClass2
        : public std::enable_shared_from_this<
              IntersectVisitorAnonymousInnerClass2>,
          public PointValues::IntersectVisitor
    {
      GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass2)
    private:
      std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance;

      std::shared_ptr<FixedBitSet> result;
      std::deque<int> cost;

    public:
      IntersectVisitorAnonymousInnerClass2(
          std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
          std::shared_ptr<FixedBitSet> result, std::deque<int> &cost);

      void visit(int docID) override;

      void visit(int docID, std::deque<char> &packedValue) override;

      PointValues::Relation compare(std::deque<char> &minPackedValue,
                                    std::deque<char> &maxPackedValue);
    };

  protected:
    std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ConstantScoreWeightAnonymousInnerClass>(
          org.apache.lucene.search.ConstantScoreWeight::shared_from_this());
    }
  };

public:
  std::wstring getField();

  double getLatitude();

  double getLongitude();

  double getRadiusMeters();

  virtual int hashCode();

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<LatLonPointDistanceQuery> other);

public:
  std::wstring toString(const std::wstring &field) override;

protected:
  std::shared_ptr<LatLonPointDistanceQuery> shared_from_this()
  {
    return std::static_pointer_cast<LatLonPointDistanceQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/document/
