#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
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
namespace org::apache::lucene::util
{

class TestMergedIterator : public LuceneTestCase
{
  GET_CLASS_NAME(TestMergedIterator)
private:
  static constexpr int REPEATS = 2;
  static constexpr int VALS_TO_MERGE = 15000;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"rawtypes", "unchecked"}) public void
  // testMergeEmpty()
  virtual void testMergeEmpty();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Repeat(iterations = REPEATS) public void
  // testNoDupsRemoveDups()
  virtual void testNoDupsRemoveDups();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Repeat(iterations = REPEATS) public void
  // testOffItrDupsRemoveDups()
  virtual void testOffItrDupsRemoveDups();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Repeat(iterations = REPEATS) public void
  // testOnItrDupsRemoveDups()
  virtual void testOnItrDupsRemoveDups();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Repeat(iterations = REPEATS) public void
  // testOnItrRandomDupsRemoveDups()
  virtual void testOnItrRandomDupsRemoveDups();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Repeat(iterations = REPEATS) public void
  // testBothDupsRemoveDups()
  virtual void testBothDupsRemoveDups();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Repeat(iterations = REPEATS) public void
  // testBothDupsWithRandomDupsRemoveDups()
  virtual void testBothDupsWithRandomDupsRemoveDups();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Repeat(iterations = REPEATS) public void
  // testNoDupsKeepDups()
  virtual void testNoDupsKeepDups();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Repeat(iterations = REPEATS) public void
  // testOffItrDupsKeepDups()
  virtual void testOffItrDupsKeepDups();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Repeat(iterations = REPEATS) public void
  // testOnItrDupsKeepDups()
  virtual void testOnItrDupsKeepDups();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Repeat(iterations = REPEATS) public void
  // testOnItrRandomDupsKeepDups()
  virtual void testOnItrRandomDupsKeepDups();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Repeat(iterations = REPEATS) public void
  // testBothDupsKeepDups()
  virtual void testBothDupsKeepDups();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Repeat(iterations = REPEATS) public void
  // testBothDupsWithRandomDupsKeepDups()
  virtual void testBothDupsWithRandomDupsKeepDups();

private:
  void testCase(int itrsWithVal, int specifiedValsOnItr, bool removeDups);

protected:
  std::shared_ptr<TestMergedIterator> shared_from_this()
  {
    return std::static_pointer_cast<TestMergedIterator>(
        LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
