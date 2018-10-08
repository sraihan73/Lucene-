using namespace std;

#include "TestHighFreqTerms.h"

namespace org::apache::lucene::misc
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<org::apache::lucene::index::IndexWriter> TestHighFreqTerms::writer =
    nullptr;
shared_ptr<org::apache::lucene::store::Directory> TestHighFreqTerms::dir =
    nullptr;
shared_ptr<org::apache::lucene::index::IndexReader> TestHighFreqTerms::reader =
    nullptr;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void setUpClass() throws Exception
void TestHighFreqTerms::setUpClass() 
{
  dir = newDirectory();
  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(random(),
                                make_shared<MockAnalyzer>(
                                    random(), MockTokenizer::WHITESPACE, false))
               ->setMaxBufferedDocs(2));
  indexDocs(writer);
  reader = DirectoryReader::open(dir);
  TestUtil::checkIndex(dir);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void tearDownClass() throws
// Exception
void TestHighFreqTerms::tearDownClass() 
{
  delete reader;
  delete dir;
  dir.reset();
  reader.reset();
  writer.reset();
}

void TestHighFreqTerms::testFirstTermHighestDocFreqAllFields() throw(
    runtime_error)
{
  int numTerms = 12;
  wstring field = L"";
  std::deque<std::shared_ptr<TermStats>> terms =
      HighFreqTerms::getHighFreqTerms(
          reader, numTerms, field,
          make_shared<HighFreqTerms::DocFreqComparator>());
  assertEquals(L"Term with highest docfreq is first", 20, terms[0]->docFreq);
}

void TestHighFreqTerms::testFirstTermHighestDocFreq() 
{
  int numTerms = 12;
  wstring field = L"FIELD_1";
  std::deque<std::shared_ptr<TermStats>> terms =
      HighFreqTerms::getHighFreqTerms(
          reader, numTerms, field,
          make_shared<HighFreqTerms::DocFreqComparator>());
  assertEquals(L"Term with highest docfreq is first", 10, terms[0]->docFreq);
}

void TestHighFreqTerms::testOrderedByDocFreqDescending() 
{
  int numTerms = 12;
  wstring field = L"FIELD_1";
  std::deque<std::shared_ptr<TermStats>> terms =
      HighFreqTerms::getHighFreqTerms(
          reader, numTerms, field,
          make_shared<HighFreqTerms::DocFreqComparator>());
  for (int i = 0; i < terms.size(); i++) {
    if (i > 0) {
      assertTrue(L"out of order " + to_wstring(terms[i - 1]->docFreq) +
                     L"should be >= " + to_wstring(terms[i]->docFreq),
                 terms[i - 1]->docFreq >= terms[i]->docFreq);
    }
  }
}

void TestHighFreqTerms::testNumTerms() 
{
  int numTerms = 12;
  wstring field = L"";
  std::deque<std::shared_ptr<TermStats>> terms =
      HighFreqTerms::getHighFreqTerms(
          reader, numTerms, field,
          make_shared<HighFreqTerms::DocFreqComparator>());
  assertEquals(L"length of terms array equals numTerms :" +
                   to_wstring(numTerms),
               numTerms, terms.size());
}

void TestHighFreqTerms::testGetHighFreqTerms() 
{
  int numTerms = 12;
  wstring field = L"FIELD_1";
  std::deque<std::shared_ptr<TermStats>> terms =
      HighFreqTerms::getHighFreqTerms(
          reader, numTerms, field,
          make_shared<HighFreqTerms::DocFreqComparator>());

  for (int i = 0; i < terms.size(); i++) {
    wstring termtext = terms[i]->termtext.utf8ToString();
    // hardcoded highTF or highTFmedDF
    if (termtext.find(L"highTF") != wstring::npos) {
      if (termtext.find(L"medDF") != wstring::npos) {
        assertEquals(L"doc freq is not as expected", 5, terms[i]->docFreq);
      } else {
        assertEquals(L"doc freq is not as expected", 1, terms[i]->docFreq);
      }
    } else {
      int n = stoi(termtext);
      assertEquals(L"doc freq is not as expected", getExpecteddocFreq(n),
                   terms[i]->docFreq);
    }
  }
}

void TestHighFreqTerms::testFirstTermHighestTotalTermFreq() 
{
  int numTerms = 20;
  wstring field = L"";
  std::deque<std::shared_ptr<TermStats>> terms =
      HighFreqTerms::getHighFreqTerms(
          reader, numTerms, field,
          make_shared<HighFreqTerms::TotalTermFreqComparator>());
  assertEquals(L"Term with highest totalTermFreq is first", 200,
               terms[0]->totalTermFreq);
}

