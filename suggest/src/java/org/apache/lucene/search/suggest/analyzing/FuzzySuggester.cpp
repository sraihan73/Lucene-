using namespace std;

#include "FuzzySuggester.h"

namespace org::apache::lucene::search::suggest::analyzing
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenStreamToAutomaton =
    org::apache::lucene::analysis::TokenStreamToAutomaton;
using TermToBytesRefAttribute =
    org::apache::lucene::analysis::tokenattributes::TermToBytesRefAttribute;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
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
using FST = org::apache::lucene::util::fst::FST;
using org::apache::lucene::util::fst::PairOutputs::Pair;
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

FuzzySuggester::FuzzySuggester(shared_ptr<Directory> tempDir,
                               const wstring &tempFileNamePrefix,
                               shared_ptr<Analyzer> analyzer)
    : FuzzySuggester(tempDir, tempFileNamePrefix, analyzer, analyzer)
{
}

FuzzySuggester::FuzzySuggester(shared_ptr<Directory> tempDir,
                               const wstring &tempFileNamePrefix,
                               shared_ptr<Analyzer> indexAnalyzer,
                               shared_ptr<Analyzer> queryAnalyzer)
    : FuzzySuggester(tempDir, tempFileNamePrefix, indexAnalyzer, queryAnalyzer,
                     EXACT_FIRST | PRESERVE_SEP, 256, -1, true,
                     DEFAULT_MAX_EDITS, DEFAULT_TRANSPOSITIONS,
                     DEFAULT_NON_FUZZY_PREFIX, DEFAULT_MIN_FUZZY_LENGTH,
                     DEFAULT_UNICODE_AWARE)
{
}

FuzzySuggester::FuzzySuggester(
    shared_ptr<Directory> tempDir, const wstring &tempFileNamePrefix,
    shared_ptr<Analyzer> indexAnalyzer, shared_ptr<Analyzer> queryAnalyzer,
    int options, int maxSurfaceFormsPerAnalyzedForm, int maxGraphExpansions,
    bool preservePositionIncrements, int maxEdits, bool transpositions,
    int nonFuzzyPrefix, int minFuzzyLength, bool unicodeAware)
    : AnalyzingSuggester(tempDir, tempFileNamePrefix, indexAnalyzer,
                         queryAnalyzer, options, maxSurfaceFormsPerAnalyzedForm,
                         maxGraphExpansions, preservePositionIncrements),
      maxEdits(maxEdits), transpositions(transpositions),
      nonFuzzyPrefix(nonFuzzyPrefix), minFuzzyLength(minFuzzyLength),
      unicodeAware(unicodeAware)
{
  if (maxEdits < 0 ||
      maxEdits > LevenshteinAutomata::MAXIMUM_SUPPORTED_DISTANCE) {
    throw invalid_argument(
        L"maxEdits must be between 0 and " +
        to_wstring(LevenshteinAutomata::MAXIMUM_SUPPORTED_DISTANCE));
  }
  if (nonFuzzyPrefix < 0) {
    throw invalid_argument(L"nonFuzzyPrefix must not be >= 0 (got " +
                           to_wstring(nonFuzzyPrefix) + L")");
  }
  if (minFuzzyLength < 0) {
    throw invalid_argument(L"minFuzzyLength must not be >= 0 (got " +
                           to_wstring(minFuzzyLength) + L")");
  }
}

deque<FSTUtil::Path<Pair<int64_t, std::shared_ptr<BytesRef>>>>
FuzzySuggester::getFullPrefixPaths(
    deque<FSTUtil::Path<Pair<int64_t, std::shared_ptr<BytesRef>>>>
        &prefixPaths,
    shared_ptr<Automaton> lookupAutomaton,
    shared_ptr<FST<Pair<int64_t, std::shared_ptr<BytesRef>>>>
        fst) 
{

  // TODO: right now there's no penalty for fuzzy/edits,
  // ie a completion whose prefix matched exactly what the
  // user typed gets no boost over completions that
  // required an edit, which get no boost over completions
  // requiring two edits.  I suspect a multiplicative
  // factor is appropriate (eg, say a fuzzy match must be at
  // least 2X better weight than the non-fuzzy match to
  // "compete") ... in which case I think the wFST needs
  // to be log weights or something ...

  shared_ptr<Automaton> levA =
      convertAutomaton(toLevenshteinAutomata(lookupAutomaton));
  /*
    Writer w = new OutputStreamWriter(new FileOutputStream("out.dot"),
    StandardCharsets.UTF_8); w.write(levA.toDot()); w.close();
    System.out.println("Wrote LevA to out.dot");
  */
  return FSTUtil::intersectPrefixPaths(levA, fst);
}

shared_ptr<Automaton> FuzzySuggester::convertAutomaton(shared_ptr<Automaton> a)
{
  if (unicodeAware) {
    shared_ptr<Automaton> utf8automaton =
        (make_shared<UTF32ToUTF8>())->convert(a);
    utf8automaton = Operations::determinize(
        utf8automaton, Operations::DEFAULT_MAX_DETERMINIZED_STATES);
    return utf8automaton;
  } else {
    return a;
  }
}

shared_ptr<TokenStreamToAutomaton> FuzzySuggester::getTokenStreamToAutomaton()
{
  shared_ptr<TokenStreamToAutomaton> *const tsta =
      AnalyzingSuggester::getTokenStreamToAutomaton();
  tsta->setUnicodeArcs(unicodeAware);
  return tsta;
}

shared_ptr<Automaton>
FuzzySuggester::toLevenshteinAutomata(shared_ptr<Automaton> automaton)
{
  deque<std::shared_ptr<Automaton>> subs =
      deque<std::shared_ptr<Automaton>>();
  shared_ptr<FiniteStringsIterator> finiteStrings =
      make_shared<FiniteStringsIterator>(automaton);
  for (IntsRef string; (string = finiteStrings->next()) != nullptr;) {
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
    return Operations::determinize(a,
                                   Operations::DEFAULT_MAX_DETERMINIZED_STATES);
  }
}
} // namespace org::apache::lucene::search::suggest::analyzing