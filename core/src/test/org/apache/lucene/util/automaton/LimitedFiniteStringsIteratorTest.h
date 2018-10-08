#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
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
namespace org::apache::lucene::util::automaton
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.util.automaton.FiniteStringsIteratorTest.getFiniteStrings;

/**
 * Test for {@link FiniteStringsIterator}.
 */
class LimitedFiniteStringsIteratorTest : public LuceneTestCase
{
  GET_CLASS_NAME(LimitedFiniteStringsIteratorTest)
public:
  virtual void testRandomFiniteStrings();

  virtual void testInvalidLimitNegative();

  virtual void testInvalidLimitNull();

  virtual void testSingleton();

  virtual void testLimit();

  virtual void testSize();

protected:
  std::shared_ptr<LimitedFiniteStringsIteratorTest> shared_from_this()
  {
    return std::static_pointer_cast<LimitedFiniteStringsIteratorTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::automaton
