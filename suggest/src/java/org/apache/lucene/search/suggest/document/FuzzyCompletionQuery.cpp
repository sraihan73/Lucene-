using namespace std;

#include "FuzzyCompletionQuery.h"

namespace org::apache::lucene::search::suggest::document
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Weight = org::apache::lucene::search::Weight;
using BitsProducer = org::apache::lucene::search::suggest::BitsProducer;
using IntsRef = org::apache::lucene::util::IntsRef;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;
using Automata = org::apache::lucene::util::automaton::Automata;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using FiniteStringsIterator =
    org::apache::lucene::util::automaton::FiniteStringsIterator;
using LevenshteinAutomata =
    org::apache::lucene::util::automaton::LevenshteinAutomata;
using Operations = org::apache::lucene::util::automaton::Operations;
using UTF32ToUTF8 = org::apache::lucene::util::automaton::UTF32ToUTF8;

FuzzyCompletionQuery::FuzzyCompletionQuery(shared_ptr<Analyzer> analyzer,
                                           shared_ptr<Term> term)
    : FuzzyCompletionQuery(analyzer, term, nullptr)
{
}

FuzzyCompletionQuery::FuzzyCompletionQuery(shared_ptr<Analyzer> analyzer,
                                           shared_ptr<Term> term,
                                           shared_ptr<BitsProducer> filter)
    : FuzzyCompletionQuery(analyzer, term, filter, DEFAULT_MAX_EDITS,
                           DEFAULT_TRANSPOSITIONS, DEFAULT_NON_FUZZY_PREFIX,
                           DEFAULT_MIN_FUZZY_LENGTH, DEFAULT_UNICODE_AWARE,
                           Operations::DEFAULT_MAX_DETERMINIZED_STATES)
{
}

FuzzyCompletionQuery::FuzzyCompletionQuery(
    shared_ptr<Analyzer> analyzer, shared_ptr<Term> term,
    shared_ptr<BitsProducer> filter, int maxEdits, bool transpositions,
    int nonFuzzyPrefix, int minFuzzyLength, bool unicodeAware,
    int maxDeterminizedStates)
    : PrefixCompletionQuery(analyzer, term, filter), maxEdits(maxEdits),
      transpositions(transpositions), nonFuzzyPrefix(nonFuzzyPrefix),
      minFuzzyLength(minFuzzyLength), unicodeAware(unicodeAware),
      maxDeterminizedStates(maxDeterminizedStates)
{
}

