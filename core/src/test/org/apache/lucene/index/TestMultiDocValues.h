#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}

namespace org::apache::lucene::index
{
class DocValuesIterator;
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
namespace org::apache::lucene::index
{

using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

/** Tests MultiDocValues versus ordinary segment merging */
class TestMultiDocValues : public LuceneTestCase
{
  GET_CLASS_NAME(TestMultiDocValues)

public:
  virtual void testNumerics() ;

  virtual void testBinary() ;

  virtual void testSorted() ;

  // tries to make more dups than testSorted
  virtual void testSortedWithLotsOfDups() ;

  virtual void testSortedSet() ;

  // tries to make more dups than testSortedSet
  virtual void testSortedSetWithDups() ;

  virtual void testSortedNumeric() ;

private:
  void
  testRandomAdvance(std::shared_ptr<DocIdSetIterator> iter1,
                    std::shared_ptr<DocIdSetIterator> iter2) ;

  void testRandomAdvanceExact(std::shared_ptr<DocValuesIterator> iter1,
                              std::shared_ptr<DocValuesIterator> iter2,
                              int maxDoc) ;

protected:
  std::shared_ptr<TestMultiDocValues> shared_from_this()
  {
    return std::static_pointer_cast<TestMultiDocValues>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
