#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <typeinfo>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class DataInput;
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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using DataInput = org::apache::lucene::store::DataInput;

class TestIndexInput : public LuceneTestCase
{
  GET_CLASS_NAME(TestIndexInput)

public:
  static std::deque<char> const READ_TEST_BYTES;

  static const int COUNT = RANDOM_MULTIPLIER * 65536;
  static std::deque<int> INTS;
  static std::deque<int64_t> LONGS;
  static std::deque<char> RANDOM_TEST_BYTES;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass() throws
  // java.io.IOException
  static void beforeClass() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClass()
  static void afterClass();

private:
  void checkReads(std::shared_ptr<DataInput> is,
                  std::type_info expectedEx) ;

  void checkRandomReads(std::shared_ptr<DataInput> is) ;

  // this test checks the IndexInput methods of any impl
public:
  virtual void testRawIndexInputRead() ;

  virtual void testByteArrayDataInput() ;

protected:
  std::shared_ptr<TestIndexInput> shared_from_this()
  {
    return std::static_pointer_cast<TestIndexInput>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
