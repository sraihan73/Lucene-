#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_set>
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
namespace org::apache::lucene::analysis
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestCharArraySet : public LuceneTestCase
{
  GET_CLASS_NAME(TestCharArraySet)

public:
  static std::deque<std::wstring> const TEST_STOP_WORDS;

  virtual void testRehash() ;

  virtual void testNonZeroOffset();

  virtual void testObjectContains();

  virtual void testClear();

  // TODO: break this up into simpler test methods, vs "telling a story"
  virtual void testModifyOnUnmodifiable();

  virtual void testUnmodifiableSet();

  virtual void testSupplementaryChars();

  virtual void testSingleHighSurrogate();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("deprecated") public void
  // testCopyCharArraySetBWCompat()
  virtual void testCopyCharArraySetBWCompat();

  /**
   * Test the static #copy() function with a CharArraySet as a source
   */
  virtual void testCopyCharArraySet();

  /**
   * Test the static #copy() function with a JDK {@link Set} as a source
   */
  virtual void testCopyJDKSet();

  /**
   * Tests a special case of {@link CharArraySet#copy(Set)} where the
   * set to copy is the {@link CharArraySet#EMPTY_SET}
   */
  virtual void testCopyEmptySet();

  /**
   * Smoketests the static empty set
   */
  virtual void testEmptySet();

  /**
   * Test for NPE
   */
  virtual void testContainsWithNull();

  virtual void testToString();

protected:
  std::shared_ptr<TestCharArraySet> shared_from_this()
  {
    return std::static_pointer_cast<TestCharArraySet>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/
