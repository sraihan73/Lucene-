using namespace std;

#include "TestTopDocsMerge.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FloatDocValuesField = org::apache::lucene::document::FloatDocValuesField;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using CompositeReaderContext =
    org::apache::lucene::index::CompositeReaderContext;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

TestTopDocsMerge::ShardSearcher::ShardSearcher(
    shared_ptr<LeafReaderContext> ctx, shared_ptr<IndexReaderContext> parent)
    : IndexSearcher(parent), ctx(Collections::singletonList(ctx))
{
}

void TestTopDocsMerge::ShardSearcher::search(
    shared_ptr<Weight> weight,
    shared_ptr<Collector> collector) 
{
  search(ctx, weight, collector);
}

shared_ptr<TopDocs>
TestTopDocsMerge::ShardSearcher::search(shared_ptr<Weight> weight,
                                        int topN) 
{
  shared_ptr<TopScoreDocCollector> collector =
      TopScoreDocCollector::create(topN);
  search(ctx, weight, collector);
  return collector->topDocs();
}

wstring TestTopDocsMerge::ShardSearcher::toString()
{
  return L"ShardSearcher(" + ctx[0] + L")";
}

void TestTopDocsMerge::testSort_1() ; }

void TestTopDocsMerge::testSort_2() ; }

void TestTopDocsMerge::testInconsistentTopDocsFail()
{
  std::deque<std::shared_ptr<TopDocs>> topDocs = {
      make_shared<TopDocs>(1,
                           std::deque<std::shared_ptr<ScoreDoc>>{
                               make_shared<ScoreDoc>(1, 1.0f)}),
      make_shared<TopDocs>(1, std::deque<std::shared_ptr<ScoreDoc>>{
                                  make_shared<ScoreDoc>(1, 1.0f, -1)})};
  if (random()->nextBoolean()) {
    ArrayUtil::swap(topDocs, 0, 1);
  }
  expectThrows(invalid_argument::typeid,
               [&]() { TopDocs::merge(0, 1, topDocs, false); });
}

void TestTopDocsMerge::testPreAssignedShardIndex()
{
  bool useConstantScore = random()->nextBoolean();
  int numTopDocs = 2 + random()->nextInt(10);
  deque<std::shared_ptr<TopDocs>> topDocs =
      deque<std::shared_ptr<TopDocs>>(numTopDocs);
  unordered_map<int, std::shared_ptr<TopDocs>> shardResultMapping =
      unordered_map<int, std::shared_ptr<TopDocs>>();
  int numHitsTotal = 0;
  for (int i = 0; i < numTopDocs; i++) {
    int numHits = 1 + random()->nextInt(10);
    numHitsTotal += numHits;
    std::deque<std::shared_ptr<ScoreDoc>> scoreDocs(numHits);
    for (int j = 0; j < scoreDocs.size(); j++) {
      float score = useConstantScore ? 1.0f : random()->nextFloat();
      // we set the shard index to index in the deque here but shuffle the entire
      // deque below
      scoreDocs[j] = make_shared<ScoreDoc>((100 * i) + j, score, i);
    }
    topDocs.push_back(make_shared<TopDocs>(numHits, scoreDocs));
    shardResultMapping.emplace(i, topDocs[i]);
  }
  // shuffle the entire thing such that we don't get 1 to 1 mapping of shard
  // index to index in the array
  // -- well likely ;)
  Collections::shuffle(topDocs, random());
  constexpr int from = random()->nextInt(numHitsTotal - 1);
  constexpr int size = 1 + random()->nextInt(numHitsTotal - from);

  // passing false here means TopDocs.merge uses the incoming
  // ScoreDoc.shardIndex that we already set, instead of the position of that
  // TopDocs in the array:
  shared_ptr<TopDocs> merge = TopDocs::merge(
      from, size, topDocs.toArray(std::deque<std::shared_ptr<TopDocs>>(0)),
      false);

  assertTrue(merge->scoreDocs.size() > 0);
  for (auto scoreDoc : merge->scoreDocs) {
    assertTrue(scoreDoc->shardIndex != -1);
    shared_ptr<TopDocs> shardTopDocs = shardResultMapping[scoreDoc->shardIndex];
    assertNotNull(shardTopDocs);
    bool found = false;
    for (auto shardScoreDoc : shardTopDocs->scoreDocs) {
      if (shardScoreDoc == scoreDoc) {
        found = true;
        break;
      }
    }
    assertTrue(found);
  }

  // now ensure merge is stable even if we use our own shard IDs
  Collections::shuffle(topDocs, random());
  shared_ptr<TopDocs> merge2 = TopDocs::merge(
      from, size, topDocs.toArray(std::deque<std::shared_ptr<TopDocs>>(0)),
      false);
  assertArrayEquals(merge->scoreDocs, merge2->scoreDocs);
}

