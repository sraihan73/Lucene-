#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Sort;
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
namespace org::apache::lucene::search
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/*
 * Very simple tests of sorting.
 *
 * THE RULES:
 * 1. keywords like 'abstract' and 'static' should not appear in this file.
 * 2. each test method should be self-contained and understandable.
 * 3. no test methods should share code with other test methods.
 * 4. no testing of things unrelated to sorting.
 * 5. no tracers.
 * 6. keyword 'class' should appear only once in this file, here ----
 *                                                                  |
 *        -----------------------------------------------------------
 *        |
 *       \./
 */
class TestSort : public LuceneTestCase
{
  GET_CLASS_NAME(TestSort)

private:
  void assertEquals(std::shared_ptr<Sort> a, std::shared_ptr<Sort> b);

  void assertDifferent(std::shared_ptr<Sort> a, std::shared_ptr<Sort> b);

public:
  virtual void testEquals();

  /** Tests sorting on type string */
  virtual void testString() ;

  /** Tests reverse sorting on type string */
  virtual void testStringReverse() ;

  /** Tests sorting on type string_val */
  virtual void testStringVal() ;

  /** Tests reverse sorting on type string_val */
  virtual void testStringValReverse() ;

  /** Tests sorting on type string_val, but with a SortedDocValuesField */
  virtual void testStringValSorted() ;

  /** Tests reverse sorting on type string_val, but with a SortedDocValuesField
   */
  virtual void testStringValReverseSorted() ;

  /** Tests sorting on type int */
  virtual void testInt() ;

  /** Tests sorting on type int in reverse */
  virtual void testIntReverse() ;

  /** Tests sorting on type int with a missing value */
  virtual void testIntMissing() ;

  /** Tests sorting on type int, specifying the missing value should be treated
   * as Integer.MAX_VALUE */
  virtual void testIntMissingLast() ;

  /** Tests sorting on type long */
  virtual void testLong() ;

  /** Tests sorting on type long in reverse */
  virtual void testLongReverse() ;

  /** Tests sorting on type long with a missing value */
  virtual void testLongMissing() ;

  /** Tests sorting on type long, specifying the missing value should be treated
   * as Long.MAX_VALUE */
  virtual void testLongMissingLast() ;

  /** Tests sorting on type float */
  virtual void testFloat() ;

  /** Tests sorting on type float in reverse */
  virtual void testFloatReverse() ;

  /** Tests sorting on type float with a missing value */
  virtual void testFloatMissing() ;

  /** Tests sorting on type float, specifying the missing value should be
   * treated as Float.MAX_VALUE */
  virtual void testFloatMissingLast() ;

  /** Tests sorting on type double */
  virtual void testDouble() ;

  /** Tests sorting on type double with +/- zero */
  virtual void testDoubleSignedZero() ;

  /** Tests sorting on type double in reverse */
  virtual void testDoubleReverse() ;

  /** Tests sorting on type double with a missing value */
  virtual void testDoubleMissing() ;

  /** Tests sorting on type double, specifying the missing value should be
   * treated as Double.MAX_VALUE */
  virtual void testDoubleMissingLast() ;

  /** Tests sorting on multiple sort fields */
  virtual void testMultiSort() ;

protected:
  std::shared_ptr<TestSort> shared_from_this()
  {
    return std::static_pointer_cast<TestSort>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
