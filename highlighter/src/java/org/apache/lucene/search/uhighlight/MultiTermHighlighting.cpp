using namespace std;

#include "MultiTermHighlighting.h"

namespace org::apache::lucene::search::uhighlight
{
using FunctionScoreQuery =
    org::apache::lucene::queries::function::FunctionScoreQuery;
using AutomatonQuery = org::apache::lucene::search::AutomatonQuery;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using ConstantScoreQuery = org::apache::lucene::search::ConstantScoreQuery;
using DisjunctionMaxQuery = org::apache::lucene::search::DisjunctionMaxQuery;
using FuzzyQuery = org::apache::lucene::search::FuzzyQuery;
using Query = org::apache::lucene::search::Query;
using SpanBoostQuery = org::apache::lucene::search::spans::SpanBoostQuery;
using SpanMultiTermQueryWrapper =
    org::apache::lucene::search::spans::SpanMultiTermQueryWrapper;
using SpanNearQuery = org::apache::lucene::search::spans::SpanNearQuery;
using SpanNotQuery = org::apache::lucene::search::spans::SpanNotQuery;
using SpanOrQuery = org::apache::lucene::search::spans::SpanOrQuery;
using SpanPositionCheckQuery =
    org::apache::lucene::search::spans::SpanPositionCheckQuery;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;
using Automata = org::apache::lucene::util::automaton::Automata;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using ByteRunAutomaton = org::apache::lucene::util::automaton::ByteRunAutomaton;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
using LevenshteinAutomata =
    org::apache::lucene::util::automaton::LevenshteinAutomata;
using Operations = org::apache::lucene::util::automaton::Operations;

MultiTermHighlighting::MultiTermHighlighting() {}

std::deque<std::shared_ptr<CharacterRunAutomaton>>
MultiTermHighlighting::extractAutomata(
    shared_ptr<Query> query, function<bool(const wstring &)> &fieldMatcher,
    bool lookInSpan,
    function<deque<std::shared_ptr<Query>>> &preRewriteFunc)
{
  // TODO Lucene needs a Query visitor API!  LUCENE-3041

  deque<std::shared_ptr<CharacterRunAutomaton>> deque =
      deque<std::shared_ptr<CharacterRunAutomaton>>();
  shared_ptr<deque<std::shared_ptr<Query>>> customSubQueries =
      preRewriteFunc(query);
  if (customSubQueries != nullptr) {
    for (auto sub : customSubQueries) {
      deque.addAll(Arrays::asList(
          extractAutomata(sub, fieldMatcher, lookInSpan, preRewriteFunc)));
    }
  } else if (std::dynamic_pointer_cast<BooleanQuery>(query) != nullptr) {
    for (auto clause : std::static_pointer_cast<BooleanQuery>(query)) {
      if (!clause->isProhibited()) {
        deque.addAll(Arrays::asList(extractAutomata(
            clause->getQuery(), fieldMatcher, lookInSpan, preRewriteFunc)));
      }
    }
  } else if (std::dynamic_pointer_cast<ConstantScoreQuery>(query) != nullptr) {
    deque.addAll(Arrays::asList(extractAutomata(
        (std::static_pointer_cast<ConstantScoreQuery>(query))->getQuery(),
        fieldMatcher, lookInSpan, preRewriteFunc)));
  } else if (std::dynamic_pointer_cast<BoostQuery>(query) != nullptr) {
    deque.addAll(Arrays::asList(extractAutomata(
        (std::static_pointer_cast<BoostQuery>(query))->getQuery(), fieldMatcher,
        lookInSpan, preRewriteFunc)));
  } else if (std::dynamic_pointer_cast<FunctionScoreQuery>(query) != nullptr) {
    deque.addAll(Arrays::asList(
        extractAutomata((std::static_pointer_cast<FunctionScoreQuery>(query))
                            ->getWrappedQuery(),
                        fieldMatcher, lookInSpan, preRewriteFunc)));
  } else if (std::dynamic_pointer_cast<DisjunctionMaxQuery>(query) != nullptr) {
    for (auto sub : (std::static_pointer_cast<DisjunctionMaxQuery>(query))
                        ->getDisjuncts()) {
      deque.addAll(Arrays::asList(
          extractAutomata(sub, fieldMatcher, lookInSpan, preRewriteFunc)));
    }
  } else if (lookInSpan &&
             std::dynamic_pointer_cast<SpanOrQuery>(query) != nullptr) {
    for (auto sub :
         (std::static_pointer_cast<SpanOrQuery>(query))->getClauses()) {
      deque.addAll(Arrays::asList(
          extractAutomata(sub, fieldMatcher, lookInSpan, preRewriteFunc)));
    }
  } else if (lookInSpan &&
             std::dynamic_pointer_cast<SpanNearQuery>(query) != nullptr) {
    for (auto sub :
         (std::static_pointer_cast<SpanNearQuery>(query))->getClauses()) {
      deque.addAll(Arrays::asList(
          extractAutomata(sub, fieldMatcher, lookInSpan, preRewriteFunc)));
    }
  } else if (lookInSpan &&
             std::dynamic_pointer_cast<SpanNotQuery>(query) != nullptr) {
    deque.addAll(Arrays::asList(extractAutomata(
        (std::static_pointer_cast<SpanNotQuery>(query))->getInclude(),
        fieldMatcher, lookInSpan, preRewriteFunc)));
  } else if (lookInSpan && std::dynamic_pointer_cast<SpanPositionCheckQuery>(
                               query) != nullptr) {
    deque.addAll(Arrays::asList(extractAutomata(
        (std::static_pointer_cast<SpanPositionCheckQuery>(query))->getMatch(),
        fieldMatcher, lookInSpan, preRewriteFunc)));
  } else if (lookInSpan &&
             std::dynamic_pointer_cast<SpanBoostQuery>(query) != nullptr) {
    deque.addAll(Arrays::asList(extractAutomata(
        (std::static_pointer_cast<SpanBoostQuery>(query))->getQuery(),
        fieldMatcher, lookInSpan, preRewriteFunc)));
  } else if (lookInSpan && std::dynamic_pointer_cast<SpanMultiTermQueryWrapper>(
                               query) != nullptr) {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE:
    // deque.addAll(java.util.Arrays.asList(extractAutomata(((org.apache.lucene.search.spans.SpanMultiTermQueryWrapper<?>)
    // query).getWrappedQuery(), fieldMatcher, lookInSpan, preRewriteFunc)));
        deque.addAll(Arrays::asList(extractAutomata((std::static_pointer_cast<SpanMultiTermQueryWrapper<?>>(query))->getWrappedQuery(), fieldMatcher, lookInSpan, preRewriteFunc)));
  } else if (std::dynamic_pointer_cast<FuzzyQuery>(query) != nullptr) {
    shared_ptr<FuzzyQuery> *const fq =
        std::static_pointer_cast<FuzzyQuery>(query);
    if (fieldMatcher(fq->getField())) {
      wstring utf16 = fq->getTerm()->text();
      std::deque<int> termText(utf16.codePointCount(0, utf16.length()));
      for (shared_ptr<int cp, > i = 0, j = 0; i < utf16.length();
           i += Character::charCount(cp)) {
        termText[j++] = cp = utf16.codePointAt(i);
      }
      int termLength = termText.size();
      int prefixLength = min(fq->getPrefixLength(), termLength);
      wstring suffix = UnicodeUtil::newString(termText, prefixLength,
                                              termText.size() - prefixLength);
      shared_ptr<LevenshteinAutomata> builder =
          make_shared<LevenshteinAutomata>(suffix, fq->getTranspositions());
      wstring prefix = UnicodeUtil::newString(termText, 0, prefixLength);
      shared_ptr<Automaton> automaton =
          builder->toAutomaton(fq->getMaxEdits(), prefix);
      deque.push_back(
          make_shared<CharacterRunAutomatonAnonymousInnerClass>(automaton, fq));
    }
  } else if (std::dynamic_pointer_cast<AutomatonQuery>(query) != nullptr) {
    shared_ptr<AutomatonQuery> *const aq =
        std::static_pointer_cast<AutomatonQuery>(query);
    if (fieldMatcher(aq->getField())) {

      if (aq->isAutomatonBinary() ==
          false) { // note: is the case for WildcardQuery, RegexpQuery
        deque.push_back(make_shared<CharacterRunAutomatonAnonymousInnerClass2>(
            aq->getAutomaton(), aq));
      } else { // note: is the case for PrefixQuery, TermRangeQuery
        // byte oriented automaton:
        deque.push_back(make_shared<CharacterRunAutomatonAnonymousInnerClass3>(
            Automata::makeEmpty(), aq));
      }
    }
  }
  return deque.toArray(
      std::deque<std::shared_ptr<CharacterRunAutomaton>>(deque.size()));
}

MultiTermHighlighting::CharacterRunAutomatonAnonymousInnerClass::
    CharacterRunAutomatonAnonymousInnerClass(shared_ptr<Automaton> automaton,
                                             shared_ptr<FuzzyQuery> fq)
    : org::apache::lucene::util::automaton::CharacterRunAutomaton(automaton)
{
  this->fq = fq;
}

wstring
MultiTermHighlighting::CharacterRunAutomatonAnonymousInnerClass::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return fq->toString();
}