void TestTopDocsMerge::testSort(bool useFrom) 
{

  shared_ptr<IndexReader> reader = nullptr;
  shared_ptr<Directory> dir = nullptr;

  constexpr int numDocs = TEST_NIGHTLY ? atLeast(1000) : atLeast(100);

  const std::deque<wstring> tokens =
      std::deque<wstring>{L"a", L"b", L"c", L"d", L"e"};

  if (VERBOSE) {
    wcout << L"TEST: make index" << endl;
  }

  {
    dir = newDirectory();
    shared_ptr<RandomIndexWriter> *const w =
        make_shared<RandomIndexWriter>(random(), dir);
    // w.setDoRandomForceMerge(false);

    // w.w.getConfig().setMaxBufferedDocs(atLeast(100));

    const std::deque<wstring> content = std::deque<wstring>(atLeast(20));

    for (int contentIDX = 0; contentIDX < content.size(); contentIDX++) {
      shared_ptr<StringBuilder> *const sb = make_shared<StringBuilder>();
      constexpr int numTokens = TestUtil::nextInt(random(), 1, 10);
      for (int tokenIDX = 0; tokenIDX < numTokens; tokenIDX++) {
        sb->append(tokens[random()->nextInt(tokens.size())])->append(L' ');
      }
      content[contentIDX] = sb->toString();
    }

    for (int docIDX = 0; docIDX < numDocs; docIDX++) {
      shared_ptr<Document> *const doc = make_shared<Document>();
      doc->push_back(make_shared<SortedDocValuesField>(
          L"string", make_shared<BytesRef>(
                         TestUtil::randomRealisticUnicodeString(random()))));
      doc->push_back(newTextField(L"text",
                                  content[random()->nextInt(content.size())],
                                  Field::Store::NO));
      doc->push_back(
          make_shared<FloatDocValuesField>(L"float", random()->nextFloat()));
      constexpr int intValue;
      if (random()->nextInt(100) == 17) {
        intValue = numeric_limits<int>::min();
      } else if (random()->nextInt(100) == 17) {
        intValue = numeric_limits<int>::max();
      } else {
        intValue = random()->nextInt();
      }
      doc->push_back(make_shared<NumericDocValuesField>(L"int", intValue));
      if (VERBOSE) {
        wcout << L"  doc=" << doc << endl;
      }
      w->addDocument(doc);
    }

    reader = w->getReader();
    delete w;
  }

  // NOTE: sometimes reader has just one segment, which is
  // important to test
  shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);
  shared_ptr<IndexReaderContext> *const ctx = searcher->getTopReaderContext();

  std::deque<std::shared_ptr<ShardSearcher>> subSearchers;
  const std::deque<int> docStarts;

  if (std::dynamic_pointer_cast<LeafReaderContext>(ctx) != nullptr) {
    subSearchers = std::deque<std::shared_ptr<ShardSearcher>>(1);
    docStarts = std::deque<int>(1);
    subSearchers[0] = make_shared<ShardSearcher>(
        std::static_pointer_cast<LeafReaderContext>(ctx), ctx);
    docStarts[0] = 0;
  } else {
    shared_ptr<CompositeReaderContext> *const compCTX =
        std::static_pointer_cast<CompositeReaderContext>(ctx);
    constexpr int size = compCTX->leaves().size();
    subSearchers = std::deque<std::shared_ptr<ShardSearcher>>(size);
    docStarts = std::deque<int>(size);
    int docBase = 0;
    for (int searcherIDX = 0; searcherIDX < subSearchers.size();
         searcherIDX++) {
      shared_ptr<LeafReaderContext> *const leave =
          compCTX->leaves()[searcherIDX];
      subSearchers[searcherIDX] = make_shared<ShardSearcher>(leave, compCTX);
      docStarts[searcherIDX] = docBase;
      docBase += leave->reader()->maxDoc();
    }
  }

  const deque<std::shared_ptr<SortField>> sortFields =
      deque<std::shared_ptr<SortField>>();
  sortFields.push_back(
      make_shared<SortField>(L"string", SortField::Type::STRING, true));
  sortFields.push_back(
      make_shared<SortField>(L"string", SortField::Type::STRING, false));
  sortFields.push_back(
      make_shared<SortField>(L"int", SortField::Type::INT, true));
  sortFields.push_back(
      make_shared<SortField>(L"int", SortField::Type::INT, false));
  sortFields.push_back(
      make_shared<SortField>(L"float", SortField::Type::FLOAT, true));
  sortFields.push_back(
      make_shared<SortField>(L"float", SortField::Type::FLOAT, false));
  sortFields.push_back(
      make_shared<SortField>(nullptr, SortField::Type::SCORE, true));
  sortFields.push_back(
      make_shared<SortField>(nullptr, SortField::Type::SCORE, false));
  sortFields.push_back(
      make_shared<SortField>(nullptr, SortField::Type::DOC, true));
  sortFields.push_back(
      make_shared<SortField>(nullptr, SortField::Type::DOC, false));

  int numIters = atLeast(300);
  for (int iter = 0; iter < numIters; iter++) {

    // TODO: custom FieldComp...
    shared_ptr<Query> *const query = make_shared<TermQuery>(
        make_shared<Term>(L"text", tokens[random()->nextInt(tokens.size())]));

    shared_ptr<Sort> *const sort;
    if (random()->nextInt(10) == 4) {
      // Sort by score
      sort.reset();
    } else {
      std::deque<std::shared_ptr<SortField>> randomSortFields(
          TestUtil::nextInt(random(), 1, 3));
      for (int sortIDX = 0; sortIDX < randomSortFields.size(); sortIDX++) {
        randomSortFields[sortIDX] =
            sortFields[random()->nextInt(sortFields.size())];
      }
      sort = make_shared<Sort>(randomSortFields);
    }

    constexpr int numHits = TestUtil::nextInt(random(), 1, numDocs + 5);
    // final int numHits = 5;

    if (VERBOSE) {
      wcout << L"TEST: search query=" << query << L" sort=" << sort
            << L" numHits=" << numHits << endl;
    }

    int from = -1;
    int size = -1;
    // First search on whole index:
    shared_ptr<TopDocs> *const topHits;
    if (sort == nullptr) {
      if (useFrom) {
        shared_ptr<TopScoreDocCollector> c =
            TopScoreDocCollector::create(numHits);
        searcher->search(query, c);
        from = TestUtil::nextInt(random(), 0, numHits - 1);
        size = numHits - from;
        shared_ptr<TopDocs> tempTopHits = c->topDocs();
        if (from < tempTopHits->scoreDocs.size()) {
          // Can't use TopDocs#topDocs(start, howMany), since it has different
          // behaviour when start >= hitCount than TopDocs#merge currently has
          std::deque<std::shared_ptr<ScoreDoc>> newScoreDocs(
              min(size, tempTopHits->scoreDocs.size() - from));
          System::arraycopy(tempTopHits->scoreDocs, from, newScoreDocs, 0,
                            newScoreDocs.size());
          tempTopHits->scoreDocs = newScoreDocs;
          topHits = tempTopHits;
        } else {
          topHits = make_shared<TopDocs>(
              tempTopHits->totalHits, std::deque<std::shared_ptr<ScoreDoc>>(0),
              tempTopHits->getMaxScore());
        }
      } else {
        topHits = searcher->search(query, numHits);
      }
    } else {
      shared_ptr<TopFieldCollector> *const c =
          TopFieldCollector::create(sort, numHits, true, true, true, true);
      searcher->search(query, c);
      if (useFrom) {
        from = TestUtil::nextInt(random(), 0, numHits - 1);
        size = numHits - from;
        shared_ptr<TopDocs> tempTopHits = c->topDocs();
        if (from < tempTopHits->scoreDocs.size()) {
          // Can't use TopDocs#topDocs(start, howMany), since it has different
          // behaviour when start >= hitCount than TopDocs#merge currently has
          std::deque<std::shared_ptr<ScoreDoc>> newScoreDocs(
              min(size, tempTopHits->scoreDocs.size() - from));
          System::arraycopy(tempTopHits->scoreDocs, from, newScoreDocs, 0,
                            newScoreDocs.size());
          tempTopHits->scoreDocs = newScoreDocs;
          topHits = tempTopHits;
        } else {
          topHits = make_shared<TopDocs>(
              tempTopHits->totalHits, std::deque<std::shared_ptr<ScoreDoc>>(0),
              tempTopHits->getMaxScore());
        }
      } else {
        topHits = c->topDocs(0, numHits);
      }
    }

    if (VERBOSE) {
      if (useFrom) {
        wcout << L"from=" << from << L" size=" << size << endl;
      }
      wcout << L"  top search: " << topHits->totalHits << L" totalHits; hits="
            << (topHits->scoreDocs.empty() ? L"null"
                                           : topHits->scoreDocs.size()
                                                 << L" maxScore="
                                                 << topHits->getMaxScore())
            << endl;
      if (topHits->scoreDocs.size() > 0) {
        for (int hitIDX = 0; hitIDX < topHits->scoreDocs.size(); hitIDX++) {
          shared_ptr<ScoreDoc> *const sd = topHits->scoreDocs[hitIDX];
          wcout << L"    doc=" << sd->doc << L" score=" << sd->score << endl;
        }
      }
    }

    // ... then all shards:
    shared_ptr<Weight> *const w =
        searcher->createWeight(searcher->rewrite(query), true, 1);

    std::deque<std::shared_ptr<TopDocs>> shardHits;
    if (sort == nullptr) {
      shardHits = std::deque<std::shared_ptr<TopDocs>>(subSearchers.size());
    } else {
      shardHits =
          std::deque<std::shared_ptr<TopFieldDocs>>(subSearchers.size());
    }
    for (int shardIDX = 0; shardIDX < subSearchers.size(); shardIDX++) {
      shared_ptr<TopDocs> *const subHits;
      shared_ptr<ShardSearcher> *const subSearcher = subSearchers[shardIDX];
      if (sort == nullptr) {
        subHits = subSearcher->search(w, numHits);
      } else {
        shared_ptr<TopFieldCollector> *const c =
            TopFieldCollector::create(sort, numHits, true, true, true, true);
        subSearcher->search(w, c);
        subHits = c->topDocs(0, numHits);
      }

      shardHits[shardIDX] = subHits;
      if (VERBOSE) {
        wcout << L"  shard=" << shardIDX << L" " << subHits->totalHits
              << L" totalHits hits="
              << (subHits->scoreDocs.empty() ? L"null"
                                             : subHits->scoreDocs.size())
              << endl;
        if (subHits->scoreDocs.size() > 0) {
          for (auto sd : subHits->scoreDocs) {
            wcout << L"    doc=" << sd->doc << L" score=" << sd->score << endl;
          }
        }
      }
    }

    // Merge:
    shared_ptr<TopDocs> *const mergedHits;
    if (useFrom) {
      if (sort == nullptr) {
        mergedHits = TopDocs::merge(from, size, shardHits, true);
      } else {
        mergedHits = TopDocs::merge(
            sort, from, size,
            static_cast<std::deque<std::shared_ptr<TopFieldDocs>>>(shardHits),
            true);
      }
    } else {
      if (sort == nullptr) {
        mergedHits = TopDocs::merge(numHits, shardHits);
      } else {
        mergedHits = TopDocs::merge(
            sort, numHits,
            static_cast<std::deque<std::shared_ptr<TopFieldDocs>>>(shardHits));
      }
    }

    if (mergedHits->scoreDocs.size() > 0) {
      // Make sure the returned shards are correct:
      for (int hitIDX = 0; hitIDX < mergedHits->scoreDocs.size(); hitIDX++) {
        shared_ptr<ScoreDoc> *const sd = mergedHits->scoreDocs[hitIDX];
        assertEquals(L"doc=" + to_wstring(sd->doc) + L" wrong shard",
                     ReaderUtil::subIndex(sd->doc, docStarts), sd->shardIndex);
      }
    }

    TestUtil::assertEquals(topHits, mergedHits);
  }
  delete reader;
  delete dir;
}
} // namespace org::apache::lucene::search