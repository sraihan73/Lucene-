using namespace std;

#include "TestDocValuesRewriteMethod.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using AutomatonTestUtil =
    org::apache::lucene::util::automaton::AutomatonTestUtil;
using RegExp = org::apache::lucene::util::automaton::RegExp;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;

void TestDocValuesRewriteMethod::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
  fieldName = random()->nextBoolean()
                  ? L"field"
                  : L""; // sometimes use an empty string as field name
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(
          make_shared<MockAnalyzer>(random(), MockTokenizer::KEYWORD, false))
          ->setMaxBufferedDocs(TestUtil::nextInt(random(), 50, 1000)));
  deque<wstring> terms = deque<wstring>();
  int num = atLeast(200);
  for (int i = 0; i < num; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(
        newStringField(L"id", Integer::toString(i), Field::Store::NO));
    int numTerms = random()->nextInt(4);
    for (int j = 0; j < numTerms; j++) {
      wstring s = TestUtil::randomUnicodeString(random());
      doc->push_back(newStringField(fieldName, s, Field::Store::NO));
      doc->push_back(make_shared<SortedSetDocValuesField>(
          fieldName, make_shared<BytesRef>(s)));
      terms.push_back(s);
    }
    writer->addDocument(doc);
  }

  if (VERBOSE) {
    // utf16 order
    sort(terms.begin(), terms.end());
    wcout << L"UTF16 order:" << endl;
    for (auto s : terms) {
      wcout << L"  " << UnicodeUtil::toHexString(s) << L" " << s << endl;
    }
  }

  int numDeletions = random()->nextInt(num / 10);
  for (int i = 0; i < numDeletions; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    writer->deleteDocuments(
        make_shared<Term>(L"id", Integer::toString(random()->nextInt(num))));
  }

  reader = writer->getReader();
  searcher1 = newSearcher(reader);
  searcher2 = newSearcher(reader);
  delete writer;
}

void TestDocValuesRewriteMethod::tearDown() 
{
  delete reader;
  delete dir;
  LuceneTestCase::tearDown();
}

void TestDocValuesRewriteMethod::testRegexps() 
{
  int num = atLeast(1000);
  for (int i = 0; i < num; i++) {
    wstring reg = AutomatonTestUtil::randomRegexp(random());
    if (VERBOSE) {
      wcout << L"TEST: regexp=" << reg << endl;
    }
    assertSame(reg);
  }
}

void TestDocValuesRewriteMethod::assertSame(const wstring &regexp) throw(
    IOException)
{
  shared_ptr<RegexpQuery> docValues = make_shared<RegexpQuery>(
      make_shared<Term>(fieldName, regexp), RegExp::NONE);
  docValues->setRewriteMethod(make_shared<DocValuesRewriteMethod>());
  shared_ptr<RegexpQuery> inverted = make_shared<RegexpQuery>(
      make_shared<Term>(fieldName, regexp), RegExp::NONE);

  shared_ptr<TopDocs> invertedDocs = searcher1->search(inverted, 25);
  shared_ptr<TopDocs> docValuesDocs = searcher2->search(docValues, 25);

  CheckHits::checkEqual(inverted, invertedDocs->scoreDocs,
                        docValuesDocs->scoreDocs);
}

void TestDocValuesRewriteMethod::testEquals() 
{
  shared_ptr<RegexpQuery> a1 = make_shared<RegexpQuery>(
      make_shared<Term>(fieldName, L"[aA]"), RegExp::NONE);
  shared_ptr<RegexpQuery> a2 = make_shared<RegexpQuery>(
      make_shared<Term>(fieldName, L"[aA]"), RegExp::NONE);
  shared_ptr<RegexpQuery> b = make_shared<RegexpQuery>(
      make_shared<Term>(fieldName, L"[bB]"), RegExp::NONE);
  TestUtil::assertEquals(a1, a2);
  assertFalse(a1->equals(b));

  a1->setRewriteMethod(make_shared<DocValuesRewriteMethod>());
  a2->setRewriteMethod(make_shared<DocValuesRewriteMethod>());
  b->setRewriteMethod(make_shared<DocValuesRewriteMethod>());
  TestUtil::assertEquals(a1, a2);
  assertFalse(a1->equals(b));
  QueryUtils::check(a1);
}
} // namespace org::apache::lucene::search