MultiTermHighlighting::CharacterRunAutomatonAnonymousInnerClass2::
    CharacterRunAutomatonAnonymousInnerClass2(
        shared_ptr<Automaton> getAutomaton, shared_ptr<AutomatonQuery> aq)
    : org::apache::lucene::util::automaton::CharacterRunAutomaton(getAutomaton)
{
  this->aq = aq;
}

wstring
MultiTermHighlighting::CharacterRunAutomatonAnonymousInnerClass2::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return aq->toString();
}

MultiTermHighlighting::CharacterRunAutomatonAnonymousInnerClass3::
    CharacterRunAutomatonAnonymousInnerClass3(shared_ptr<Automaton> makeEmpty,
                                              shared_ptr<AutomatonQuery> aq)
    : org::apache::lucene::util::automaton::CharacterRunAutomaton(makeEmpty)
{
  this->aq = aq;
}

bool MultiTermHighlighting::CharacterRunAutomatonAnonymousInnerClass3::run(
    std::deque<wchar_t> &chars, int offset, int length)
{
  int state = 0;
  constexpr int maxIdx = offset + length;
  for (int i = offset; i < maxIdx; i++) {
    constexpr int code = chars[i];
    int b;
    // UTF16 to UTF8   (inlined logic from UnicodeUtil.UTF16toUTF8 )
    if (code < 0x80) {
      state = byteRunAutomaton::step(state, code);
      if (state == -1) {
        return false;
      }
    } else if (code < 0x800) {
      b = (0xC0 | (code >> 6));
      state = byteRunAutomaton::step(state, b);
      if (state == -1) {
        return false;
      }
      b = (0x80 | (code & 0x3F));
      state = byteRunAutomaton::step(state, b);
      if (state == -1) {
        return false;
      }
    } else {
      // more complex
      std::deque<char> utf8Bytes(4 * (maxIdx - i));
      int utf8Len = UnicodeUtil::UTF16toUTF8(chars, i, maxIdx - i, utf8Bytes);
      for (int utfIdx = 0; utfIdx < utf8Len; utfIdx++) {
        state = byteRunAutomaton::step(state, utf8Bytes[utfIdx] & 0xFF);
        if (state == -1) {
          return false;
        }
      }
      break;
    }
  }
  return byteRunAutomaton::isAccept(state);
}

wstring
MultiTermHighlighting::CharacterRunAutomatonAnonymousInnerClass3::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return aq->toString();
}
} // namespace org::apache::lucene::search::uhighlight