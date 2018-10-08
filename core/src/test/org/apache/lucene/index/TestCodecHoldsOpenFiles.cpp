using namespace std;

#include "TestCodecHoldsOpenFiles.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using IntPoint = org::apache::lucene::document::IntPoint;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using TextField = org::apache::lucene::document::TextField;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestCodecHoldsOpenFiles::test() 
{
  shared_ptr<BaseDirectoryWrapper> d = newDirectory();
  d->setCheckIndexOnClose(false);
  // we nuke files, but verify the reader still works
  shared_ptr<RandomIndexWriter> w = make_shared<RandomIndexWriter>(random(), d);
  int numDocs = atLeast(100);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newField(L"foo", L"bar", TextField::TYPE_NOT_STORED));
    doc->push_back(make_shared<IntPoint>(L"doc", i));
    doc->push_back(make_shared<IntPoint>(L"doc2d", i, i));
    doc->push_back(make_shared<NumericDocValuesField>(L"dv", i));
    w->addDocument(doc);
  }

  shared_ptr<IndexReader> r = w->getReader();
  w->commit();
  delete w;

  for (auto name : d->listAll()) {
    d->deleteFile(name);
  }

  for (auto cxt : r->leaves()) {
    TestUtil::checkReader(cxt->reader());
  }

  delete r;
  delete d;
}
} // namespace org::apache::lucene::index