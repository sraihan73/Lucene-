using namespace std;

#include "TestSortRescorer.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestSortRescorer::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig()->setSimilarity(make_shared<ClassicSimilarity>()));

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  doc->push_back(newTextField(L"body", L"some contents and more contents",
                              Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"popularity", 5));
  iw->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  doc->push_back(newTextField(
      L"body", L"another document with different contents", Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"popularity", 20));
  iw->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(newStringField(L"id", L"3", Field::Store::YES));
  doc->push_back(newTextField(L"body", L"crappy contents", Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"popularity", 2));
  iw->addDocument(doc);

  reader = iw->getReader();
  searcher = make_shared<IndexSearcher>(reader);
  // TODO: fix this test to not be so flaky and use newSearcher
  searcher->setSimilarity(make_shared<ClassicSimilarity>());
  delete iw;
}

void TestSortRescorer::tearDown() 
{
  reader->close();
  delete dir;
  LuceneTestCase::tearDown();
}

void TestSortRescorer::testBasic() 
{

  // create a sort field and sort by it (reverse order)
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"contents"));
  shared_ptr<IndexReader> r = searcher->getIndexReader();

  // Just first pass query
  shared_ptr<TopDocs> hits = searcher->search(query, 10);
  TestUtil::assertEquals(3, hits->totalHits);
  TestUtil::assertEquals(L"3", r->document(hits->scoreDocs[0]->doc)[L"id"]);
  TestUtil::assertEquals(L"1", r->document(hits->scoreDocs[1]->doc)[L"id"]);
  TestUtil::assertEquals(L"2", r->document(hits->scoreDocs[2]->doc)[L"id"]);

  // Now, rescore:
  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortField>(L"popularity", SortField::Type::INT, true));
  shared_ptr<Rescorer> rescorer = make_shared<SortRescorer>(sort);
  hits = rescorer->rescore(searcher, hits, 10);
  TestUtil::assertEquals(3, hits->totalHits);
  TestUtil::assertEquals(L"2", r->document(hits->scoreDocs[0]->doc)[L"id"]);
  TestUtil::assertEquals(L"1", r->document(hits->scoreDocs[1]->doc)[L"id"]);
  TestUtil::assertEquals(L"3", r->document(hits->scoreDocs[2]->doc)[L"id"]);

  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring expl =
      rescorer
          ->explain(searcher, searcher->explain(query, hits->scoreDocs[0]->doc),
                    hits->scoreDocs[0]->doc)
          ->toString();

  // Confirm the explanation breaks out the individual
  // sort fields:
  assertTrue(expl, expl.find(L"= sort field <int: \"popularity\">! value=20") !=
                       wstring::npos);

  // Confirm the explanation includes first pass details:
  assertTrue(expl.find(L"= first pass score") != wstring::npos);
  assertTrue(expl.find(L"body:contents in") != wstring::npos);
}

void TestSortRescorer::testDoubleValuesSourceSort() 
{
  // create a sort field and sort by it (reverse order)
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"contents"));
  shared_ptr<IndexReader> r = searcher->getIndexReader();

  // Just first pass query
  shared_ptr<TopDocs> hits = searcher->search(query, 10);
  TestUtil::assertEquals(3, hits->totalHits);
  TestUtil::assertEquals(L"3", r->document(hits->scoreDocs[0]->doc)[L"id"]);
  TestUtil::assertEquals(L"1", r->document(hits->scoreDocs[1]->doc)[L"id"]);
  TestUtil::assertEquals(L"2", r->document(hits->scoreDocs[2]->doc)[L"id"]);

  shared_ptr<DoubleValuesSource> source =
      DoubleValuesSource::fromLongField(L"popularity");

  // Now, rescore:
  shared_ptr<Sort> sort = make_shared<Sort>(source->getSortField(true));
  shared_ptr<Rescorer> rescorer = make_shared<SortRescorer>(sort);
  hits = rescorer->rescore(searcher, hits, 10);
  TestUtil::assertEquals(3, hits->totalHits);
  TestUtil::assertEquals(L"2", r->document(hits->scoreDocs[0]->doc)[L"id"]);
  TestUtil::assertEquals(L"1", r->document(hits->scoreDocs[1]->doc)[L"id"]);
  TestUtil::assertEquals(L"3", r->document(hits->scoreDocs[2]->doc)[L"id"]);

  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring expl =
      rescorer
          ->explain(searcher, searcher->explain(query, hits->scoreDocs[0]->doc),
                    hits->scoreDocs[0]->doc)
          ->toString();

  // Confirm the explanation breaks out the individual
  // sort fields:
  assertTrue(expl,
             expl.find(L"= sort field <double(popularity)>! value=20.0") !=
                 wstring::npos);

  // Confirm the explanation includes first pass details:
  assertTrue(expl.find(L"= first pass score") != wstring::npos);
  assertTrue(expl.find(L"body:contents in") != wstring::npos);
}

