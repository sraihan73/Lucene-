using namespace std;

#include "PhraseWeight.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Similarity = org::apache::lucene::search::similarities::Similarity;

PhraseWeight::PhraseWeight(shared_ptr<Query> query, const wstring &field,
                           shared_ptr<IndexSearcher> searcher,
                           bool needsScores) 
    : Weight(query), needsScores(needsScores), stats(getStats(searcher)),
      similarity(searcher->getSimilarity(needsScores)), field(field)
{
}

shared_ptr<Scorer>
PhraseWeight::scorer(shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<PhraseMatcher> matcher = getPhraseMatcher(context, false);
  if (matcher == nullptr) {
    return nullptr;
  }
  shared_ptr<Similarity::SimScorer> simScorer =
      similarity->simScorer(stats, context);
  return make_shared<PhraseScorer>(shared_from_this(), matcher, needsScores,
                                   simScorer);
}

shared_ptr<Explanation>
PhraseWeight::explain(shared_ptr<LeafReaderContext> context,
                      int doc) 
{
  shared_ptr<PhraseMatcher> matcher = getPhraseMatcher(context, false);
  if (matcher == nullptr || matcher->approximation->advance(doc) != doc) {
    return Explanation::noMatch(L"no matching terms");
  }
  matcher->reset();
  if (matcher->nextMatch() == false) {
    return Explanation::noMatch(L"no matching phrase");
  }
  shared_ptr<Similarity::SimScorer> simScorer =
      similarity->simScorer(stats, context);
  float freq = matcher->sloppyWeight(simScorer);
  while (matcher->nextMatch()) {
    freq += matcher->sloppyWeight(simScorer);
  }
  shared_ptr<Explanation> freqExplanation =
      Explanation::match(freq, L"phraseFreq=" + to_wstring(freq));
  shared_ptr<Explanation> scoreExplanation =
      simScorer->explain(doc, freqExplanation);
  return Explanation::match(
      scoreExplanation->getValue(),
      L"weight(" + getQuery() + L" in " + to_wstring(doc) + L") [" +
          similarity->getClass().getSimpleName() + L"], result of:",
      scoreExplanation);
}

shared_ptr<Matches> PhraseWeight::matches(shared_ptr<LeafReaderContext> context,
                                          int doc) 
{
  // C++ TODO: The following anonymous inner class could not be converted:
  //      return Matches.forField(field, () => { PhraseMatcher matcher =
  //      getPhraseMatcher(context, true); if(matcher == nullptr ||
  //      matcher.approximation.advance(doc) != doc) { return nullptr; }
  //      matcher.reset(); if(matcher.nextMatch() == false) { return nullptr; }
  //      return new MatchesIterator()
  //    {
  //        bool started = false;
  //        @@Override public bool next() throws IOException
  //        {
  //          if (started == false)
  //          {
  //            return started = true;
  //          }
  //          return matcher.nextMatch();
  //        }
  //
  //        @@Override public int startPosition()
  //        {
  //          return matcher.startPosition();
  //        }
  //
  //        @@Override public int endPosition()
  //        {
  //          return matcher.endPosition();
  //        }
  //
  //        @@Override public int startOffset() throws IOException
  //        {
  //          return matcher.startOffset();
  //        }
  //
  //        @@Override public int endOffset() throws IOException
  //        {
  //          return matcher.endOffset();
  //        }
  //      };
}

bool<missing_class_definition>::isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return true;
}
} // namespace org::apache::lucene::search