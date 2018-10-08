#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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
namespace org::apache::lucene::index
{

using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using namespace org::apache::lucene::util::automaton;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

class TestTermsEnum2 : public LuceneTestCase
{
  GET_CLASS_NAME(TestTermsEnum2)
private:
  std::shared_ptr<Directory> dir;
  std::shared_ptr<IndexReader> reader;
  std::shared_ptr<IndexSearcher> searcher;
  std::shared_ptr<SortedSet<std::shared_ptr<BytesRef>>>
      terms;                                 // the terms we put in the index
  std::shared_ptr<Automaton> termsAutomaton; // automata of the same
public:
  int numIterations = 0;

  void setUp()  override;

  void tearDown()  override;

  /** tests a pre-intersected automaton against the original */
  virtual void testFiniteVersusInfinite() ;

  /** seeks to every term accepted by some automata */
  virtual void testSeeking() ;

  /** mixes up seek and next for all terms */
  virtual void testSeekingAndNexting() ;

  /** tests intersect: TODO start at a random term! */
  virtual void testIntersect() ;

protected:
  std::shared_ptr<TestTermsEnum2> shared_from_this()
  {
    return std::static_pointer_cast<TestTermsEnum2>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
