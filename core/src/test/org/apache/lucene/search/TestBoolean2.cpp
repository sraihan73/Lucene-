using namespace std;

#include "TestBoolean2.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::AfterClass;
using org::junit::BeforeClass;
using org::junit::Test;
shared_ptr<IndexSearcher> TestBoolean2::searcher;
shared_ptr<IndexSearcher> TestBoolean2::singleSegmentSearcher;
shared_ptr<IndexSearcher> TestBoolean2::bigSearcher;
shared_ptr<org::apache::lucene::index::IndexReader> TestBoolean2::reader;
shared_ptr<org::apache::lucene::index::IndexReader> TestBoolean2::littleReader;
shared_ptr<org::apache::lucene::index::IndexReader>
    TestBoolean2::singleSegmentReader;
int TestBoolean2::NUM_FILLER_DOCS = 0;
int TestBoolean2::PRE_FILLER_DOCS = 0;
const wstring TestBoolean2::field = L"field";
shared_ptr<org::apache::lucene::store::Directory> TestBoolean2::directory;
shared_ptr<org::apache::lucene::store::Directory>
    TestBoolean2::singleSegmentDirectory;
shared_ptr<org::apache::lucene::store::Directory> TestBoolean2::dir2;
int TestBoolean2::mulFactor = 0;

shared_ptr<Directory>
TestBoolean2::copyOf(shared_ptr<Directory> dir) 
{
  shared_ptr<Directory> copy = newFSDirectory(createTempDir());
  for (auto name : dir->listAll()) {
    if (name.startsWith(L"extra")) {
      continue;
    }
    copy->copyFrom(dir, name, name, IOContext::DEFAULT);
    copy->sync(Collections::singleton(name));
  }
  return copy;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestBoolean2::beforeClass() 
{
  // in some runs, test immediate adjacency of matches - in others, force a full
  // bucket gap between docs
  NUM_FILLER_DOCS = random()->nextBoolean() ? 0 : BooleanScorer::SIZE;
  PRE_FILLER_DOCS = TestUtil::nextInt(random(), 0, (NUM_FILLER_DOCS / 2));
  if (VERBOSE) {
    wcout << L"TEST: NUM_FILLER_DOCS=" << NUM_FILLER_DOCS
          << L" PRE_FILLER_DOCS=" << PRE_FILLER_DOCS << endl;
  }

  if (NUM_FILLER_DOCS * PRE_FILLER_DOCS > 100000) {
    directory = newFSDirectory(createTempDir());
  } else {
    directory = newDirectory();
  }

  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  // randomized codecs are sometimes too costly for this test:
  iwc->setCodec(Codec::forName(L"Lucene70"));
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory, iwc);
  // we'll make a ton of docs, disable store/norms/vectors
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setOmitNorms(true);

  shared_ptr<Document> doc = make_shared<Document>();
  for (int filler = 0; filler < PRE_FILLER_DOCS; filler++) {
    writer->addDocument(doc);
  }
  for (int i = 0; i < docFields.size(); i++) {
    doc->push_back(make_shared<Field>(field, docFields[i], ft));
    writer->addDocument(doc);

    doc = make_shared<Document>();
    for (int filler = 0; filler < NUM_FILLER_DOCS; filler++) {
      writer->addDocument(doc);
    }
  }
  delete writer;
  littleReader = DirectoryReader::open(directory);
  searcher = newSearcher(littleReader);
  // this is intentionally using the baseline sim, because it compares against
  // bigSearcher (which uses a random one)
  searcher->setSimilarity(make_shared<ClassicSimilarity>());

  // make a copy of our index using a single segment
  if (NUM_FILLER_DOCS * PRE_FILLER_DOCS > 100000) {
    singleSegmentDirectory = newFSDirectory(createTempDir());
  } else {
    singleSegmentDirectory = newDirectory();
  }

  // TODO: this test does not need to be doing this crazy stuff. please improve
  // it!
  for (auto fileName : directory->listAll()) {
    if (fileName.startsWith(L"extra")) {
      continue;
    }
    singleSegmentDirectory->copyFrom(directory, fileName, fileName,
                                     IOContext::DEFAULT);
    singleSegmentDirectory->sync(Collections::singleton(fileName));
  }

  iwc = newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  // we need docID order to be preserved:
  // randomized codecs are sometimes too costly for this test:
  iwc->setCodec(Codec::forName(L"Lucene70"));
  iwc->setMergePolicy(newLogMergePolicy());
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.IndexWriter w = new
  // org.apache.lucene.index.IndexWriter(singleSegmentDirectory, iwc))
  {
    org::apache::lucene::index::IndexWriter w =
        org::apache::lucene::index::IndexWriter(singleSegmentDirectory, iwc);
    w->forceMerge(1, true);
  }
  singleSegmentReader = DirectoryReader::open(singleSegmentDirectory);
  singleSegmentSearcher = newSearcher(singleSegmentReader);
  singleSegmentSearcher->setSimilarity(searcher->getSimilarity(true));

  // Make big index
  dir2 = copyOf(directory);

  // First multiply small test index:
  mulFactor = 1;
  int docCount = 0;
  if (VERBOSE) {
    wcout << L"\nTEST: now copy index..." << endl;
  }
  do {
    if (VERBOSE) {
      wcout << L"\nTEST: cycle..." << endl;
    }
    shared_ptr<Directory> *const copy = copyOf(dir2);

    iwc = newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
    // randomized codecs are sometimes too costly for this test:
    iwc->setCodec(Codec::forName(L"Lucene70"));
    RandomIndexWriter w = RandomIndexWriter(random(), dir2, iwc);
    w->addIndexes({copy});
    delete copy;
    docCount = w->maxDoc();
    delete w;
    mulFactor *= 2;
  } while (docCount < 3000 * NUM_FILLER_DOCS);

  iwc = newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setMaxBufferedDocs(TestUtil::nextInt(random(), 50, 1000));
  // randomized codecs are sometimes too costly for this test:
  iwc->setCodec(Codec::forName(L"Lucene70"));
  RandomIndexWriter w = RandomIndexWriter(random(), dir2, iwc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"field2", L"xxx", ft));
  for (int i = 0; i < NUM_EXTRA_DOCS / 2; i++) {
    w->addDocument(doc);
  }
  doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"field2", L"big bad bug", ft));
  for (int i = 0; i < NUM_EXTRA_DOCS / 2; i++) {
    w->addDocument(doc);
  }
  reader = w->getReader();
  bigSearcher = newSearcher(reader);
  delete w;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestBoolean2::afterClass() 
{
  delete reader;
  delete littleReader;
  delete singleSegmentReader;
  delete dir2;
  delete directory;
  delete singleSegmentDirectory;
  singleSegmentSearcher.reset();
  singleSegmentReader.reset();
  singleSegmentDirectory.reset();
  searcher.reset();
  reader.reset();
  littleReader.reset();
  dir2.reset();
  directory.reset();
  bigSearcher.reset();
}

