#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}

namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util
{
class Bits;
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
namespace org::apache::lucene::search::join
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestBlockJoinSelector : public LuceneTestCase
{
  GET_CLASS_NAME(TestBlockJoinSelector)

public:
  virtual void testDocsWithValue();

  static void assertNoMoreDoc(std::shared_ptr<DocIdSetIterator> sdv,
                              int maxDoc) ;

  static int nextDoc(std::shared_ptr<DocIdSetIterator> sdv,
                     int docId) ;

private:
  static bool advanceExact(std::shared_ptr<DocIdSetIterator> sdv,
                           int target) ;

public:
  virtual void testSortedSelector() ;

private:
  class CannedSortedDocValues : public SortedDocValues
  {
    GET_CLASS_NAME(CannedSortedDocValues)
  private:
    std::deque<int> const ords;

  public:
    // C++ NOTE: Fields cannot have the same name as methods:
    int docID_ = -1;

    CannedSortedDocValues(std::deque<int> &ords);

    int docID() override;

    int nextDoc() override;

    int advance(int target) override;

    bool advanceExact(int target)  override;

    int ordValue() override;

    int64_t cost() override;

    std::shared_ptr<BytesRef> lookupOrd(int ord) override;

    int getValueCount() override;

  protected:
    std::shared_ptr<CannedSortedDocValues> shared_from_this()
    {
      return std::static_pointer_cast<CannedSortedDocValues>(
          org.apache.lucene.index.SortedDocValues::shared_from_this());
    }
  };

public:
  virtual void testNumericSelector() ;

private:
  class CannedNumericDocValues : public NumericDocValues
  {
    GET_CLASS_NAME(CannedNumericDocValues)
  public:
    const std::shared_ptr<Bits> docsWithValue;
    std::deque<int64_t> const values;
    // C++ NOTE: Fields cannot have the same name as methods:
    int docID_ = -1;

    CannedNumericDocValues(std::deque<int64_t> &values,
                           std::shared_ptr<Bits> docsWithValue);

    int docID() override;

    int nextDoc() override;

    int advance(int target) override;

    bool advanceExact(int target)  override;

    int64_t longValue() override;

    int64_t cost() override;

  protected:
    std::shared_ptr<CannedNumericDocValues> shared_from_this()
    {
      return std::static_pointer_cast<CannedNumericDocValues>(
          org.apache.lucene.index.NumericDocValues::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestBlockJoinSelector> shared_from_this()
  {
    return std::static_pointer_cast<TestBlockJoinSelector>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::join
