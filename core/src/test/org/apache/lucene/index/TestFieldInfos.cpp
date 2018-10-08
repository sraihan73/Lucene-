using namespace std;

#include "TestFieldInfos.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StringField = org::apache::lucene::document::StringField;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestFieldInfos::testFieldInfos() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMergePolicy(NoMergePolicy::INSTANCE));

  shared_ptr<Document> d1 = make_shared<Document>();
  for (int i = 0; i < 15; i++) {
    d1->push_back(make_shared<StringField>(
        L"f" + to_wstring(i), L"v" + to_wstring(i), Field::Store::YES));
  }
  writer->addDocument(d1);
  writer->commit();

  shared_ptr<Document> d2 = make_shared<Document>();
  d2->push_back(make_shared<StringField>(L"f0", L"v0", Field::Store::YES));
  d2->push_back(make_shared<StringField>(L"f15", L"v15", Field::Store::YES));
  d2->push_back(make_shared<StringField>(L"f16", L"v16", Field::Store::YES));
  writer->addDocument(d2);
  writer->commit();

  shared_ptr<Document> d3 = make_shared<Document>();
  writer->addDocument(d3);
  delete writer;

  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  assertEquals(3, sis->size());

  shared_ptr<FieldInfos> fis1 = IndexWriter::readFieldInfos(sis->info(0));
  shared_ptr<FieldInfos> fis2 = IndexWriter::readFieldInfos(sis->info(1));
  shared_ptr<FieldInfos> fis3 = IndexWriter::readFieldInfos(sis->info(2));

  // testing dense FieldInfos
  org::apache::lucene::index::FieldInfos::const_iterator it = fis1->begin();
  int i = 0;
  while (it != fis1->end()) {
    shared_ptr<FieldInfo> fi = *it;
    assertEquals(i, fi->number);
    assertEquals(L"f" + to_wstring(i), fi->name);
    assertEquals(L"f" + to_wstring(i),
                 fis1->fieldInfo(i)->name); // lookup by number
    assertEquals(L"f" + to_wstring(i),
                 fis1->fieldInfo(L"f" + to_wstring(i))->name); // lookup by name
    i++;
    it++;
  }

  // testing sparse FieldInfos
  assertEquals(L"f0", fis2->fieldInfo(0)->name);     // lookup by number
  assertEquals(L"f0", fis2->fieldInfo(L"f0")->name); // lookup by name
  assertNull(fis2->fieldInfo(1));
  assertNull(fis2->fieldInfo(L"f1"));
  assertEquals(L"f15", fis2->fieldInfo(15)->name);
  assertEquals(L"f15", fis2->fieldInfo(L"f15")->name);
  assertEquals(L"f16", fis2->fieldInfo(16)->name);
  assertEquals(L"f16", fis2->fieldInfo(L"f16")->name);

  // testing empty FieldInfos
  assertNull(fis3->fieldInfo(0));     // lookup by number
  assertNull(fis3->fieldInfo(L"f0")); // lookup by name
  assertEquals(0, fis3->size());
  org::apache::lucene::index::FieldInfos::const_iterator it3 = fis3->begin();
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertFalse(it3->hasNext());
  delete dir;
}
} // namespace org::apache::lucene::index