void TestSortRescorer::testRandom() 
{
  shared_ptr<Directory> dir = newDirectory();
  int numDocs = atLeast(1000);
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);

  const std::deque<int> idToNum = std::deque<int>(numDocs);
  int maxValue = TestUtil::nextInt(random(), 10, 1000000);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(i), Field::Store::YES));
    int numTokens = TestUtil::nextInt(random(), 1, 10);
    shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
    for (int j = 0; j < numTokens; j++) {
      b->append(L"a ");
    }
    doc->push_back(newTextField(L"field", b->toString(), Field::Store::NO));
    idToNum[i] = random()->nextInt(maxValue);
    doc->push_back(make_shared<NumericDocValuesField>(L"num", idToNum[i]));
    w->addDocument(doc);
  }
  shared_ptr<IndexReader> *const r = w->getReader();
  delete w;

  shared_ptr<IndexSearcher> s = newSearcher(r);
  int numHits = TestUtil::nextInt(random(), 1, numDocs);
  bool reverse = random()->nextBoolean();

  shared_ptr<TopDocs> hits = s->search(
      make_shared<TermQuery>(make_shared<Term>(L"field", L"a")), numHits);

  shared_ptr<Rescorer> rescorer = make_shared<SortRescorer>(make_shared<Sort>(
      make_shared<SortField>(L"num", SortField::Type::INT, reverse)));
  shared_ptr<TopDocs> hits2 = rescorer->rescore(s, hits, numHits);

  std::deque<optional<int>> expected(numHits);
  for (int i = 0; i < numHits; i++) {
    expected[i] = hits->scoreDocs[i]->doc;
  }

  constexpr int reverseInt = reverse ? -1 : 1;

  Arrays::sort(expected, make_shared<ComparatorAnonymousInnerClass>(
                             shared_from_this(), idToNum, r, reverseInt));

  bool fail = false;
  for (int i = 0; i < numHits; i++) {
    fail |= expected[i] != hits2->scoreDocs[i]->doc;
  }
  assertFalse(fail);

  delete r;
  delete dir;
}

TestSortRescorer::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass(
    shared_ptr<TestSortRescorer> outerInstance, deque<int> &idToNum,
    shared_ptr<IndexReader> r, int reverseInt)
{
  this->outerInstance = outerInstance;
  this->idToNum = idToNum;
  this->r = r;
  this->reverseInt = reverseInt;
}

int TestSortRescorer::ComparatorAnonymousInnerClass::compare(optional<int> &a,
                                                             optional<int> &b)
{
  try {
    int av = idToNum[static_cast<Integer>(r->document(a)[L"id"])];
    int bv = idToNum[static_cast<Integer>(r->document(b)[L"id"])];
    if (av < bv) {
      return -reverseInt;
    } else if (bv < av) {
      return reverseInt;
    } else {
      // Tie break by docID
      return a - b;
    }
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }
}
} // namespace org::apache::lucene::search