void TestHighFreqTerms::testFirstTermHighestTotalTermFreqDifferentField() throw(
    runtime_error)
{
  int numTerms = 20;
  wstring field = L"different_field";
  std::deque<std::shared_ptr<TermStats>> terms =
      HighFreqTerms::getHighFreqTerms(
          reader, numTerms, field,
          make_shared<HighFreqTerms::TotalTermFreqComparator>());
  assertEquals(L"Term with highest totalTermFreq is first" +
                   terms[0]->getTermText(),
               150, terms[0]->totalTermFreq);
}

void TestHighFreqTerms::testOrderedByTermFreqDescending() 
{
  int numTerms = 12;
  wstring field = L"FIELD_1";
  std::deque<std::shared_ptr<TermStats>> terms =
      HighFreqTerms::getHighFreqTerms(
          reader, numTerms, field,
          make_shared<HighFreqTerms::TotalTermFreqComparator>());

  for (int i = 0; i < terms.size(); i++) {
    // check that they are sorted by descending termfreq
    // order
    if (i > 0) {
      assertTrue(L"out of order" + terms[i - 1] + L" > " + terms[i],
                 terms[i - 1]->totalTermFreq >= terms[i]->totalTermFreq);
    }
  }
}

void TestHighFreqTerms::testGetTermFreqOrdered() 
{
  int numTerms = 12;
  wstring field = L"FIELD_1";
  std::deque<std::shared_ptr<TermStats>> terms =
      HighFreqTerms::getHighFreqTerms(
          reader, numTerms, field,
          make_shared<HighFreqTerms::TotalTermFreqComparator>());

  for (int i = 0; i < terms.size(); i++) {
    wstring text = terms[i]->termtext.utf8ToString();
    if (text.find(L"highTF") != wstring::npos) {
      if (text.find(L"medDF") != wstring::npos) {
        assertEquals(L"total term freq is expected", 125,
                     terms[i]->totalTermFreq);
      } else {
        assertEquals(L"total term freq is expected", 200,
                     terms[i]->totalTermFreq);
      }

    } else {
      int n = stoi(text);
      assertEquals(L"doc freq is expected", getExpecteddocFreq(n),
                   terms[i]->docFreq);
      assertEquals(L"total term freq is expected", getExpectedtotalTermFreq(n),
                   terms[i]->totalTermFreq);
    }
  }
}

void TestHighFreqTerms::indexDocs(shared_ptr<IndexWriter> writer) throw(
    runtime_error)
{
  shared_ptr<Random> rnd = random();

  /**
   * Generate 10 documents where term n  has a docFreq of n and a totalTermFreq
   * of n*2 (squared).
   */
  for (int i = 1; i <= 10; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    wstring content = getContent(i);

    doc->push_back(newTextField(rnd, L"FIELD_1", content, Field::Store::YES));
    // add a different field
    doc->push_back(
        newTextField(rnd, L"different_field", L"diff", Field::Store::YES));
    writer->addDocument(doc);
  }

  // add 10 more docs with the term "diff" this will make it have the highest
  // docFreq if we don't ask for the highest freq terms for a specific field.
  for (int i = 1; i <= 10; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newTextField(rnd, L"different_field", L"diff", Field::Store::YES));
    writer->addDocument(doc);
  }
  // add some docs where tf < df so we can see if sorting works
  // highTF low df
  int highTF = 200;
  shared_ptr<Document> doc = make_shared<Document>();
  wstring content = L"";
  for (int i = 0; i < highTF; i++) {
    content += L"highTF ";
  }
  doc->push_back(newTextField(rnd, L"FIELD_1", content, Field::Store::YES));
  writer->addDocument(doc);
  // highTF medium df =5
  int medium_df = 5;
  for (int i = 0; i < medium_df; i++) {
    int tf = 25;
    shared_ptr<Document> newdoc = make_shared<Document>();
    wstring newcontent = L"";
    for (int j = 0; j < tf; j++) {
      newcontent += L"highTFmedDF ";
    }
    newdoc->push_back(
        newTextField(rnd, L"FIELD_1", newcontent, Field::Store::YES));
    writer->addDocument(newdoc);
  }
  // add a doc with high tf in field different_field
  int targetTF = 150;
  doc = make_shared<Document>();
  content = L"";
  for (int i = 0; i < targetTF; i++) {
    content += L"TF150 ";
  }
  doc->push_back(
      newTextField(rnd, L"different_field", content, Field::Store::YES));
  writer->addDocument(doc);
  delete writer;
}

wstring TestHighFreqTerms::getContent(int i)
{
  wstring s = L"";
  for (int j = 10; j >= i; j--) {
    for (int k = 0; k < j; k++) {
      // if j is 3 we return "3 3 3"
      s += to_wstring(j) + L" ";
    }
  }
  return s;
}

int TestHighFreqTerms::getExpectedtotalTermFreq(int i)
{
  return getExpecteddocFreq(i) * i;
}

int TestHighFreqTerms::getExpecteddocFreq(int i) { return i; }
} // namespace org::apache::lucene::misc