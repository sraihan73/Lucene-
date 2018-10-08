#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class LeafReaderContext;
}

namespace org::apache::lucene::util
{
class Bits;
}
namespace org::apache::lucene::store
{
class Directory;
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
namespace org::apache::lucene::search::suggest::document
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using BitsProducer = org::apache::lucene::search::suggest::BitsProducer;
using Directory = org::apache::lucene::store::Directory;
using Bits = org::apache::lucene::util::Bits;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.search.suggest.document.TestSuggestField.Entry;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.search.suggest.document.TestSuggestField.assertSuggestions;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.search.suggest.document.TestSuggestField.iwcWithSuggestField;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.hamcrest.core.IsEqual.equalTo;

class TestPrefixCompletionQuery : public LuceneTestCase
{
  GET_CLASS_NAME(TestPrefixCompletionQuery)

private:
  class NumericRangeBitsProducer : public BitsProducer
  {
    GET_CLASS_NAME(NumericRangeBitsProducer)

  private:
    const std::wstring field;
    const int64_t min, max;

  public:
    NumericRangeBitsProducer(const std::wstring &field, int64_t min,
                             int64_t max);

    virtual std::wstring toString();

    bool equals(std::any obj) override;

    virtual int hashCode();

    std::shared_ptr<Bits> getBits(
        std::shared_ptr<LeafReaderContext> context)  override;

  protected:
    std::shared_ptr<NumericRangeBitsProducer> shared_from_this()
    {
      return std::static_pointer_cast<NumericRangeBitsProducer>(
          org.apache.lucene.search.suggest.BitsProducer::shared_from_this());
    }
  };

public:
  std::shared_ptr<Directory> dir;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Before public void before() throws Exception
  virtual void before() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @After public void after() throws Exception
  virtual void after() ;

  virtual void testSimple() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testEmptyPrefixQuery() throws Exception
  virtual void testEmptyPrefixQuery() ;

  virtual void testMostlyFilteredOutDocuments() ;

  virtual void testDocFiltering() ;

  virtual void testAnalyzerDefaults() ;

  virtual void testAnalyzerWithoutSeparator() ;

  virtual void testAnalyzerNoPreservePosInc() ;

  virtual void testGhostField() ;

  virtual void testEmptyPrefixContextQuery() ;

protected:
  std::shared_ptr<TestPrefixCompletionQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestPrefixCompletionQuery>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest::document
