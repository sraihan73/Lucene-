#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/automaton/Automaton.h"

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
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

class TestLevenshteinAutomata : public LuceneTestCase
{
  GET_CLASS_NAME(TestLevenshteinAutomata)

public:
  virtual void testLev0() ;

  virtual void testLev1() ;

  virtual void testLev2() ;

  // LUCENE-3094
  virtual void testNoWastedStates() ;

  /**
   * Tests all possible characteristic vectors for some n
   * This exhaustively tests the parametric transitions tables.
   */
private:
  void assertCharVectors(int n);

  /**
   * Builds a DFA for some string, and checks all Lev automata
   * up to some maximum distance.
   */
  void assertLev(const std::wstring &s, int maxDistance);

  /**
   * Return an automaton that accepts all 1-character insertions, deletions, and
   * substitutions of s.
   */
  std::shared_ptr<Automaton> naiveLev1(const std::wstring &s);

  /**
   * Return an automaton that accepts all 1-character insertions, deletions,
   * substitutions, and transpositions of s.
   */
  std::shared_ptr<Automaton> naiveLev1T(const std::wstring &s);

  /**
   * Return an automaton that accepts all 1-character insertions of s (inserting
   * one character)
   */
  std::shared_ptr<Automaton> insertionsOf(const std::wstring &s);

  /**
   * Return an automaton that accepts all 1-character deletions of s (deleting
   * one character).
   */
  std::shared_ptr<Automaton> deletionsOf(const std::wstring &s);

  /**
   * Return an automaton that accepts all 1-character substitutions of s
   * (replacing one character)
   */
  std::shared_ptr<Automaton> substitutionsOf(const std::wstring &s);

  /**
   * Return an automaton that accepts all transpositions of s
   * (transposing two adjacent characters)
   */
  std::shared_ptr<Automaton> transpositionsOf(const std::wstring &s);

  void assertBruteForce(const std::wstring &input,
                        std::shared_ptr<Automaton> dfa, int distance);

  void assertBruteForceT(const std::wstring &input,
                         std::shared_ptr<Automaton> dfa, int distance);

  //*****************************
  // Compute Levenshtein distance: see
  // org.apache.commons.lang.StringUtils#getLevenshteinDistance(std::wstring, std::wstring)
  //*****************************
  int getDistance(const std::wstring &target, const std::wstring &other);

  int getTDistance(const std::wstring &target, const std::wstring &other);

protected:
  std::shared_ptr<TestLevenshteinAutomata> shared_from_this()
  {
    return std::static_pointer_cast<TestLevenshteinAutomata>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/automaton/
