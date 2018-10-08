#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::util::automaton
{
class Automaton;
}
namespace org::apache::lucene::util
{
class IntsRef;
}
namespace org::apache::lucene::util::automaton
{
class FiniteStringsIterator;
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
namespace org::apache::lucene::util::automaton
{

using namespace org::apache::lucene::util;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

class TestOperations : public LuceneTestCase
{
  GET_CLASS_NAME(TestOperations)
  /** Test string union. */
public:
  virtual void testStringUnion();

private:
  static std::shared_ptr<Automaton>
  naiveUnion(std::deque<std::shared_ptr<BytesRef>> &strings);

  /** Test concatenation with empty language returns empty */
public:
  virtual void testEmptyLanguageConcatenate();

  /** Test optimization to concatenate() with empty std::wstring to an NFA */
  virtual void testEmptySingletonNFAConcatenate();

  virtual void testGetRandomAcceptedString() ;
  /**
   * tests against the original brics implementation.
   */
  virtual void testIsFinite();

  virtual void testIsFiniteEatsStack();

  virtual void testTopoSortEatsStack();

  /**
   * Returns the set of all accepted strings.
   *
   * This method exist just to ease testing.
   * For production code directly use {@link FiniteStringsIterator} instead.
   *
   * @see FiniteStringsIterator
   */
  static std::shared_ptr<Set<std::shared_ptr<IntsRef>>>
  getFiniteStrings(std::shared_ptr<Automaton> a);

  /**
   * Returns the set of accepted strings, up to at most <code>limit</code>
   * strings.
   *
   * This method exist just to ease testing.
   * For production code directly use {@link LimitedFiniteStringsIterator}
   * instead.
   *
   * @see LimitedFiniteStringsIterator
   */
  static std::shared_ptr<Set<std::shared_ptr<IntsRef>>>
  getFiniteStrings(std::shared_ptr<Automaton> a, int limit);

  /**
   * Get all finite strings of an iterator.
   */
private:
  static std::shared_ptr<Set<std::shared_ptr<IntsRef>>>
  getFiniteStrings(std::shared_ptr<FiniteStringsIterator> iterator);

protected:
  std::shared_ptr<TestOperations> shared_from_this()
  {
    return std::static_pointer_cast<TestOperations>(
        LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::automaton
