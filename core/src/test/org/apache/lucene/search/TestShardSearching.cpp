using namespace std;

#include "TestShardSearching.h"

namespace org::apache::lucene::search
{
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using MultiFields = org::apache::lucene::index::MultiFields;
using MultiReader = org::apache::lucene::index::MultiReader;
using Term = org::apache::lucene::index::Term;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using TestUtil = org::apache::lucene::util::TestUtil;

TestShardSearching::PreviousSearchState::PreviousSearchState(
    shared_ptr<Query> query, shared_ptr<Sort> sort,
    shared_ptr<ScoreDoc> searchAfterLocal,
    shared_ptr<ScoreDoc> searchAfterShard, std::deque<int64_t> &versions,
    int numHitsPaged)
    : searchTimeNanos(System::nanoTime()), versions(versions.clone()),
      searchAfterLocal(searchAfterLocal), searchAfterShard(searchAfterShard),
      sort(sort), query(query), numHitsPaged(numHitsPaged)
{
}

void TestShardSearching::testSimple() 
{
  constexpr int numNodes = TestUtil::nextInt(random(), 1, 10);

  constexpr double runTimeSec = atLeast(3);

  constexpr int minDocsToMakeTerms = TestUtil::nextInt(random(), 5, 20);

  constexpr int maxSearcherAgeSeconds = TestUtil::nextInt(random(), 1, 3);

  if (VERBOSE) {
    wcout << L"TEST: numNodes=" << numNodes << L" runTimeSec=" << runTimeSec
          << L" maxSearcherAgeSeconds=" << maxSearcherAgeSeconds << endl;
  }

  start(numNodes, runTimeSec, maxSearcherAgeSeconds);

  const deque<std::shared_ptr<PreviousSearchState>> priorSearches =
      deque<std::shared_ptr<PreviousSearchState>>();
  deque<std::shared_ptr<BytesRef>> terms;
  while (System::nanoTime() < endTimeNanos) {

    constexpr bool doFollowon =
        priorSearches.size() > 0 && random()->nextInt(7) == 1;

    // Pick a random node; we will run the query on this node:
    constexpr int myNodeID = random()->nextInt(numNodes);

    shared_ptr<NodeState::ShardIndexSearcher> *const localShardSearcher;

    shared_ptr<PreviousSearchState> *const prevSearchState;

    if (doFollowon) {
      // Pretend user issued a followon query:
      prevSearchState = priorSearches[random()->nextInt(priorSearches.size())];

      if (VERBOSE) {
        wcout << L"\nTEST: follow-on query age="
              << ((System::nanoTime() - prevSearchState->searchTimeNanos) /
                  1000000000.0)
              << endl;
      }

      try {
        localShardSearcher =
            nodes[myNodeID]->acquire(prevSearchState->versions);
      } catch (const SearcherExpiredException &see) {
        // Expected, sometimes; in a "real" app we would
        // either forward this error to the user ("too
        // much time has passed; please re-run your
        // search") or sneakily just switch to newest
        // searcher w/o telling them...
        if (VERBOSE) {
          wcout << L"  searcher expired during local shard searcher init: "
                << see << endl;
        }
        // C++ TODO: The Java deque 'remove(Object)' method is not
        // converted:
        priorSearches.remove(prevSearchState);
        continue;
      }
    } else {
      if (VERBOSE) {
        wcout << L"\nTEST: fresh query" << endl;
      }
      // Do fresh query:
      localShardSearcher = nodes[myNodeID]->acquire();
      prevSearchState.reset();
    }

    std::deque<std::shared_ptr<IndexReader>> subs(numNodes);

    shared_ptr<PreviousSearchState> searchState = nullptr;

    try {

      // Mock: now make a single reader (MultiReader) from all node
      // searchers.  In a real shard env you can't do this... we
      // do it to confirm results from the shard searcher
      // are correct:
      int docCount = 0;
      try {
        for (int nodeID = 0; nodeID < numNodes; nodeID++) {
          constexpr int64_t subVersion =
              localShardSearcher->nodeVersions[nodeID];
          shared_ptr<IndexSearcher> *const sub =
              nodes[nodeID]->searchers.acquire(subVersion);
          if (sub == nullptr) {
            nodeID--;
            while (nodeID >= 0) {
              subs[nodeID]->decRef();
              subs[nodeID].reset();
              nodeID--;
            }
            throw make_shared<SearcherExpiredException>(
                L"nodeID=" + to_wstring(nodeID) + L" version=" +
                to_wstring(subVersion));
          }
          subs[nodeID] = sub->getIndexReader();
          docCount += subs[nodeID]->maxDoc();
        }
      } catch (const SearcherExpiredException &see) {
        // Expected
        if (VERBOSE) {
          wcout << L"  searcher expired during mock reader init: " << see
                << endl;
        }
        continue;
      }

      shared_ptr<IndexReader> *const mockReader =
          make_shared<MultiReader>(subs);
      shared_ptr<IndexSearcher> *const mockSearcher =
          make_shared<IndexSearcher>(mockReader);

      shared_ptr<Query> query;
      shared_ptr<Sort> sort;

      if (prevSearchState != nullptr) {
        query = prevSearchState->query;
        sort = prevSearchState->sort;
      } else {
        if (terms.empty() && docCount > minDocsToMakeTerms) {
          // TODO: try to "focus" on high freq terms sometimes too
          // TODO: maybe also periodically reset the terms...?
          shared_ptr<TermsEnum> *const termsEnum =
              MultiFields::getTerms(mockReader, L"body")->begin();
          terms = deque<>();
          while (termsEnum->next() != nullptr) {
            terms.push_back(BytesRef::deepCopyOf(termsEnum->term()));
          }
          if (VERBOSE) {
            wcout << L"TEST: init terms: " << terms.size() << L" terms" << endl;
          }
          if (terms.empty()) {
            terms.clear();
          }
        }

        if (VERBOSE) {
          wcout << L"  maxDoc=" << mockReader->maxDoc() << endl;
        }

        if (terms.size() > 0) {
          if (random()->nextBoolean()) {
            query = make_shared<TermQuery>(make_shared<Term>(
                L"body", terms[random()->nextInt(terms.size())]));
          } else {
            const wstring t =
                terms[random()->nextInt(terms.size())]->utf8ToString();
            const wstring prefix;
            if (t.length() <= 1) {
              prefix = t;
            } else {
              prefix = t.substr(0, TestUtil::nextInt(random(), 1, 2));
            }
            query =
                make_shared<PrefixQuery>(make_shared<Term>(L"body", prefix));
          }

          if (random()->nextBoolean()) {
            sort.reset();
          } else {
            // TODO: sort by more than 1 field
            constexpr int what = random()->nextInt(3);
            if (what == 0) {
              sort = make_shared<Sort>(SortField::FIELD_SCORE);
            } else if (what == 1) {
              // TODO: this sort doesn't merge
              // correctly... it's tricky because you
              // could have > 2.1B docs across all shards:
              // sort = new Sort(SortField.FIELD_DOC);
              sort.reset();
            } else if (what == 2) {
              sort = make_shared<Sort>(std::deque<std::shared_ptr<SortField>>{
                  make_shared<SortField>(L"docid_intDV", SortField::Type::INT,
                                         random()->nextBoolean())});
            } else {
              sort = make_shared<Sort>(std::deque<std::shared_ptr<SortField>>{
                  make_shared<SortField>(L"titleDV", SortField::Type::STRING,
                                         random()->nextBoolean())});
            }
          }
        } else {
          query.reset();
          sort.reset();
        }
      }

      if (query != nullptr) {

        try {
          searchState = assertSame(mockSearcher, localShardSearcher, query,
                                   sort, prevSearchState);
        } catch (const SearcherExpiredException &see) {
          // Expected; in a "real" app we would
          // either forward this error to the user ("too
          // much time has passed; please re-run your
          // search") or sneakily just switch to newest
          // searcher w/o telling them...
          if (VERBOSE) {
            wcout << L"  searcher expired during search: " << see << endl;
            see->printStackTrace(System::out);
          }
          // We can't do this in general: on a very slow
          // computer it's possible the local searcher
          // expires before we can finish our search:
          // assert prevSearchState != null;
          if (prevSearchState != nullptr) {
            // C++ TODO: The Java deque 'remove(Object)' method is not
            // converted:
            priorSearches.remove(prevSearchState);
          }
        }
      }
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      nodes[myNodeID]->release(localShardSearcher);
      for (auto sub : subs) {
        if (sub != nullptr) {
          sub->decRef();
        }
      }
    }

    if (searchState != nullptr && searchState->searchAfterLocal != nullptr &&
        random()->nextInt(5) == 3) {
      priorSearches.push_back(searchState);
      if (priorSearches.size() > 200) {
        Collections::shuffle(priorSearches, random());
        priorSearches.subList(100, priorSearches.size())->clear();
      }
    }
  }

  finish();
}

shared_ptr<PreviousSearchState> TestShardSearching::assertSame(
    shared_ptr<IndexSearcher> mockSearcher,
    shared_ptr<NodeState::ShardIndexSearcher> shardSearcher,
    shared_ptr<Query> q, shared_ptr<Sort> sort,
    shared_ptr<PreviousSearchState> state) 
{

  int numHits = TestUtil::nextInt(random(), 1, 100);
  if (state != nullptr && state->searchAfterLocal == nullptr) {
    // In addition to what we last searched:
    numHits += state->numHitsPaged;
  }

  if (VERBOSE) {
    wcout << L"TEST: query=" << q << L" sort=" << sort << L" numHits="
          << numHits << endl;
    if (state != nullptr) {
      wcout << L"  prev: searchAfterLocal=" << state->searchAfterLocal
            << L" searchAfterShard=" << state->searchAfterShard
            << L" numHitsPaged=" << state->numHitsPaged << endl;
    }
  }

  // Single (mock local) searcher:
  shared_ptr<TopDocs> *const hits;
  if (sort == nullptr) {
    if (state != nullptr && state->searchAfterLocal != nullptr) {
      hits = mockSearcher->searchAfter(state->searchAfterLocal, q, numHits);
    } else {
      hits = mockSearcher->search(q, numHits);
    }
  } else {
    hits = mockSearcher->search(q, numHits, sort);
  }

  // Shard searcher
  shared_ptr<TopDocs> *const shardHits;
  if (sort == nullptr) {
    if (state != nullptr && state->searchAfterShard != nullptr) {
      shardHits =
          shardSearcher->searchAfter(state->searchAfterShard, q, numHits);
    } else {
      shardHits = shardSearcher->search(q, numHits);
    }
  } else {
    shardHits = shardSearcher->search(q, numHits, sort);
  }

  constexpr int numNodes = shardSearcher->nodeVersions.size();
  std::deque<int> base(numNodes);
  const deque<std::shared_ptr<IndexReaderContext>> subs =
      mockSearcher->getTopReaderContext()->children();
  TestUtil::assertEquals(numNodes, subs.size());

  for (int nodeID = 0; nodeID < numNodes; nodeID++) {
    base[nodeID] = subs[nodeID]->docBaseInParent;
  }

  if (VERBOSE) {
    /*
    for(int shardID=0;shardID<shardSearchers.length;shardID++) {
      System.out.println("  shard=" + shardID + " maxDoc=" +
    shardSearchers[shardID].searcher.getIndexReader().maxDoc());
    }
    */
    wcout << L"  single searcher: " << hits->totalHits
          << L" totalHits maxScore=" << hits->getMaxScore() << endl;
    for (int i = 0; i < hits->scoreDocs.size(); i++) {
      shared_ptr<ScoreDoc> *const sd = hits->scoreDocs[i];
      wcout << L"    doc=" << sd->doc << L" score=" << sd->score << endl;
    }
    wcout << L"  shard searcher: " << shardHits->totalHits
          << L" totalHits maxScore=" << shardHits->getMaxScore() << endl;
    for (int i = 0; i < shardHits->scoreDocs.size(); i++) {
      shared_ptr<ScoreDoc> *const sd = shardHits->scoreDocs[i];
      wcout << L"    doc=" << sd->doc << L" (rebased: "
            << (sd->doc << base[sd->shardIndex]) << L") score=" << sd->score
            << L" shard=" << sd->shardIndex << endl;
    }
  }

  int numHitsPaged;
  if (state != nullptr && state->searchAfterLocal != nullptr) {
    numHitsPaged = hits->scoreDocs.size();
    if (state != nullptr) {
      numHitsPaged += state->numHitsPaged;
    }
  } else {
    numHitsPaged = hits->scoreDocs.size();
  }

  constexpr bool moreHits;

  shared_ptr<ScoreDoc> *const bottomHit;
  shared_ptr<ScoreDoc> *const bottomHitShards;

  if (numHitsPaged < hits->totalHits) {
    // More hits to page through
    moreHits = true;
    if (sort == nullptr) {
      bottomHit = hits->scoreDocs[hits->scoreDocs.size() - 1];
      shared_ptr<ScoreDoc> *const sd =
          shardHits->scoreDocs[shardHits->scoreDocs.size() - 1];
      // Must copy because below we rebase:
      bottomHitShards =
          make_shared<ScoreDoc>(sd->doc, sd->score, sd->shardIndex);
      if (VERBOSE) {
        wcout << L"  save bottomHit=" << bottomHit << endl;
      }
    } else {
      bottomHit.reset();
      bottomHitShards.reset();
    }

  } else {
    TestUtil::assertEquals(hits->totalHits, numHitsPaged);
    bottomHit.reset();
    bottomHitShards.reset();
    moreHits = false;
  }

  // Must rebase so assertEquals passes:
  for (int hitID = 0; hitID < shardHits->scoreDocs.size(); hitID++) {
    shared_ptr<ScoreDoc> *const sd = shardHits->scoreDocs[hitID];
    sd->doc += base[sd->shardIndex];
  }

  TestUtil::assertEquals(hits, shardHits);

  if (moreHits) {
    // Return a continuation:
    return make_shared<PreviousSearchState>(q, sort, bottomHit, bottomHitShards,
                                            shardSearcher->nodeVersions,
                                            numHitsPaged);
  } else {
    return nullptr;
  }
}
} // namespace org::apache::lucene::search