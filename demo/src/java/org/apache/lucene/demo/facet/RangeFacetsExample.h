#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/facet/range/LongRange.h"
#include  "core/src/java/org/apache/lucene/facet/FacetsConfig.h"
#include  "core/src/java/org/apache/lucene/facet/FacetResult.h"
#include  "core/src/java/org/apache/lucene/search/TopDocs.h"
#include  "core/src/java/org/apache/lucene/facet/DrillSideways.h"
#include  "core/src/java/org/apache/lucene/facet/DrillSidewaysResult.h"
#include  "core/src/java/org/apache/lucene/facet/Facets.h"
#include  "core/src/java/org/apache/lucene/facet/FacetsCollector.h"

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

using DrillSideways = org::apache::lucene::facet::DrillSideways;
using FacetResult = org::apache::lucene::facet::FacetResult;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using LongRange = org::apache::lucene::facet::range::LongRange;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;

/** Shows simple usage of dynamic range faceting. */
class RangeFacetsExample
    : public std::enable_shared_from_this<RangeFacetsExample>
{
  GET_CLASS_NAME(RangeFacetsExample)

private:
  const std::shared_ptr<Directory> indexDir = std::make_shared<RAMDirectory>();
  std::shared_ptr<IndexSearcher> searcher;
  const int64_t nowSec = System::currentTimeMillis();

public:
  const std::shared_ptr<LongRange> PAST_HOUR = std::make_shared<LongRange>(
      L"Past hour", nowSec - 3600, true, nowSec, true);
  const std::shared_ptr<LongRange> PAST_SIX_HOURS = std::make_shared<LongRange>(
      L"Past six hours", nowSec - 6 * 3600, true, nowSec, true);
  const std::shared_ptr<LongRange> PAST_DAY = std::make_shared<LongRange>(
      L"Past day", nowSec - 24 * 3600, true, nowSec, true);

  /** Empty constructor */
  RangeFacetsExample();

  /** Build the example index. */
  virtual void index() ;

private:
  std::shared_ptr<FacetsConfig> getConfig();

  /** User runs a query and counts facets. */
public:
  virtual std::shared_ptr<FacetResult> search() ;

  /** User drills down on the specified range. */
  virtual std::shared_ptr<TopDocs>
  drillDown(std::shared_ptr<LongRange> range) ;

  /** User drills down on the specified range, and also computes drill sideways
   * counts. */
  virtual std::shared_ptr<DrillSideways::DrillSidewaysResult>
  drillSideways(std::shared_ptr<LongRange> range) ;

private:
  class DrillSidewaysAnonymousInnerClass : public DrillSideways
  {
    GET_CLASS_NAME(DrillSidewaysAnonymousInnerClass)
  private:
    std::shared_ptr<RangeFacetsExample> outerInstance;

  public:
    DrillSidewaysAnonymousInnerClass(
        std::shared_ptr<RangeFacetsExample> outerInstance,
        std::shared_ptr<IndexSearcher> searcher,
        std::shared_ptr<FacetsConfig> getConfig);

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
  virtual ~RangeFacetsExample();

  /** Runs the search and drill-down examples and prints the results. */
  static void main(std::deque<std::wstring> &args) ;
};

} // #include  "core/src/java/org/apache/lucene/demo/facet/
