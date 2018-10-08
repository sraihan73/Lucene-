#pragma once
#include "stringhelper.h"
#include <cmath>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/SortedNumericDocValues.h"

#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/LeafFieldComparator.h"

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

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using FieldComparator = org::apache::lucene::search::FieldComparator;
using LeafFieldComparator = org::apache::lucene::search::LeafFieldComparator;
using Scorer = org::apache::lucene::search::Scorer;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLatitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLongitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitude;

/**
 * Compares documents by distance from an origin point
 * <p>
 * When the least competitive item on the priority queue changes (setBottom), we
 * recompute a bounding box representing competitive distance to the top-N. Then
 * in compareBottom, we can quickly reject hits based on bounding box alone
 * without computing distance for every element.
 */
class LatLonPointDistanceComparator : public FieldComparator<double>,
                                      public LeafFieldComparator
{
  GET_CLASS_NAME(LatLonPointDistanceComparator)
public:
  const std::wstring field;
  const double latitude;
  const double longitude;

  std::deque<double> const values;
  double bottom = 0;
  double topValue = 0;
  std::shared_ptr<SortedNumericDocValues> currentDocs;

  // current bounding box(es) for the bottom distance on the PQ.
  // these are pre-encoded with LatLonPoint's encoding and
  // used to exclude uncompetitive hits faster.
  int minLon = std::numeric_limits<int>::min();
  int maxLon = std::numeric_limits<int>::max();
  int minLat = std::numeric_limits<int>::min();
  int maxLat = std::numeric_limits<int>::max();

  // second set of longitude ranges to check (for cross-dateline case)
  int minLon2 = std::numeric_limits<int>::max();

  // the number of times setBottom has been called (adversary protection)
  int setBottomCounter = 0;

private:
  std::deque<int64_t> currentValues = std::deque<int64_t>(4);
  int valuesDocID = -1;

public:
  LatLonPointDistanceComparator(const std::wstring &field, double latitude,
                                double longitude, int numHits);

  void setScorer(std::shared_ptr<Scorer> scorer) override;
  int compare(int slot1, int slot2) override;

  void setBottom(int slot) override;

  void setTopValue(std::optional<double> &value) override;

private:
  void setValues() ;

public:
  int compareBottom(int doc)  override;

  void copy(int slot, int doc)  override;

  std::shared_ptr<LeafFieldComparator> getLeafComparator(
      std::shared_ptr<LeafReaderContext> context)  override;

  std::optional<double> value(int slot) override;

  int compareTop(int doc)  override;

  // TODO: optimize for single-valued case?
  // TODO: do all kinds of other optimizations!
  virtual double sortKey(int doc) ;

  // second half of the haversin calculation, used to convert results from
  // haversin1 (used internally for sorting) for display purposes.
  static double haversin2(double partial_);

protected:
  std::shared_ptr<LatLonPointDistanceComparator> shared_from_this()
  {
    return std::static_pointer_cast<LatLonPointDistanceComparator>(
        org.apache.lucene.search.FieldComparator<double>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/document/
