#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

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
namespace org::apache::lucene::search
{

/**
 * Basic equivalence tests for approximations.
 */
class TestApproximationSearchEquivalence : public SearchEquivalenceTestBase
{
  GET_CLASS_NAME(TestApproximationSearchEquivalence)

public:
  virtual void testConjunction() ;

  virtual void testNestedConjunction() ;

  virtual void testDisjunction() ;

  virtual void testNestedDisjunction() ;

  virtual void testDisjunctionInConjunction() ;

  virtual void testConjunctionInDisjunction() ;

  virtual void testConstantScore() ;

  virtual void testExclusion() ;

  virtual void testNestedExclusion() ;

  virtual void testReqOpt() ;

protected:
  std::shared_ptr<TestApproximationSearchEquivalence> shared_from_this()
  {
    return std::static_pointer_cast<TestApproximationSearchEquivalence>(
        SearchEquivalenceTestBase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
