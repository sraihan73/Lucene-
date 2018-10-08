#pragma once
#include "stringhelper.h"
#include <cmath>
#include <limits>
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

class TestMathUtil : public LuceneTestCase
{
  GET_CLASS_NAME(TestMathUtil)

public:
  static std::deque<int64_t> PRIMES;

  static int64_t randomLong();

  // slow version used for testing
  static int64_t gcd(int64_t l1, int64_t l2);

  virtual void testGCD();

  // ported test from commons-math
  virtual void testGCD2();

  virtual void testAcoshMethod();

  virtual void testAsinhMethod();

  virtual void testAtanhMethod();

protected:
  std::shared_ptr<TestMathUtil> shared_from_this()
  {
    return std::static_pointer_cast<TestMathUtil>(
        LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
