#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/MultiPhraseQuery.h"

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
 * random sloppy phrase query tests
 */
class TestSloppyPhraseQuery2 : public SearchEquivalenceTestBase
{
  GET_CLASS_NAME(TestSloppyPhraseQuery2)
  /** "A B"~N ⊆ "A B"~N+1 */
public:
  virtual void testIncreasingSloppiness() ;

  /** same as the above with posincr */
  virtual void testIncreasingSloppinessWithHoles() ;

  /** "A B C"~N ⊆ "A B C"~N+1 */
  virtual void testIncreasingSloppiness3() ;

  /** same as the above with posincr */
  virtual void testIncreasingSloppiness3WithHoles() ;

  /** "A A"~N ⊆ "A A"~N+1 */
  virtual void testRepetitiveIncreasingSloppiness() ;

  /** same as the above with posincr */
  virtual void
  testRepetitiveIncreasingSloppinessWithHoles() ;

  /** "A A A"~N ⊆ "A A A"~N+1 */
  virtual void testRepetitiveIncreasingSloppiness3() ;

  /** same as the above with posincr */
  virtual void
  testRepetitiveIncreasingSloppiness3WithHoles() ;

  /** MultiPhraseQuery~N ⊆ MultiPhraseQuery~N+1 */
  virtual void testRandomIncreasingSloppiness() ;

private:
  std::shared_ptr<MultiPhraseQuery> randomPhraseQuery(int64_t seed);

protected:
  std::shared_ptr<TestSloppyPhraseQuery2> shared_from_this()
  {
    return std::static_pointer_cast<TestSloppyPhraseQuery2>(
        SearchEquivalenceTestBase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
