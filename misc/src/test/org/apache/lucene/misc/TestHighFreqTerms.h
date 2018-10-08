#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexWriter;
}

namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class IndexReader;
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
namespace org::apache::lucene::misc
{

using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestHighFreqTerms : public LuceneTestCase
{
  GET_CLASS_NAME(TestHighFreqTerms)

private:
  static std::shared_ptr<IndexWriter> writer;
  static std::shared_ptr<Directory> dir;
  static std::shared_ptr<IndexReader> reader;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void setUpClass() throws
  // Exception
  static void setUpClass() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void tearDownClass() throws
  // Exception
  static void tearDownClass() ;
  /******************** Tests for getHighFreqTerms
   * **********************************/

  // test without specifying field (i.e. if we pass in field=null it should
  // examine all fields) the term "diff" in the field "different_field" occurs
  // 20 times and is the highest df term
  virtual void testFirstTermHighestDocFreqAllFields() ;

  virtual void testFirstTermHighestDocFreq() ;

  virtual void testOrderedByDocFreqDescending() ;

  virtual void testNumTerms() ;

  virtual void testGetHighFreqTerms() ;

  /********************Test
   * sortByTotalTermFreq**********************************/

  virtual void testFirstTermHighestTotalTermFreq() ;

  virtual void
  testFirstTermHighestTotalTermFreqDifferentField() ;

  virtual void testOrderedByTermFreqDescending() ;

  virtual void testGetTermFreqOrdered() ;

  /********************Testing Utils**********************************/

private:
  static void
  indexDocs(std::shared_ptr<IndexWriter> writer) ;

  /**
   *  getContent
   *  return string containing numbers 1 to i with each number n occurring n
   * times. i.e. for input of 3 return string "3 3 3 2 2 1"
   */

  static std::wstring getContent(int i);

  static int getExpectedtotalTermFreq(int i);

  static int getExpecteddocFreq(int i);

protected:
  std::shared_ptr<TestHighFreqTerms> shared_from_this()
  {
    return std::static_pointer_cast<TestHighFreqTerms>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::misc