std::deque<wstring> TestBoolean2::docFields = {
    L"w1 w2 w3 w4 w5", L"w1 w3 w2 w3", L"w1 xx w2 yy w3", L"w1 w3 xx w2 yy mm"};

void TestBoolean2::queriesTest(shared_ptr<Query> query,
                               std::deque<int> &expDocNrs) 
{

  // adjust the expected doc numbers according to our filler docs
  if (0 < NUM_FILLER_DOCS) {
    expDocNrs = Arrays::copyOf(expDocNrs, expDocNrs.size());
    for (int i = 0; i < expDocNrs.size(); i++) {
      expDocNrs[i] = PRE_FILLER_DOCS + ((NUM_FILLER_DOCS + 1) * expDocNrs[i]);
    }
  }

  constexpr int topDocsToCheck = atLeast(1000);
  // The asserting searcher will sometimes return the bulk scorer and
  // sometimes return a default impl around the scorer so that we can
  // compare BS1 and BS2
  shared_ptr<TopScoreDocCollector> collector =
      TopScoreDocCollector::create(topDocsToCheck);
  searcher->search(query, collector);
  std::deque<std::shared_ptr<ScoreDoc>> hits1 =
      collector->topDocs()->scoreDocs;
  collector = TopScoreDocCollector::create(topDocsToCheck);
  searcher->search(query, collector);
  std::deque<std::shared_ptr<ScoreDoc>> hits2 =
      collector->topDocs()->scoreDocs;

  CheckHits::checkHitsQuery(query, hits1, hits2, expDocNrs);

  // Since we have no deleted docs, we should also be able to verify identical
  // matches & scores against an single segment copy of our index
  collector = TopScoreDocCollector::create(topDocsToCheck);
  singleSegmentSearcher->search(query, collector);
  hits2 = collector->topDocs()->scoreDocs;
  CheckHits::checkHitsQuery(query, hits1, hits2, expDocNrs);

  // sanity check expected num matches in bigSearcher
  TestUtil::assertEquals(mulFactor * collector->totalHits,
                         bigSearcher->search(query, 1)->totalHits);

  // now check 2 diff scorers from the bigSearcher as well
  collector = TopScoreDocCollector::create(topDocsToCheck);
  bigSearcher->search(query, collector);
  hits1 = collector->topDocs()->scoreDocs;
  collector = TopScoreDocCollector::create(topDocsToCheck);
  bigSearcher->search(query, collector);
  hits2 = collector->topDocs()->scoreDocs;

  // NOTE: just comparing results, not vetting against expDocNrs
  // since we have dups in bigSearcher
  CheckHits::checkEqual(query, hits1, hits2);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testQueries01() throws Exception
void TestBoolean2::testQueries01() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(field, L"w3")),
             BooleanClause::Occur::MUST);
  query->add(make_shared<TermQuery>(make_shared<Term>(field, L"xx")),
             BooleanClause::Occur::MUST);
  std::deque<int> expDocNrs = {2, 3};
  queriesTest(query->build(), expDocNrs);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testQueries02() throws Exception
