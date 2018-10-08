#pragma once
#include "stringhelper.h"
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRefHash.h"

#include  "core/src/java/org/apache/lucene/util/ByteBlockPool.h"

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
namespace org::apache::lucene::util
{

class TestBytesRefHash : public LuceneTestCase
{
  GET_CLASS_NAME(TestBytesRefHash)

public:
  std::shared_ptr<BytesRefHash> hash;
  std::shared_ptr<ByteBlockPool> pool;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @Before public void setUp() throws Exception
  void setUp()  override;

private:
  std::shared_ptr<ByteBlockPool> newPool();

  std::shared_ptr<BytesRefHash>
  newHash(std::shared_ptr<ByteBlockPool> blockPool);

  /**
   * Test method for {@link org.apache.lucene.util.BytesRefHash#size()}.
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSize()
  virtual void testSize();

  /**
   * Test method for
   * {@link org.apache.lucene.util.BytesRefHash#get(int, BytesRef)}
   * .
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGet()
  virtual void testGet();

  /**
   * Test method for {@link org.apache.lucene.util.BytesRefHash#compact()}.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testCompact()
  virtual void testCompact();

  /**
   * Test method for
   * {@link org.apache.lucene.util.BytesRefHash#sort()}.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSort()
  virtual void testSort();

  /**
   * Test method for
   * {@link
   * org.apache.lucene.util.BytesRefHash#add(org.apache.lucene.util.BytesRef)}
   * .
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testAdd()
  virtual void testAdd();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFind() throws Exception
  virtual void testFind() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test(expected =
  // org.apache.lucene.util.BytesRefHash.MaxBytesLengthExceededException.class)
  // public void testLargeValue()
  virtual void testLargeValue();

  /**
   * Test method for
   * {@link org.apache.lucene.util.BytesRefHash#addByPoolOffset(int)}
   * .
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testAddByPoolOffset()
  virtual void testAddByPoolOffset();

private:
  void assertAllIn(std::shared_ptr<Set<std::wstring>> strings,
                   std::shared_ptr<BytesRefHash> hash);

protected:
  std::shared_ptr<TestBytesRefHash> shared_from_this()
  {
    return std::static_pointer_cast<TestBytesRefHash>(
        LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
