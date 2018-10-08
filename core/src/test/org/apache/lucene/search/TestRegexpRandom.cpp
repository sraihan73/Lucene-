using namespace std;

#include "TestRegexpRandom.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestRegexpRandom::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMaxBufferedDocs(TestUtil::nextInt(random(), 50, 1000)));

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setOmitNorms(true);
  shared_ptr<Field> field = newField(L"field", L"", customType);
  doc->push_back(field);

  shared_ptr<NumberFormat> df = make_shared<DecimalFormat>(
      L"000", make_shared<DecimalFormatSymbols>(Locale::ROOT));
  for (int i = 0; i < 1000; i++) {
    field->setStringValue(df->format(i));
    writer->addDocument(doc);
  }

  reader = writer->getReader();
  delete writer;
  searcher = newSearcher(reader);
}

wchar_t TestRegexpRandom::N()
{
  return static_cast<wchar_t>(0x30 + random()->nextInt(10));
}

wstring TestRegexpRandom::fillPattern(const wstring &wildcardPattern)
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

void TestRegexpRandom::assertPatternHits(const wstring &pattern,
                                         int numHits) 
{
  shared_ptr<Query> wq = make_shared<RegexpQuery>(
      make_shared<Term>(L"field", fillPattern(pattern)));
  shared_ptr<TopDocs> docs = searcher->search(wq, 25);
  assertEquals(L"Incorrect hits for pattern: " + pattern, numHits,
               docs->totalHits);
}

void TestRegexpRandom::tearDown() 
{
  delete reader;
  delete dir;
  LuceneTestCase::tearDown();
}

void TestRegexpRandom::testRegexps() 
{
  int num = atLeast(1);
  for (int i = 0; i < num; i++) {
    assertPatternHits(L"NNN", 1);
    assertPatternHits(L".NN", 10);
    assertPatternHits(L"N.N", 10);
    assertPatternHits(L"NN.", 10);
  }

  for (int i = 0; i < num; i++) {
    assertPatternHits(L".{1,2}N", 100);
    assertPatternHits(L"N.{1,2}", 100);
    assertPatternHits(L".{1,3}", 1000);

    assertPatternHits(L"NN[3-7]", 5);
    assertPatternHits(L"N[2-6][3-7]", 25);
    assertPatternHits(L"[1-5][2-6][3-7]", 125);
    assertPatternHits(L"[0-4][3-7][4-8]", 125);
    assertPatternHits(L"[2-6][0-4]N", 25);
    assertPatternHits(L"[2-6]NN", 5);

    assertPatternHits(L"NN.*", 10);
    assertPatternHits(L"N.*", 100);
    assertPatternHits(L".*", 1000);

    assertPatternHits(L".*NN", 10);
    assertPatternHits(L".*N", 100);

    assertPatternHits(L"N.*N", 10);

    // combo of ? and * operators
    assertPatternHits(L".N.*", 100);
    assertPatternHits(L"N..*", 100);

    assertPatternHits(L".*N.", 100);
    assertPatternHits(L".*..", 1000);
    assertPatternHits(L".*.N", 100);
  }
}
} // namespace org::apache::lucene::search