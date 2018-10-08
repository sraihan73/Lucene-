using namespace std;

#include "TestParallelTermEnum.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestParallelTermEnum::setUp() 
{
  LuceneTestCase::setUp();
  shared_ptr<Document> doc;
  rd1 = newDirectory();
  shared_ptr<IndexWriter> iw1 = make_shared<IndexWriter>(
      rd1, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));

  doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field1", L"the quick brown fox jumps", Field::Store::YES));
  doc->push_back(
      newTextField(L"field2", L"the quick brown fox jumps", Field::Store::YES));
  iw1->addDocument(doc);

  delete iw1;
  rd2 = newDirectory();
  shared_ptr<IndexWriter> iw2 = make_shared<IndexWriter>(
      rd2, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));

  doc = make_shared<Document>();
  doc->push_back(newTextField(L"field1", L"the fox jumps over the lazy dog",
                              Field::Store::YES));
  doc->push_back(newTextField(L"field3", L"the fox jumps over the lazy dog",
                              Field::Store::YES));
  iw2->addDocument(doc);

  delete iw2;

  this->ir1 = getOnlyLeafReader(DirectoryReader::open(rd1));
  this->ir2 = getOnlyLeafReader(DirectoryReader::open(rd2));
}

void TestParallelTermEnum::tearDown() 
{
  delete ir1;
  delete ir2;
  delete rd1;
  delete rd2;
  LuceneTestCase::tearDown();
}

void TestParallelTermEnum::checkTerms(
    shared_ptr<Terms> terms, deque<wstring> &termsList) 
{
  assertNotNull(terms);
  shared_ptr<TermsEnum> *const te = terms->begin();

  for (wstring t : termsList) {
    shared_ptr<BytesRef> b = te->next();
    assertNotNull(b);
    TestUtil::assertEquals(t, b->utf8ToString());
    shared_ptr<PostingsEnum> td =
        TestUtil::docs(random(), te, nullptr, PostingsEnum::NONE);
    assertTrue(td->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
    TestUtil::assertEquals(0, td->docID());
    TestUtil::assertEquals(td->nextDoc(), DocIdSetIterator::NO_MORE_DOCS);
  }
  assertNull(te->next());
}

void TestParallelTermEnum::test1() 
{
  shared_ptr<ParallelLeafReader> pr = make_shared<ParallelLeafReader>(ir1, ir2);

  TestUtil::assertEquals(3, pr->getFieldInfos()->size());

  checkTerms(pr->terms(L"field1"),
             {L"brown", L"fox", L"jumps", L"quick", L"the"});
  checkTerms(pr->terms(L"field2"),
             {L"brown", L"fox", L"jumps", L"quick", L"the"});
  checkTerms(pr->terms(L"field3"),
             {L"dog", L"fox", L"jumps", L"lazy", L"over", L"the"});
}
} // namespace org::apache::lucene::index