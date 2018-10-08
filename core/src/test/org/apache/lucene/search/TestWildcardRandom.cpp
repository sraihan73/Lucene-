using namespace std;

#include "TestWildcardRandom.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestWildcardRandom::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMaxBufferedDocs(TestUtil::nextInt(random(), 50, 1000)));

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> field = newStringField(L"field", L"", Field::Store::NO);
  doc->push_back(field);

  shared_ptr<NumberFormat> df = make_shared<DecimalFormat>(
      L"000", make_shared<DecimalFormatSymbols>(Locale::ROOT));
  for (int i = 0; i < 1000; i++) {
    field->setStringValue(df->format(i));
    writer->addDocument(doc);
  }

  reader = writer->getReader();
  searcher = newSearcher(reader);
  delete writer;
  if (VERBOSE) {
    wcout << L"TEST: setUp searcher=" << searcher << endl;
  }
}

wchar_t TestWildcardRandom::N()
{
  return static_cast<wchar_t>(0x30 + random()->nextInt(10));
}

wstring TestWildcardRandom::fillPattern(const wstring &wildcardPattern)
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int i = 0; i < wildcardPattern.length(); i++) {
    switch (wildcardPattern[i]) {
    case L'N':
      sb->append(N());
      break;
    default:
      sb->append(wildcardPattern[i]);
    }
  }
  return sb->toString();
}

void TestWildcardRandom::assertPatternHits(const wstring &pattern,
                                           int numHits) 
{
  // TODO: run with different rewrites
  const wstring filledPattern = fillPattern(pattern);
  if (VERBOSE) {
    wcout << L"TEST: run wildcard pattern=" << pattern << L" filled="
          << filledPattern << endl;
  }
  shared_ptr<Query> wq =
      make_shared<WildcardQuery>(make_shared<Term>(L"field", filledPattern));
  shared_ptr<TopDocs> docs = searcher->search(wq, 25);
  assertEquals(L"Incorrect hits for pattern: " + pattern, numHits,
               docs->totalHits);
}

void TestWildcardRandom::tearDown() 
{
  delete reader;
  delete dir;
  LuceneTestCase::tearDown();
}

void TestWildcardRandom::testWildcards() 
{
  ;
  int num = atLeast(1);
  for (int i = 0; i < num; i++) {
    assertPatternHits(L"NNN", 1);
    assertPatternHits(L"?NN", 10);
    assertPatternHits(L"N?N", 10);
    assertPatternHits(L"NN?", 10);
  }

  for (int i = 0; i < num; i++) {
    assertPatternHits(L"??N", 100);
    assertPatternHits(L"N??", 100);
    assertPatternHits(L"???", 1000);

    assertPatternHits(L"NN*", 10);
    assertPatternHits(L"N*", 100);
    assertPatternHits(L"*", 1000);

    assertPatternHits(L"*NN", 10);
    assertPatternHits(L"*N", 100);

    assertPatternHits(L"N*N", 10);

    // combo of ? and * operators
    assertPatternHits(L"?N*", 100);
    assertPatternHits(L"N?*", 100);

    assertPatternHits(L"*N?", 100);
    assertPatternHits(L"*??", 1000);
    assertPatternHits(L"*?N", 100);
  }
}
} // namespace org::apache::lucene::search