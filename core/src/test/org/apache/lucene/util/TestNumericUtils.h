#pragma once
#include "stringhelper.h"
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
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

/**
 * Tests for NumericUtils static methods.
 */
class TestNumericUtils : public LuceneTestCase
{
  GET_CLASS_NAME(TestNumericUtils)

  /**
   * generate a series of encoded longs, each numerical one bigger than the one
   * before. check for correct ordering of the encoded bytes and that values
   * round-trip.
   */
public:
  virtual void testLongConversionAndOrdering() ;

  /**
   * generate a series of encoded ints, each numerical one bigger than the one
   * before. check for correct ordering of the encoded bytes and that values
   * round-trip.
   */
  virtual void testIntConversionAndOrdering() ;

  /**
   * generate a series of encoded BigIntegers, each numerical one bigger than
   * the one before. check for correct ordering of the encoded bytes and that
   * values round-trip.
   */
  virtual void testBigIntConversionAndOrdering() ;

  /**
   * check extreme values of longs
   * check for correct ordering of the encoded bytes and that values round-trip.
   */
  virtual void testLongSpecialValues() ;

  /**
   * check extreme values of ints
   * check for correct ordering of the encoded bytes and that values round-trip.
   */
  virtual void testIntSpecialValues() ;

  /**
   * check extreme values of big integers (4 bytes)
   * check for correct ordering of the encoded bytes and that values round-trip.
   */
  virtual void testBigIntSpecialValues() ;

  /**
   * check various sorted values of doubles (including extreme values)
   * check for correct ordering of the encoded bytes and that values round-trip.
   */
  virtual void testDoubles() ;

  static std::deque<double> const DOUBLE_NANs;

  virtual void testSortableDoubleNaN();

  /**
   * check various sorted values of floats (including extreme values)
   * check for correct ordering of the encoded bytes and that values round-trip.
   */
  virtual void testFloats() ;

  static std::deque<float> const FLOAT_NANs;

  virtual void testSortableFloatNaN();

  virtual void testAdd() ;

  virtual void testIllegalAdd() ;

  virtual void testSubtract() ;

  virtual void testIllegalSubtract() ;

  /** test round-trip encoding of random integers */
  virtual void testIntsRoundTrip();

  /** test round-trip encoding of random longs */
  virtual void testLongsRoundTrip();

  /** test round-trip encoding of random floats */
  virtual void testFloatsRoundTrip();

  /** test round-trip encoding of random doubles */
  virtual void testDoublesRoundTrip();

  /** test round-trip encoding of random big integers */
  virtual void testBigIntsRoundTrip();

  /** check sort order of random integers consistent with Integer.compare */
  virtual void testIntsCompare();

  /** check sort order of random longs consistent with Long.compare */
  virtual void testLongsCompare();

  /** check sort order of random floats consistent with Float.compare */
  virtual void testFloatsCompare();

  /** check sort order of random doubles consistent with Double.compare */
  virtual void testDoublesCompare();

  /** check sort order of random bigintegers consistent with
   * int64_t.compareTo */
  virtual void testBigIntsCompare();

protected:
  std::shared_ptr<TestNumericUtils> shared_from_this()
  {
    return std::static_pointer_cast<TestNumericUtils>(
        LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