void TestBoolean2::testQueries02() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(field, L"w3")),
             BooleanClause::Occur::MUST);
  query->add(make_shared<TermQuery>(make_shared<Term>(field, L"xx")),
             BooleanClause::Occur::SHOULD);
  std::deque<int> expDocNrs = {2, 3, 1, 0};
  queriesTest(query->build(), expDocNrs);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testQueries03() throws Exception
void TestBoolean2::testQueries03() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(field, L"w3")),
             BooleanClause::Occur::SHOULD);
  query->add(make_shared<TermQuery>(make_shared<Term>(field, L"xx")),
             BooleanClause::Occur::SHOULD);
  std::deque<int> expDocNrs = {2, 3, 1, 0};
  queriesTest(query->build(), expDocNrs);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testQueries04() throws Exception
void TestBoolean2::testQueries04() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(field, L"w3")),
             BooleanClause::Occur::SHOULD);
  query->add(make_shared<TermQuery>(make_shared<Term>(field, L"xx")),
             BooleanClause::Occur::MUST_NOT);
  std::deque<int> expDocNrs = {1, 0};
  queriesTest(query->build(), expDocNrs);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testQueries05() throws Exception
void TestBoolean2::testQueries05() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(field, L"w3")),
             BooleanClause::Occur::MUST);
  query->add(make_shared<TermQuery>(make_shared<Term>(field, L"xx")),
             BooleanClause::Occur::MUST_NOT);
  std::deque<int> expDocNrs = {1, 0};
  queriesTest(query->build(), expDocNrs);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testQueries06() throws Exception
void TestBoolean2::testQueries06() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(field, L"w3")),
             BooleanClause::Occur::MUST);
  query->add(make_shared<TermQuery>(make_shared<Term>(field, L"xx")),
             BooleanClause::Occur::MUST_NOT);
  query->add(make_shared<TermQuery>(make_shared<Term>(field, L"w5")),
             BooleanClause::Occur::MUST_NOT);
  std::deque<int> expDocNrs = {1};
  queriesTest(query->build(), expDocNrs);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testQueries07() throws Exception
void TestBoolean2::testQueries07() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(field, L"w3")),
             BooleanClause::Occur::MUST_NOT);
  query->add(make_shared<TermQuery>(make_shared<Term>(field, L"xx")),
             BooleanClause::Occur::MUST_NOT);
  query->add(make_shared<TermQuery>(make_shared<Term>(field, L"w5")),
             BooleanClause::Occur::MUST_NOT);
  std::deque<int> expDocNrs = {};
  queriesTest(query->build(), expDocNrs);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testQueries08() throws Exception
void TestBoolean2::testQueries08() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(field, L"w3")),
             BooleanClause::Occur::MUST);
  query->add(make_shared<TermQuery>(make_shared<Term>(field, L"xx")),
             BooleanClause::Occur::SHOULD);
  query->add(make_shared<TermQuery>(make_shared<Term>(field, L"w5")),
             BooleanClause::Occur::MUST_NOT);
  std::deque<int> expDocNrs = {2, 3, 1};
  queriesTest(query->build(), expDocNrs);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testQueries09() throws Exception
