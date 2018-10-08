#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::automaton
{
class Automaton;
}

namespace org::apache::lucene::util::automaton
{
class CompiledAutomaton;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::index
{
class Terms;
}
namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::util
{
class AttributeSource;
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

using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;

/**
 * A {@link Query} that will match terms against a finite-state machine.
 * <p>
 * This query will match documents that contain terms accepted by a given
 * finite-state machine. The automaton can be constructed with the
 * {@link org.apache.lucene.util.automaton} API. Alternatively, it can be
 * created from a regular expression with {@link RegexpQuery} or from
 * the standard Lucene wildcard syntax with {@link WildcardQuery}.
 * </p>
 * <p>
 * When the query is executed, it will create an equivalent DFA of the
 * finite-state machine, and will enumerate the term dictionary in an
 * intelligent way to reduce the number of comparisons. For example: the regular
 * expression of <code>[dl]og?</code> will make approximately four comparisons:
 * do, dog, lo, and log.
 * </p>
 * @lucene.experimental
 */
class AutomatonQuery : public MultiTermQuery
{
  GET_CLASS_NAME(AutomatonQuery)
  /** the automaton to match index terms against */
protected:
  const std::shared_ptr<Automaton> automaton;
  const std::shared_ptr<CompiledAutomaton> compiled;
  /** term containing the field, and possibly some pattern structure */
  const std::shared_ptr<Term> term;
  const bool automatonIsBinary;

  /**
   * Create a new AutomatonQuery from an {@link Automaton}.
   *
   * @param term Term containing field and possibly some pattern structure. The
   *        term text is ignored.
   * @param automaton Automaton to run, terms that are accepted are considered a
   *        match.
   */
public:
  AutomatonQuery(std::shared_ptr<Term> term,
                 std::shared_ptr<Automaton> automaton);

  /**
   * Create a new AutomatonQuery from an {@link Automaton}.
   *
   * @param term Term containing field and possibly some pattern structure. The
   *        term text is ignored.
   * @param automaton Automaton to run, terms that are accepted are considered a
   *        match.
   * @param maxDeterminizedStates maximum number of states in the resulting
   *   automata.  If the automata would need more than this many states
   *   TooComplextToDeterminizeException is thrown.  Higher number require more
   *   space but can process more complex automata.
   */
  AutomatonQuery(std::shared_ptr<Term> term,
                 std::shared_ptr<Automaton> automaton,
                 int maxDeterminizedStates);

  /**
   * Create a new AutomatonQuery from an {@link Automaton}.
   *
   * @param term Term containing field and possibly some pattern structure. The
   *        term text is ignored.
   * @param automaton Automaton to run, terms that are accepted are considered a
   *        match.
   * @param maxDeterminizedStates maximum number of states in the resulting
   *   automata.  If the automata would need more than this many states
   *   TooComplextToDeterminizeException is thrown.  Higher number require more
   *   space but can process more complex automata.
   * @param isBinary if true, this automaton is already binary and
   *   will not go through the UTF32ToUTF8 conversion
   */
  AutomatonQuery(std::shared_ptr<Term> term,
                 std::shared_ptr<Automaton> automaton,
                 int maxDeterminizedStates, bool isBinary);

protected:
  std::shared_ptr<TermsEnum> getTermsEnum(
      std::shared_ptr<Terms> terms,
      std::shared_ptr<AttributeSource> atts)  override;

public:
  virtual int hashCode();

  bool equals(std::any obj) override;

  std::wstring toString(const std::wstring &field) override;

  /** Returns the automaton used to create this query */
  virtual std::shared_ptr<Automaton> getAutomaton();

  /** Is this a binary (byte) oriented automaton. See the constructor.  */
  virtual bool isAutomatonBinary();

protected:
  std::shared_ptr<AutomatonQuery> shared_from_this()
  {
    return std::static_pointer_cast<AutomatonQuery>(
        MultiTermQuery::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
