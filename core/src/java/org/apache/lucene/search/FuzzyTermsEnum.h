#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class TermsEnum;
}

namespace org::apache::lucene::search
{
class BoostAttribute;
}
namespace org::apache::lucene::search
{
class MaxNonCompetitiveBoostAttribute;
}
namespace org::apache::lucene::search
{
class LevenshteinAutomataAttribute;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util::automaton
{
class CompiledAutomaton;
}
namespace org::apache::lucene::index
{
class Terms;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::util
{
class AttributeSource;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
}
namespace org::apache::lucene::index
{
class TermState;
}
namespace org::apache::lucene::util
{
class AttributeImpl;
}
namespace org::apache::lucene::util
{
class AttributeReflector;
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

using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using TermState = org::apache::lucene::index::TermState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Attribute = org::apache::lucene::util::Attribute;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using BytesRef = org::apache::lucene::util::BytesRef;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;

/** Subclass of TermsEnum for enumerating all terms that are similar
 * to the specified filter term.
 *
 * <p>Term enumerations are always ordered by
 * {@link BytesRef#compareTo}.  Each term in the enumeration is
 * greater than all that precede it.</p>
 */
class FuzzyTermsEnum final : public TermsEnum
{
  GET_CLASS_NAME(FuzzyTermsEnum)

  // NOTE: we can't subclass FilteredTermsEnum here because we need to sometimes
  // change actualEnum:
private:
  std::shared_ptr<TermsEnum> actualEnum;

  // We use this to communicate the score (boost) of the current matched term we
  // are on back to MultiTermQuery.TopTermsBlendedFreqScoringRewrite that is
  // collecting the best (default 50) matched terms:
  const std::shared_ptr<BoostAttribute> boostAtt;

  // MultiTermQuery.TopTermsBlendedFreqScoringRewrite tells us the worst boost
  // still in its queue using this att, which we use to know when we can reduce
  // the automaton from ed=2 to ed=1, or ed=0 if only single top term is
  // collected:
  const std::shared_ptr<MaxNonCompetitiveBoostAttribute> maxBoostAtt;

  // We use this to share the pre-built (once for the query) Levenshtein
  // automata across segments:
  const std::shared_ptr<LevenshteinAutomataAttribute> dfaAtt;

  float bottom = 0;
  std::shared_ptr<BytesRef> bottomTerm;
  std::deque<std::shared_ptr<CompiledAutomaton>> const automata;

  std::shared_ptr<BytesRef> queuedBottom;

public:
  const int termLength;

  // Maximum number of edits we will accept.  This is either 2 or 1 (or,
  // degenerately, 0) passed by the user originally, but as we collect terms, we
  // can lower this (e.g. from 2 to 1) if we detect that the term queue is full,
  // and all collected terms are ed=1:
private:
  int maxEdits = 0;

public:
  const std::shared_ptr<Terms> terms;
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<Term> term_;
  std::deque<int> const termText;
  const int realPrefixLength;

  // True (the default, in FuzzyQuery) if a transposition should count as a
  // single edit:
  const bool transpositions;

  /**
   * Constructor for enumeration of all terms from specified <code>reader</code>
   * which share a prefix of length <code>prefixLength</code> with
   * <code>term</code> and which have at most {@code maxEdits} edits. <p> After
   * calling the constructor the enumeration is already pointing to the first
   * valid term if such a term exists.
   *
   * @param terms Delivers terms.
   * @param atts {@link AttributeSource} created by the rewrite method of {@link
   * MultiTermQuery} thats contains information about competitive boosts during
   * rewrite. It is also used to cache DFAs between segment transitions.
   * @param term Pattern term.
   * @param maxEdits Maximum edit distance.
   * @param prefixLength Length of required common prefix. Default value is 0.
   * @throws IOException if there is a low-level IO error
   */
  FuzzyTermsEnum(std::shared_ptr<Terms> terms,
                 std::shared_ptr<AttributeSource> atts,
                 std::shared_ptr<Term> term, int const maxEdits,
                 int const prefixLength,
                 bool transpositions) ;

  /**
   * return an automata-based enum for matching up to editDistance from
   * lastTerm, if possible
   */
private:
  std::shared_ptr<TermsEnum>
  getAutomatonEnum(int editDistance,
                   std::shared_ptr<BytesRef> lastTerm) ;

  /**
   * fired when the max non-competitive boost has changed. this is the hook to
   * swap in a smarter actualEnum.
   */
  void bottomChanged(std::shared_ptr<BytesRef> lastTerm) ;

public:
  std::shared_ptr<BytesRef> next()  override;

  /** returns true if term is within k edits of the query term */
private:
  bool matches(std::shared_ptr<BytesRef> termIn, int k);

  // proxy all other enum calls to the actual enum
public:
  int docFreq()  override;

  int64_t totalTermFreq()  override;

  std::shared_ptr<PostingsEnum> postings(std::shared_ptr<PostingsEnum> reuse,
                                         int flags)  override;

  void seekExact(std::shared_ptr<BytesRef> term,
                 std::shared_ptr<TermState> state)  override;

  std::shared_ptr<TermState> termState()  override;

  int64_t ord()  override;

  bool seekExact(std::shared_ptr<BytesRef> text)  override;

  SeekStatus
  seekCeil(std::shared_ptr<BytesRef> text)  override;

  void seekExact(int64_t ord)  override;

  std::shared_ptr<BytesRef> term()  override;

  /**
   * reuses compiled automata across different segments,
   * because they are independent of the index
   * @lucene.internal */
public:
  class LevenshteinAutomataAttribute : public Attribute
  {
    GET_CLASS_NAME(LevenshteinAutomataAttribute)
  public:
    virtual std::deque<std::shared_ptr<CompiledAutomaton>> automata() = 0;
    virtual void
    setAutomata(std::deque<std::shared_ptr<CompiledAutomaton>> &automata) = 0;
  };

  /**
   * Stores compiled automata as a deque (indexed by edit distance)
   * @lucene.internal */
public:
  class LevenshteinAutomataAttributeImpl final
      : public AttributeImpl,
        public LevenshteinAutomataAttribute
  {
    GET_CLASS_NAME(LevenshteinAutomataAttributeImpl)
  private:
    // C++ NOTE: Fields cannot have the same name as methods:
    std::deque<std::shared_ptr<CompiledAutomaton>> automata_;

  public:
    std::deque<std::shared_ptr<CompiledAutomaton>> automata() override;

    void setAutomata(
        std::deque<std::shared_ptr<CompiledAutomaton>> &automata) override;

    void clear() override;

    virtual int hashCode();

    bool equals(std::any other) override;

    void copyTo(std::shared_ptr<AttributeImpl> _target) override;

    void reflectWith(AttributeReflector reflector) override;

  protected:
    std::shared_ptr<LevenshteinAutomataAttributeImpl> shared_from_this()
    {
      return std::static_pointer_cast<LevenshteinAutomataAttributeImpl>(
          org.apache.lucene.util.AttributeImpl::shared_from_this());
    }
  };

protected:
  std::shared_ptr<FuzzyTermsEnum> shared_from_this()
  {
    return std::static_pointer_cast<FuzzyTermsEnum>(
        org.apache.lucene.index.TermsEnum::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
