using namespace std;

#include "TestDocCount.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestDocCount::testSimple() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  int numDocs = atLeast(100);
  for (int i = 0; i < numDocs; i++) {
    iw->addDocument(doc());
  }
  shared_ptr<IndexReader> ir = iw->getReader();
  verifyCount(ir);
  delete ir;
  iw->forceMerge(1);
  ir = iw->getReader();
  verifyCount(ir);
  delete ir;
  delete iw;
  delete dir;
}

shared_ptr<Document> TestDocCount::doc()
{
  shared_ptr<Document> doc = make_shared<Document>();
  int numFields = TestUtil::nextInt(random(), 1, 10);
  for (int i = 0; i < numFields; i++) {
    doc->push_back(newStringField(
        L"" + to_wstring(TestUtil::nextInt(random(), L'a', L'z')),
        L"" + to_wstring(TestUtil::nextInt(random(), L'a', L'z')),
        Field::Store::NO));
  }
  return doc;
}

void TestDocCount::verifyCount(shared_ptr<IndexReader> ir) 
{
  shared_ptr<Fields> fields = MultiFields::getFields(ir);
  for (auto field : fields) {
    shared_ptr<Terms> terms = fields->terms(field);
    if (terms == nullptr) {
      continue;
    }
    int docCount = terms->getDocCount();
    shared_ptr<FixedBitSet> visited = make_shared<FixedBitSet>(ir->maxDoc());
    shared_ptr<TermsEnum> te = terms->begin();
    while (te->next() != nullptr) {
      shared_ptr<PostingsEnum> de =
          TestUtil::docs(random(), te, nullptr, PostingsEnum::NONE);
      while (de->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
        visited->set(de->docID());
      }
    }
    TestUtil::assertEquals(visited->cardinality(), docCount);
  }
}
} // namespace org::apache::lucene::index