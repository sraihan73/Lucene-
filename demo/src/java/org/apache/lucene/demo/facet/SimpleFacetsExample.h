#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::facet
{
class FacetsConfig;
}
namespace org::apache::lucene::facet
{
class FacetResult;
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
using Directory = org::apache::lucene::store::Directory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;

/** Shows simple usage of faceted indexing and search. */
class SimpleFacetsExample
    : public std::enable_shared_from_this<SimpleFacetsExample>
{
  GET_CLASS_NAME(SimpleFacetsExample)

private:
  const std::shared_ptr<Directory> indexDir = std::make_shared<RAMDirectory>();
  const std::shared_ptr<Directory> taxoDir = std::make_shared<RAMDirectory>();
  const std::shared_ptr<FacetsConfig> config = std::make_shared<FacetsConfig>();

  /** Empty constructor */
public:
  SimpleFacetsExample();

  /** Build the example index. */
private:
  void index() ;

  /** User runs a query and counts facets. */
  std::deque<std::shared_ptr<FacetResult>>
  facetsWithSearch() ;

  /** User runs a query and counts facets only without collecting the matching
   * documents.*/
  std::deque<std::shared_ptr<FacetResult>> facetsOnly() ;

  /** User drills down on 'Publish Date/2010', and we
   *  return facets for 'Author' */
  std::shared_ptr<FacetResult> drillDown() ;

  /** User drills down on 'Publish Date/2010', and we
   *  return facets for both 'Publish Date' and 'Author',
   *  using DrillSideways. */
  std::deque<std::shared_ptr<FacetResult>> drillSideways() ;

  /** Runs the search example. */
public:
  virtual std::deque<std::shared_ptr<FacetResult>>
  runFacetOnly() ;

  /** Runs the search example. */
  virtual std::deque<std::shared_ptr<FacetResult>>
  runSearch() ;

  /** Runs the drill-down example. */
  virtual std::shared_ptr<FacetResult> runDrillDown() ;

  /** Runs the drill-sideways example. */
  virtual std::deque<std::shared_ptr<FacetResult>>
  runDrillSideways() ;

  /** Runs the search and drill-down examples and prints the results. */
  static void main(std::deque<std::wstring> &args) ;
};

} // namespace org::apache::lucene::demo::facet
