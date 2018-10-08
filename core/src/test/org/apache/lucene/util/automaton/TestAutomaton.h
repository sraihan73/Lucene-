#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/automaton/Automaton.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/util/IntsRef.h"

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

using BytesRef = org::apache::lucene::util::BytesRef;
using IntsRef = org::apache::lucene::util::IntsRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

class TestAutomaton : public LuceneTestCase
{
  GET_CLASS_NAME(TestAutomaton)

public:
  virtual void testBasic() ;

  virtual void testReduceBasic() ;

  virtual void testSameLanguage() ;

  virtual void testCommonPrefix() ;

  virtual void testConcatenate1() ;

  virtual void testConcatenate2() ;

  virtual void testUnion1() ;

  virtual void testUnion2() ;

  virtual void testMinimizeSimple() ;

  virtual void testMinimize2() ;

  virtual void testReverse() ;

  virtual void testOptional() ;

  virtual void testRepeatAny() ;

  virtual void testRepeatMin() ;

  virtual void testRepeatMinMax1() ;

  virtual void testRepeatMinMax2() ;

  virtual void testComplement() ;

  virtual void testInterval() ;

  virtual void testCommonSuffix() ;

  virtual void testReverseRandom1() ;

  virtual void testReverseRandom2() ;

  virtual void testAnyStringEmptyString() ;

  virtual void testBasicIsEmpty() ;

  virtual void testRemoveDeadTransitionsEmpty() ;

  virtual void testInvalidAddTransition() ;

  virtual void testBuilderRandom() ;

  virtual void testIsTotal() ;

  virtual void testMinimizeEmpty() ;

  virtual void testMinus() ;

  virtual void testOneInterval() ;

  virtual void testAnotherInterval() ;

  virtual void testIntervalRandom() ;

private:
  void assertMatches(std::shared_ptr<Automaton> a,
                     std::deque<std::wstring> &strings);

public:
  virtual void testConcatenatePreservesDet() ;

  virtual void testRemoveDeadStates() ;

  virtual void testRemoveDeadStatesEmpty1() ;

  virtual void testRemoveDeadStatesEmpty2() ;

  virtual void testRemoveDeadStatesEmpty3() ;

  virtual void testConcatEmpty() ;

  virtual void testSeemsNonEmptyButIsNot1() ;

  virtual void testSeemsNonEmptyButIsNot2() ;

  virtual void testSameLanguage1() ;

private:
  std::shared_ptr<Automaton> randomNoOp(std::shared_ptr<Automaton> a);

  std::shared_ptr<Automaton>
  unionTerms(std::shared_ptr<std::deque<std::shared_ptr<BytesRef>>> terms);

  std::wstring getRandomString();

public:
  virtual void testRandomFinite() ;

  /** Runs topo sort, verifies transitions then only "go forwards", and
   *  builds and returns new automaton with those remapped toposorted states. */
private:
  std::shared_ptr<Automaton> verifyTopoSort(std::shared_ptr<Automaton> a);

  void assertSame(std::shared_ptr<std::deque<std::shared_ptr<BytesRef>>> terms,
                  std::shared_ptr<Automaton> a);

  bool accepts(std::shared_ptr<Automaton> a, std::shared_ptr<BytesRef> b);

  std::shared_ptr<Automaton>
  makeBinaryInterval(std::shared_ptr<BytesRef> minTerm, bool minInclusive,
                     std::shared_ptr<BytesRef> maxTerm, bool maxInclusive);

public:
  virtual void
  testMakeBinaryIntervalFiniteCasesBasic() ;

  virtual void
  testMakeBinaryIntervalFiniteCasesRandom() ;

  virtual void testMakeBinaryIntervalRandom() ;

private:
  static std::shared_ptr<IntsRef> intsRef(const std::wstring &s);

public:
  virtual void testMakeBinaryIntervalBasic() ;

  virtual void testMakeBinaryIntervalEqual() ;

  virtual void testMakeBinaryIntervalCommonPrefix() ;

  virtual void testMakeBinaryIntervalOpenMax() ;

  virtual void testMakeBinaryIntervalOpenMin() ;

  virtual void testMakeBinaryIntervalOpenBoth() ;

  virtual void testAcceptAllEmptyStringMin() ;

private:
  static std::shared_ptr<IntsRef> toIntsRef(const std::wstring &s);

public:
  virtual void testGetSingleton();

  virtual void testGetSingletonEmptyString();

  virtual void testGetSingletonNothing();

  virtual void testGetSingletonTwo();

protected:
  std::shared_ptr<TestAutomaton> shared_from_this()
  {
    return std::static_pointer_cast<TestAutomaton>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/automaton/
