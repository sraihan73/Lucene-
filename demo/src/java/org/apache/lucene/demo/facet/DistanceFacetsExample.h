#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include "exceptionhelper.h"
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::facet::range
{
class DoubleRange;
}

namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::facet
{
class FacetsConfig;
}
namespace org::apache::lucene::search
{
class DoubleValuesSource;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::facet
{
class FacetResult;
}
namespace org::apache::lucene::search
{
class TopDocs;
}
namespace org::apache::lucene::facet::taxonomy
{
class TaxonomyReader;
}
namespace org::apache::lucene::facet
{
class Facets;
}
namespace org::apache::lucene::facet
{
class FacetsCollector;
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
namespace org::apache::lucene::demo::facet
{

using FacetResult = org::apache::lucene::facet::FacetResult;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using DoubleRange = org::apache::lucene::facet::range::DoubleRange;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;

/** Shows simple usage of dynamic range faceting, using the
 *  expressions module to calculate distance. */
class DistanceFacetsExample
    : public std::enable_shared_from_this<DistanceFacetsExample>
{
  GET_CLASS_NAME(DistanceFacetsExample)

public:
  const std::shared_ptr<DoubleRange> ONE_KM =
      std::make_shared<DoubleRange>(L"< 1 km", 0.0, true, 1.0, false);
  const std::shared_ptr<DoubleRange> TWO_KM =
      std::make_shared<DoubleRange>(L"< 2 km", 0.0, true, 2.0, false);
  const std::shared_ptr<DoubleRange> FIVE_KM =
      std::make_shared<DoubleRange>(L"< 5 km", 0.0, true, 5.0, false);
  const std::shared_ptr<DoubleRange> TEN_KM =
      std::make_shared<DoubleRange>(L"< 10 km", 0.0, true, 10.0, false);

private:
  const std::shared_ptr<Directory> indexDir = std::make_shared<RAMDirectory>();
  std::shared_ptr<IndexSearcher> searcher;
  const std::shared_ptr<FacetsConfig> config = std::make_shared<FacetsConfig>();

  /** The "home" latitude. */
public:
  static constexpr double ORIGIN_LATITUDE = 40.7143528;

  /** The "home" longitude. */
  static constexpr double ORIGIN_LONGITUDE = -74.0059731;

  /** Mean radius of the Earth in KM
   *
   * NOTE: this is approximate, because the earth is a bit
   * wider at the equator than the poles.  See
   * http://en.wikipedia.org/wiki/Earth_radius */
  // see http://earth-info.nga.mil/GandG/publications/tr8350.2/wgs84fin.pdf
  static constexpr double EARTH_RADIUS_KM = 6'371.0087714;

  /** Empty constructor */
  DistanceFacetsExample();

  /** Build the example index. */
  virtual void index() ;

private:
  std::shared_ptr<DoubleValuesSource> getDistanceValueSource();

  /** Given a latitude and longitude (in degrees) and the
   *  maximum great circle (surface of the earth) distance,
   *  returns a simple Filter bounding box to "fast match"
   *  candidates. */
public:
  static std::shared_ptr<Query>
  getBoundingBoxQuery(double originLat, double originLng, double maxDistanceKM);

  /** User runs a query and counts facets. */
  virtual std::shared_ptr<FacetResult> search() ;

  /** User drills down on the specified range. */
  virtual std::shared_ptr<TopDocs>
  drillDown(std::shared_ptr<DoubleRange> range) ;

private:
  class DrillSidewaysAnonymousInnerClass : public DrillSideways
  {
    GET_CLASS_NAME(DrillSidewaysAnonymousInnerClass)
  private:
    std::shared_ptr<DistanceFacetsExample> outerInstance;

    std::shared_ptr<DoubleValuesSource> vs;

  public:
    DrillSidewaysAnonymousInnerClass(
        std::shared_ptr<DistanceFacetsExample> outerInstance,
        std::shared_ptr<IndexSearcher> searcher,
        std::shared_ptr<FacetsConfig> config,
        std::shared_ptr<TaxonomyReader> org);

  protected:
    std::shared_ptr<Facets> buildFacetsResult(
        std::shared_ptr<FacetsCollector> drillDowns,
        std::deque<std::shared_ptr<FacetsCollector>> &drillSideways,
        std::deque<std::wstring> &drillSidewaysDims) 
        override;

  protected:
    std::shared_ptr<DrillSidewaysAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DrillSidewaysAnonymousInnerClass>(
          org.apache.lucene.facet.DrillSideways::shared_from_this());
    }
  };

public:
  virtual ~DistanceFacetsExample();

  /** Runs the search and drill-down examples and prints the results. */
  static void main(std::deque<std::wstring> &args) ;
};

} // namespace org::apache::lucene::demo::facet
