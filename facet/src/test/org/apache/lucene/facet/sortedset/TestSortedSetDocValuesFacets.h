#pragma once
#include "stringhelper.h"
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/facet/Facets.h"
#include  "core/src/java/org/apache/lucene/facet/sortedset/SortedSetDocValuesReaderState.h"

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
namespace org::apache::lucene::facet::sortedset
{

using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using Facets = org::apache::lucene::facet::Facets;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

class TestSortedSetDocValuesFacets : public FacetTestCase
{
  GET_CLASS_NAME(TestSortedSetDocValuesFacets)

  // NOTE: TestDrillSideways.testRandom also sometimes
  // randomly uses SortedSetDV

public:
  virtual void testBasic() ;

  // LUCENE-5090
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unused") public void testStaleState()
  // throws Exception
  virtual void testStaleState() ;

  // LUCENE-5333
  virtual void testSparseFacets() ;

  virtual void testSomeSegmentsMissing() ;

  virtual void testRandom() ;

private:
  static std::shared_ptr<Facets> getAllFacets(
      std::shared_ptr<IndexSearcher> searcher,
      std::shared_ptr<SortedSetDocValuesReaderState> state,
      std::shared_ptr<ExecutorService> exec) throw(IOException,
                                                   InterruptedException);

  std::shared_ptr<ExecutorService> randomExecutorServiceOrNull();

protected:
  std::shared_ptr<TestSortedSetDocValuesFacets> shared_from_this()
  {
    return std::static_pointer_cast<TestSortedSetDocValuesFacets>(
        org.apache.lucene.facet.FacetTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/sortedset/
