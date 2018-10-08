using namespace std;

#include "TestFlex.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestFlex::testNonFlex() 
{
  shared_ptr<Directory> d = newDirectory();

  constexpr int DOC_COUNT = 177;

  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      d, (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
             ->setMaxBufferedDocs(7)
             ->setMergePolicy(newLogMergePolicy()));

  for (int iter = 0; iter < 2; iter++) {
    if (iter == 0) {
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(
          newTextField(L"field1", L"this is field1", Field::Store::NO));
      doc->push_back(
          newTextField(L"field2", L"this is field2", Field::Store::NO));
      doc->push_back(newTextField(L"field3", L"aaa", Field::Store::NO));
      doc->push_back(newTextField(L"field4", L"bbb", Field::Store::NO));
      for (int i = 0; i < DOC_COUNT; i++) {
        w->addDocument(doc);
      }
    } else {
      w->forceMerge(1);
    }

    shared_ptr<IndexReader> r = w->getReader();

    shared_ptr<TermsEnum> terms = MultiFields::getTerms(r, L"field3")->begin();
    TestUtil::assertEquals(TermsEnum::SeekStatus::END,
                           terms->seekCeil(make_shared<BytesRef>(L"abc")));
    delete r;
  }

  delete w;
  delete d;
}

void TestFlex::testTermOrd() 
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      d, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
             ->setCodec(TestUtil::alwaysPostingsFormat(
                 TestUtil::getDefaultPostingsFormat())));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"f", L"a b c", Field::Store::NO));
  w->addDocument(doc);
  w->forceMerge(1);
  shared_ptr<DirectoryReader> r = w->getReader();
  shared_ptr<TermsEnum> terms = getOnlyLeafReader(r)->terms(L"f")->begin();
  assertTrue(terms->next() != nullptr);
  try {
    TestUtil::assertEquals(0, terms->ord());
  } catch (const UnsupportedOperationException &uoe) {
    // ok -- codec is not required to support this op
  }
  r->close();
  delete w;
  delete d;
}
} // namespace org::apache::lucene::index