#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/Term.h"

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
namespace org::apache::lucene::search
{

using Term = org::apache::lucene::index::Term;
using Automaton = org::apache::lucene::util::automaton::Automaton;

/** Implements the wildcard search query. Supported wildcards are
 * <code>*</code>, which matches any character sequence (including the empty
 * one), and <code>?</code>, which matches any single character. '\' is the
 * escape character. <p> Note this query can be slow, as it needs to iterate
 * over many terms. In order to prevent extremely slow WildcardQueries, a
 * Wildcard term should not start with the wildcard <code>*</code>
 *
 * <p>This query uses the {@link
 * MultiTermQuery#CONSTANT_SCORE_REWRITE}
 * rewrite method.
 *
 * @see AutomatonQuery
 */
class WildcardQuery : public AutomatonQuery
{
  GET_CLASS_NAME(WildcardQuery)
  /** std::wstring equality with support for wildcards */
public:
  static constexpr wchar_t WILDCARD_STRING = L'*';

  /** Char equality with support for wildcards */
  static constexpr wchar_t WILDCARD_CHAR = L'?';

  /** Escape character */
  static constexpr wchar_t WILDCARD_ESCAPE = L'\\';

  /**
   * Constructs a query for terms matching <code>term</code>.
   */
  WildcardQuery(std::shared_ptr<Term> term);

  /**
   * Constructs a query for terms matching <code>term</code>.
   * @param maxDeterminizedStates maximum number of states in the resulting
   *   automata.  If the automata would need more than this many states
   *   TooComplextToDeterminizeException is thrown.  Higher number require more
   *   space but can process more complex automata.
   */
  WildcardQuery(std::shared_ptr<Term> term, int maxDeterminizedStates);

  /**
   * Convert Lucene wildcard syntax into an automaton.
   * @lucene.internal
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("fallthrough") public static
  // org.apache.lucene.util.automaton.Automaton
  // toAutomaton(org.apache.lucene.index.Term wildcardquery)
  static std::shared_ptr<Automaton>
  toAutomaton(std::shared_ptr<Term> wildcardquery);

  /**
   * Returns the pattern term.
   */
  virtual std::shared_ptr<Term> getTerm();

  /** Prints a user-readable version of this query. */
  std::wstring toString(const std::wstring &field) override;

protected:
  std::shared_ptr<WildcardQuery> shared_from_this()
  {
    return std::static_pointer_cast<WildcardQuery>(
        AutomatonQuery::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
