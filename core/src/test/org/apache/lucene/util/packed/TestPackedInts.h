#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/packed/PackedInts.h"

#include  "core/src/java/org/apache/lucene/util/packed/Mutable.h"

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
namespace org::apache::lucene::util::packed
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestPackedInts : public LuceneTestCase
{
  GET_CLASS_NAME(TestPackedInts)

public:
  virtual void testByteCount();

  virtual void testBitsRequired();

  virtual void testMaxValues();

  virtual void testPackedInts() ;

  virtual void testEndPointer() ;

  virtual void testControlledEquality();

  virtual void testRandomBulkCopy();

  virtual void testRandomEquality();

private:
  static void assertRandomEquality(int valueCount, int bitsPerValue,
                                   int64_t randomSeed);

  static std::deque<std::shared_ptr<PackedInts::Mutable>>
  createPackedInts(int valueCount, int bitsPerValue);

  static void fill(std::shared_ptr<PackedInts::Mutable> packedInt,
                   int64_t maxValue, int64_t randomSeed);

  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: private static void
  // assertListEquality(java.util.List<? extends PackedInts.Reader> packedInts)
  static void assertListEquality(std::deque<T1> packedInts);

  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: private static void assertListEquality(std::wstring
  // message, java.util.List<? extends PackedInts.Reader> packedInts)
  static void assertListEquality(const std::wstring &message,
                                 std::deque<T1> packedInts);

public:
  virtual void testSingleValue() ;

  virtual void testSecondaryBlockChange();

  /*
    Check if the structures properly handle the case where
    index * bitsPerValue > Integer.MAX_VALUE

    NOTE: this test allocates 256 MB
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Ignore("See LUCENE-4488") public void testIntOverflow()
  virtual void testIntOverflow();

  virtual void testFill();

  virtual void testPackedIntsNull();

  virtual void testBulkGet();

  virtual void testBulkSet();

  virtual void testCopy();

  virtual void testGrowableWriter();

  virtual void testPagedGrowableWriter();

  virtual void testPagedMutable();

  // memory hole
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Ignore public void testPagedGrowableWriterOverflow()
  virtual void testPagedGrowableWriterOverflow();

  virtual void testSave() ;

  virtual void testEncodeDecode();

private:
  static bool equals(std::deque<int> &ints, std::deque<int64_t> &longs);

public:
  enum class DataType {
    GET_CLASS_NAME(DataType) PACKED,
    DELTA_PACKED,
    MONOTONIC
  };

public:
  virtual void testPackedLongValuesOnZeros();

  virtual void testPackedLongValues();

  virtual void testPackedInputOutput() ;

  virtual void testBlockPackedReaderWriter() ;

  virtual void testMonotonicBlockPackedReaderWriter() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testBlockReaderOverflow() throws
  // java.io.IOException
  virtual void testBlockReaderOverflow() ;

protected:
  std::shared_ptr<TestPackedInts> shared_from_this()
  {
    return std::static_pointer_cast<TestPackedInts>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/packed/
