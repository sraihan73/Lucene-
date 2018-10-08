#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

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
namespace org::apache::lucene::facet::taxonomy
{

using FacetTestCase = org::apache::lucene::facet::FacetTestCase;

class TestTaxonomyFacetSumValueSource : public FacetTestCase
{
  GET_CLASS_NAME(TestTaxonomyFacetSumValueSource)

public:
  virtual void testBasic() ;

  // LUCENE-5333
  virtual void testSparseFacets() ;

  virtual void testWrongIndexFieldName() ;

  virtual void testSumScoreAggregator() ;

  virtual void testNoScore() ;

  virtual void testWithScore() ;

  virtual void testRollupValues() ;

  virtual void testCountAndSumScore() ;

  virtual void testRandom() ;

protected:
  std::shared_ptr<TestTaxonomyFacetSumValueSource> shared_from_this()
  {
    return std::static_pointer_cast<TestTaxonomyFacetSumValueSource>(
        org.apache.lucene.facet.FacetTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::facet::taxonomy
