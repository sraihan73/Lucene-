#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/automaton/AutomatonProvider.h"

#include  "core/src/java/org/apache/lucene/util/automaton/Automaton.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"

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
using AutomatonProvider =
    org::apache::lucene::util::automaton::AutomatonProvider;

/**
 * A fast regular expression query based on the
 * {@link org.apache.lucene.util.automaton} package.
 * <ul>
 * <li>Comparisons are <a
 * href="http://tusker.org/regex/regex_benchmark.html">fast</a>
 * <li>The term dictionary is enumerated in an intelligent way, to avoid
 * comparisons. See {@link AutomatonQuery} for more details.
 * </ul>
 * <p>
 * The supported syntax is documented in the {@link RegExp} class.
 * Note this might be different than other regular expression implementations.
 * For some alternatives with different syntax, look under the sandbox.
 * </p>
 * <p>
 * Note this query can be slow, as it needs to iterate over many terms. In order
 * to prevent extremely slow RegexpQueries, a Regexp term should not start with
 * the expression <code>.*</code>
 *
 * @see RegExp
 * @lucene.experimental
 */
class RegexpQuery : public AutomatonQuery
{
  GET_CLASS_NAME(RegexpQuery)
  /**
   * A provider that provides no named automata
   */
private:
  static std::shared_ptr<AutomatonProvider> defaultProvider;

private:
  class AutomatonProviderAnonymousInnerClass
      : public std::enable_shared_from_this<
            AutomatonProviderAnonymousInnerClass>,
        public AutomatonProvider
  {
    GET_CLASS_NAME(AutomatonProviderAnonymousInnerClass)
  public:
    AutomatonProviderAnonymousInnerClass();

    std::shared_ptr<Automaton> getAutomaton(const std::wstring &name) override;
  };

  /**
   * Constructs a query for terms matching <code>term</code>.
   * <p>
   * By default, all regular expression features are enabled.
   * </p>
   *
   * @param term regular expression.
   */
public:
  RegexpQuery(std::shared_ptr<Term> term);

  /**
   * Constructs a query for terms matching <code>term</code>.
   *
   * @param term regular expression.
   * @param flags optional RegExp features from {@link RegExp}
   */
  RegexpQuery(std::shared_ptr<Term> term, int flags);

  /**
   * Constructs a query for terms matching <code>term</code>.
   *
   * @param term regular expression.
   * @param flags optional RegExp features from {@link RegExp}
   * @param maxDeterminizedStates maximum number of states that compiling the
   *  automaton for the regexp can result in.  Set higher to allow more complex
   *  queries and lower to prevent memory exhaustion.
   */
  RegexpQuery(std::shared_ptr<Term> term, int flags, int maxDeterminizedStates);

  /**
   * Constructs a query for terms matching <code>term</code>.
   *
   * @param term regular expression.
   * @param flags optional RegExp features from {@link RegExp}
   * @param provider custom AutomatonProvider for named automata
   * @param maxDeterminizedStates maximum number of states that compiling the
   *  automaton for the regexp can result in.  Set higher to allow more complex
   *  queries and lower to prevent memory exhaustion.
   */
  RegexpQuery(std::shared_ptr<Term> term, int flags,
              std::shared_ptr<AutomatonProvider> provider,
              int maxDeterminizedStates);

  /** Returns the regexp of this query wrapped in a Term. */
  virtual std::shared_ptr<Term> getRegexp();

  /** Prints a user-readable version of this query. */
  std::wstring toString(const std::wstring &field) override;

protected:
  std::shared_ptr<RegexpQuery> shared_from_this()
  {
    return std::static_pointer_cast<RegexpQuery>(
        AutomatonQuery::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
