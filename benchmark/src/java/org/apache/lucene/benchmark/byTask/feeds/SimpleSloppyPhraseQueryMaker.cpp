using namespace std;

#include "SimpleSloppyPhraseQueryMaker.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/Term.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/PhraseQuery.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/Query.h"
#include "DocMaker.h"
#include "SingleDocSource.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using Term = org::apache::lucene::index::Term;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Query = org::apache::lucene::search::Query;

std::deque<std::shared_ptr<Query>>
SimpleSloppyPhraseQueryMaker::prepareQueries() 
{
  // extract some 100 words from doc text to an array
  std::deque<wstring> words;
  deque<wstring> w = deque<wstring>();
  shared_ptr<StringTokenizer> st =
      make_shared<StringTokenizer>(SingleDocSource::DOC_TEXT);
  while (st->hasMoreTokens() && w.size() < 100) {
    w.push_back(st->nextToken());
  }
  words = w.toArray(std::deque<wstring>(0));

  // create queries (that would find stuff) with varying slops
  deque<std::shared_ptr<Query>> queries = deque<std::shared_ptr<Query>>();
  for (int slop = 0; slop < 8; slop++) {
    for (int qlen = 2; qlen < 6; qlen++) {
      for (int wd = 0; wd < words.size() - qlen - slop; wd++) {
        // ordered
        int remainedSlop = slop;
        int wind = wd;
        shared_ptr<PhraseQuery::Builder> builder =
            make_shared<PhraseQuery::Builder>();
        for (int i = 0; i < qlen; i++) {
          builder->add(make_shared<Term>(DocMaker::BODY_FIELD, words[wind++]),
                       i);
          if (remainedSlop > 0) {
            remainedSlop--;
            wind++;
          }
        }
        builder->setSlop(slop);
        shared_ptr<PhraseQuery> q = builder->build();
        queries.push_back(q);
        // reversed
        remainedSlop = slop;
        wind = wd + qlen + remainedSlop - 1;
        builder = make_shared<PhraseQuery::Builder>();
        for (int i = 0; i < qlen; i++) {
          builder->add(make_shared<Term>(DocMaker::BODY_FIELD, words[wind--]),
                       i);
          if (remainedSlop > 0) {
            remainedSlop--;
            wind--;
          }
        }
        builder->setSlop(slop + 2 * qlen);
        q = builder->build();
        queries.push_back(q);
      }
    }
  }
  return queries.toArray(std::deque<std::shared_ptr<Query>>(0));
}
} // namespace org::apache::lucene::benchmark::byTask::feeds