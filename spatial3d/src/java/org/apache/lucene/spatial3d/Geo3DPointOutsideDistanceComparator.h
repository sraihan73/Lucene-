#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoOutsideDistance.h"

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
namespace org::apache::lucene::spatial3d
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using FieldComparator = org::apache::lucene::search::FieldComparator;
using LeafFieldComparator = org::apache::lucene::search::LeafFieldComparator;
using Scorer = org::apache::lucene::search::Scorer;
using GeoOutsideDistance =
    org::apache::lucene::spatial3d::geom::GeoOutsideDistance;

/**
 * Compares documents by outside distance, using a GeoOutsideDistance to compute
 * the distance
 */
class Geo3DPointOutsideDistanceComparator : public FieldComparator<double>,
                                            public LeafFieldComparator
{
  GET_CLASS_NAME(Geo3DPointOutsideDistanceComparator)
public:
  const std::wstring field;

  const std::shared_ptr<GeoOutsideDistance> distanceShape;

  std::deque<double> const values;
  double bottomDistance = 0;
  double topValue = 0;
  std::shared_ptr<SortedNumericDocValues> currentDocs;

  Geo3DPointOutsideDistanceComparator(
      const std::wstring &field,
      std::shared_ptr<GeoOutsideDistance> distanceShape, int numHits);

  void setScorer(std::shared_ptr<Scorer> scorer) override;
  int compare(int slot1, int slot2) override;

  void setBottom(int slot) override;

  void setTopValue(std::optional<double> &value) override;

  int compareBottom(int doc)  override;

  void copy(int slot, int doc)  override;

  std::shared_ptr<LeafFieldComparator> getLeafComparator(
      std::shared_ptr<LeafReaderContext> context)  override;

  std::optional<double> value(int slot) override;

  int compareTop(int doc)  override;

  virtual double computeMinimumDistance(int const doc) ;

protected:
  std::shared_ptr<Geo3DPointOutsideDistanceComparator> shared_from_this()
  {
    return std::static_pointer_cast<Geo3DPointOutsideDistanceComparator>(
        org.apache.lucene.search.FieldComparator<double>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/