void TestBoolean2::testQueries09() 
{
  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(make_shared<TermQuery>(make_shared<Term>(field, L"w3")),
             BooleanClause::Occur::MUST);
  query->add(make_shared<TermQuery>(make_shared<Term>(field, L"xx")),
             BooleanClause::Occur::MUST);
  query->add(make_shared<TermQuery>(make_shared<Term>(field, L"w2")),
             BooleanClause::Occur::MUST);
  query->add(make_shared<TermQuery>(make_shared<Term>(field, L"zz")),
             BooleanClause::Occur::SHOULD);
  std::deque<int> expDocNrs = {2, 3};
  queriesTest(query->build(), expDocNrs);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRandomQueries() throws Exception
void TestBoolean2::testRandomQueries() 
{
  std::deque<wstring> vals = {L"w1", L"w2", L"w3", L"w4",
                               L"w5", L"xx", L"yy", L"zzz"};

  int tot = 0;

  shared_ptr<BooleanQuery> q1;
  try {

    // increase number of iterations for more complete testing
    int num = atLeast(20);
    for (int i = 0; i < num; i++) {
      int level = random()->nextInt(3);
      q1 = randBoolQuery(make_shared<Random>(random()->nextLong()),
                         random()->nextBoolean(), level, field, vals, nullptr)
               ->build();

      // Can't sort by relevance since floating point numbers may not quite
      // match up.
      shared_ptr<Sort> sort = Sort::INDEXORDER;

      QueryUtils::check(random(), q1, searcher); // baseline sim
      try {
        // a little hackish, QueryUtils.check is too costly to do on bigSearcher
        // in this loop.
        searcher->setSimilarity(bigSearcher->getSimilarity(true)); // random sim
        QueryUtils::check(random(), q1, searcher);
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        searcher->setSimilarity(make_shared<ClassicSimilarity>()); // restore
      }

      // check diff (randomized) scorers (from AssertingSearcher) produce the
      // same results
      shared_ptr<TopFieldCollector> collector =
          TopFieldCollector::create(sort, 1000, false, true, true, false);
      searcher->search(q1, collector);
      std::deque<std::shared_ptr<ScoreDoc>> hits1 =
          collector->topDocs()->scoreDocs;
      collector =
          TopFieldCollector::create(sort, 1000, false, true, true, false);
      searcher->search(q1, collector);
      std::deque<std::shared_ptr<ScoreDoc>> hits2 =
          collector->topDocs()->scoreDocs;
      tot += hits2.size();
      CheckHits::checkEqual(q1, hits1, hits2);

      shared_ptr<BooleanQuery::Builder> q3 =
          make_shared<BooleanQuery::Builder>();
      q3->add(q1, BooleanClause::Occur::SHOULD);
      q3->add(make_shared<PrefixQuery>(make_shared<Term>(L"field2", L"b")),
              BooleanClause::Occur::SHOULD);
      shared_ptr<TopDocs> hits4 = bigSearcher->search(q3->build(), 1);
      TestUtil::assertEquals(mulFactor * collector->totalHits +
                                 NUM_EXTRA_DOCS / 2,
                             hits4->totalHits);

      // test diff (randomized) scorers produce the same results on bigSearcher
      // as well
      collector = TopFieldCollector::create(sort, 1000 * mulFactor, false, true,
                                            true, false);
      bigSearcher->search(q1, collector);
      hits1 = collector->topDocs()->scoreDocs;
      collector = TopFieldCollector::create(sort, 1000 * mulFactor, false, true,
                                            true, false);
      bigSearcher->search(q1, collector);
      hits2 = collector->topDocs()->scoreDocs;
      CheckHits::checkEqual(q1, hits1, hits2);
    }

  } catch (const runtime_error &e) {
    // For easier debugging
    wcout << L"failed query: " << q1 << endl;
    throw e;
  }

  // System.out.println("Total hits:"+tot);
}

shared_ptr<BooleanQuery::Builder>
TestBoolean2::randBoolQuery(shared_ptr<Random> rnd, bool allowMust, int level,
                            const wstring &field, std::deque<wstring> &vals,
                            shared_ptr<Callback> cb)
{
  shared_ptr<BooleanQuery::Builder> current =
      make_shared<BooleanQuery::Builder>();
  for (int i = 0; i < rnd->nextInt(vals.size()) + 1; i++) {
    int qType = 0; // term query
    if (level > 0) {
      qType = rnd->nextInt(10);
    }
    shared_ptr<Query> q;
    if (qType < 3) {
      q = make_shared<TermQuery>(
          make_shared<Term>(field, vals[rnd->nextInt(vals.size())]));
    } else if (qType < 4) {
      wstring t1 = vals[rnd->nextInt(vals.size())];
      wstring t2 = vals[rnd->nextInt(vals.size())];
      q = make_shared<PhraseQuery>(
          10, field, t1, t2); // slop increases possibility of matching
    } else if (qType < 7) {
      q = make_shared<WildcardQuery>(make_shared<Term>(field, L"w*"));
    } else {
      q = randBoolQuery(rnd, allowMust, level - 1, field, vals, cb)->build();
    }

    int r = rnd->nextInt(10);
    BooleanClause::Occur occur;
    if (r < 2) {
      occur = BooleanClause::Occur::MUST_NOT;
    } else if (r < 5) {
      if (allowMust) {
        occur = BooleanClause::Occur::MUST;
      } else {
        occur = BooleanClause::Occur::SHOULD;
      }
    } else {
      occur = BooleanClause::Occur::SHOULD;
    }

    current->add(q, occur);
  }
  if (cb != nullptr) {
    cb->postCreate(current);
  }
  return current;
}
} // namespace org::apache::lucene::search