shared_ptr<Weight>
FuzzyCompletionQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                   bool needsScores,
                                   float boost) 
{
  shared_ptr<Automaton> *const originalAutomata;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (CompletionTokenStream stream =
  // (CompletionTokenStream) analyzer.tokenStream(getField(), getTerm().text()))
  {
    CompletionTokenStream stream =
        std::static_pointer_cast<CompletionTokenStream>(
            analyzer->tokenStream(getField(), getTerm()->text()));
    originalAutomata = stream->toAutomaton(unicodeAware);
  }
  shared_ptr<Set<std::shared_ptr<IntsRef>>> refs =
      unordered_set<std::shared_ptr<IntsRef>>();
  shared_ptr<Automaton> automaton =
      toLevenshteinAutomata(originalAutomata, refs);
  if (unicodeAware) {
    shared_ptr<Automaton> utf8automaton =
        (make_shared<UTF32ToUTF8>())->convert(automaton);
    utf8automaton =
        Operations::determinize(utf8automaton, maxDeterminizedStates);
    automaton = utf8automaton;
  }
  // TODO Accumulating all refs is bad, because the resulting set may be very
  // big.
  // TODO Better iterate over automaton again inside FuzzyCompletionWeight?
  return make_shared<FuzzyCompletionWeight>(shared_from_this(), automaton,
                                            refs);
}

shared_ptr<Automaton> FuzzyCompletionQuery::toLevenshteinAutomata(
    shared_ptr<Automaton> automaton,
    shared_ptr<Set<std::shared_ptr<IntsRef>>> refs)
{
  deque<std::shared_ptr<Automaton>> subs =
      deque<std::shared_ptr<Automaton>>();
  shared_ptr<FiniteStringsIterator> finiteStrings =
      make_shared<FiniteStringsIterator>(automaton);
  for (IntsRef string; (string = finiteStrings->next()) != nullptr;) {
    refs->add(IntsRef::deepCopyOf(string));

    if (string::length <= nonFuzzyPrefix || string::length < minFuzzyLength) {
      subs.push_back(
          Automata::makeString(string::ints, string::offset, string::length));
    } else {
      std::deque<int> ints(string::length - nonFuzzyPrefix);
      System::arraycopy(string::ints, string::offset + nonFuzzyPrefix, ints, 0,
                        ints.size());
      // TODO: maybe add alphaMin to LevenshteinAutomata,
      // and pass 1 instead of 0?  We probably don't want
      // to allow the trailing dedup bytes to be
      // edited... but then 0 byte is "in general" allowed
      // on input (but not in UTF8).
      shared_ptr<LevenshteinAutomata> lev = make_shared<LevenshteinAutomata>(
          ints, unicodeAware ? Character::MAX_CODE_POINT : 255, transpositions);
      subs.push_back(lev->toAutomaton(
          maxEdits, UnicodeUtil::newString(string::ints, string::offset,
                                           nonFuzzyPrefix)));
    }
  }

  if (subs.empty()) {
    // automaton is empty, there is no accepted paths through it
    return Automata::makeEmpty(); // matches nothing
  } else if (subs.size() == 1) {
    // no synonyms or anything: just a single path through the tokenstream
    return subs[0];
  } else {
    // multiple paths: this is really scary! is it slow?
    // maybe we should not do this and throw UOE?
    shared_ptr<Automaton> a = Operations::union_(subs);
    // TODO: we could call toLevenshteinAutomata() before det?
    // this only happens if you have multiple paths anyway (e.g. synonyms)
    return Operations::determinize(a, maxDeterminizedStates);
  }
}

int FuzzyCompletionQuery::getMaxEdits() { return maxEdits; }

bool FuzzyCompletionQuery::isTranspositions() { return transpositions; }

int FuzzyCompletionQuery::getNonFuzzyPrefix() { return nonFuzzyPrefix; }

int FuzzyCompletionQuery::getMinFuzzyLength() { return minFuzzyLength; }

bool FuzzyCompletionQuery::isUnicodeAware() { return unicodeAware; }

int FuzzyCompletionQuery::getMaxDeterminizedStates()
{
  return maxDeterminizedStates;
}

wstring FuzzyCompletionQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  if (getField() != field) {
    buffer->append(getField());
    buffer->append(L":");
  }
  buffer->append(getTerm()->text());
  buffer->append(L'*');
  buffer->append(L'~');
  // C++ TODO: There is no native C++ equivalent to 'toString':
  buffer->append(Integer::toString(maxEdits));
  if (getFilter() != nullptr) {
    buffer->append(L",");
    buffer->append(L"filter");
    // C++ TODO: There is no native C++ equivalent to 'toString':
    buffer->append(getFilter()->toString());
  }
  return buffer->toString();
}

FuzzyCompletionQuery::FuzzyCompletionWeight::FuzzyCompletionWeight(
    shared_ptr<CompletionQuery> query, shared_ptr<Automaton> automaton,
    shared_ptr<Set<std::shared_ptr<IntsRef>>> refs) 
    : CompletionWeight(query, automaton), refs(refs)
{
}

void FuzzyCompletionQuery::FuzzyCompletionWeight::setNextMatch(
    shared_ptr<IntsRef> pathPrefix)
{
  // NOTE: the last letter of the matched prefix for the exact
  // match never makes it through here
  // so an exact match and a match with only a edit at the
  // end is boosted the same
  int maxCount = 0;
  for (auto ref : refs) {
    int minLength = min(ref->length, pathPrefix->length);
    int count = 0;
    for (int i = 0; i < minLength; i++) {
      if (ref->ints[i + ref->offset] ==
          pathPrefix->ints[i + pathPrefix->offset]) {
        count++;
      } else {
        break;
      }
    }
    maxCount = max(maxCount, count);
  }
  currentBoost = maxCount;
}

float FuzzyCompletionQuery::FuzzyCompletionWeight::boost()
{
  return currentBoost;
}
} // namespace org::apache::lucene::search::suggest::document