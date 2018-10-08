using namespace std;

#include "TestOmitPositions.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestOmitPositions::testBasic() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
  shared_ptr<Field> f = newField(L"foo", L"this is a test test", ft);
  doc->push_back(f);
  for (int i = 0; i < 100; i++) {
    w->addDocument(doc);
  }

  shared_ptr<IndexReader> reader = w->getReader();
  delete w;

  assertNotNull(MultiFields::getTermPositionsEnum(
      reader, L"foo", make_shared<BytesRef>(L"test")));

  shared_ptr<PostingsEnum> de =
      TestUtil::docs(random(), reader, L"foo", make_shared<BytesRef>(L"test"),
                     nullptr, PostingsEnum::FREQS);
  while (de->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
    TestUtil::assertEquals(2, de->freq());
  }

  delete reader;
  delete dir;
}

void TestOmitPositions::testPositions() 
{
  shared_ptr<Directory> ram = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(ram, newIndexWriterConfig(analyzer));
  shared_ptr<Document> d = make_shared<Document>();

  // f1,f2,f3: docs only
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setIndexOptions(IndexOptions::DOCS);

  shared_ptr<Field> f1 = newField(L"f1", L"This field has docs only", ft);
  d->push_back(f1);

  shared_ptr<Field> f2 = newField(L"f2", L"This field has docs only", ft);
  d->push_back(f2);

  shared_ptr<Field> f3 = newField(L"f3", L"This field has docs only", ft);
  d->push_back(f3);

  shared_ptr<FieldType> ft2 =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft2->setIndexOptions(IndexOptions::DOCS_AND_FREQS);

  // f4,f5,f6 docs and freqs
  shared_ptr<Field> f4 = newField(L"f4", L"This field has docs and freqs", ft2);
  d->push_back(f4);

  shared_ptr<Field> f5 = newField(L"f5", L"This field has docs and freqs", ft2);
  d->push_back(f5);

  shared_ptr<Field> f6 = newField(L"f6", L"This field has docs and freqs", ft2);
  d->push_back(f6);

  shared_ptr<FieldType> ft3 =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft3->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);

  // f7,f8,f9 docs/freqs/positions
  shared_ptr<Field> f7 =
      newField(L"f7", L"This field has docs and freqs and positions", ft3);
  d->push_back(f7);

  shared_ptr<Field> f8 =
      newField(L"f8", L"This field has docs and freqs and positions", ft3);
  d->push_back(f8);

  shared_ptr<Field> f9 =
      newField(L"f9", L"This field has docs and freqs and positions", ft3);
  d->push_back(f9);

  writer->addDocument(d);
  writer->forceMerge(1);

  // now we add another document which has docs-only for f1, f4, f7, docs/freqs
  // for f2, f5, f8, and docs/freqs/positions for f3, f6, f9
  d = make_shared<Document>();

  // f1,f4,f7: docs only
  f1 = newField(L"f1", L"This field has docs only", ft);
  d->push_back(f1);

  f4 = newField(L"f4", L"This field has docs only", ft);
  d->push_back(f4);

  f7 = newField(L"f7", L"This field has docs only", ft);
  d->push_back(f7);

  // f2, f5, f8: docs and freqs
  f2 = newField(L"f2", L"This field has docs and freqs", ft2);
  d->push_back(f2);

  f5 = newField(L"f5", L"This field has docs and freqs", ft2);
  d->push_back(f5);

  f8 = newField(L"f8", L"This field has docs and freqs", ft2);
  d->push_back(f8);

  // f3, f6, f9: docs and freqs and positions
  f3 = newField(L"f3", L"This field has docs and freqs and positions", ft3);
  d->push_back(f3);

  f6 = newField(L"f6", L"This field has docs and freqs and positions", ft3);
  d->push_back(f6);

  f9 = newField(L"f9", L"This field has docs and freqs and positions", ft3);
  d->push_back(f9);

  writer->addDocument(d);

  // force merge
  writer->forceMerge(1);
  // flush
  delete writer;

  shared_ptr<LeafReader> reader = getOnlyLeafReader(DirectoryReader::open(ram));
  shared_ptr<FieldInfos> fi = reader->getFieldInfos();
  // docs + docs = docs
  TestUtil::assertEquals(IndexOptions::DOCS,
                         fi->fieldInfo(L"f1")->getIndexOptions());
  // docs + docs/freqs = docs
  TestUtil::assertEquals(IndexOptions::DOCS,
                         fi->fieldInfo(L"f2")->getIndexOptions());
  // docs + docs/freqs/pos = docs
  TestUtil::assertEquals(IndexOptions::DOCS,
                         fi->fieldInfo(L"f3")->getIndexOptions());
  // docs/freqs + docs = docs
  TestUtil::assertEquals(IndexOptions::DOCS,
                         fi->fieldInfo(L"f4")->getIndexOptions());
  // docs/freqs + docs/freqs = docs/freqs
  TestUtil::assertEquals(IndexOptions::DOCS_AND_FREQS,
                         fi->fieldInfo(L"f5")->getIndexOptions());
  // docs/freqs + docs/freqs/pos = docs/freqs
  TestUtil::assertEquals(IndexOptions::DOCS_AND_FREQS,
                         fi->fieldInfo(L"f6")->getIndexOptions());
  // docs/freqs/pos + docs = docs
  TestUtil::assertEquals(IndexOptions::DOCS,
                         fi->fieldInfo(L"f7")->getIndexOptions());
  // docs/freqs/pos + docs/freqs = docs/freqs
  TestUtil::assertEquals(IndexOptions::DOCS_AND_FREQS,
                         fi->fieldInfo(L"f8")->getIndexOptions());
  // docs/freqs/pos + docs/freqs/pos = docs/freqs/pos
  TestUtil::assertEquals(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS,
                         fi->fieldInfo(L"f9")->getIndexOptions());

  delete reader;
  delete ram;
}

void TestOmitPositions::assertNoPrx(shared_ptr<Directory> dir) throw(
    runtime_error)
{
  const std::deque<wstring> files = dir->listAll();
  for (int i = 0; i < files.size(); i++) {
    assertFalse(StringHelper::endsWith(files[i], L".prx"));
    assertFalse(StringHelper::endsWith(files[i], L".pos"));
  }
}

void TestOmitPositions::testNoPrxFile() 
{
  shared_ptr<Directory> ram = newDirectory();

  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      ram,
      newIndexWriterConfig(analyzer)->setMaxBufferedDocs(3)->setMergePolicy(
          newLogMergePolicy()));
  shared_ptr<LogMergePolicy> lmp = std::static_pointer_cast<LogMergePolicy>(
      writer->getConfig()->getMergePolicy());
  lmp->setMergeFactor(2);
  lmp->setNoCFSRatio(0.0);
  shared_ptr<Document> d = make_shared<Document>();

  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
  shared_ptr<Field> f1 = newField(L"f1", L"This field has term freqs", ft);
  d->push_back(f1);

  for (int i = 0; i < 30; i++) {
    writer->addDocument(d);
  }

  writer->commit();

  assertNoPrx(ram);

  // now add some documents with positions, and check there is no prox after
  // optimization
  d = make_shared<Document>();
  f1 = newTextField(L"f1", L"This field has positions", Field::Store::NO);
  d->push_back(f1);

  for (int i = 0; i < 30; i++) {
    writer->addDocument(d);
  }

  // force merge
  writer->forceMerge(1);
  // flush
  delete writer;

  assertNoPrx(ram);
  delete ram;
}

void TestOmitPositions::testMixing() 
{
  // no positions
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setIndexOptions(IndexOptions::DOCS_AND_FREQS);

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);

  for (int i = 0; i < 20; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    if (i < 19 && random()->nextBoolean()) {
      for (int j = 0; j < 50; j++) {
        doc->push_back(make_shared<TextField>(L"foo", L"i have positions",
                                              Field::Store::NO));
      }
    } else {
      for (int j = 0; j < 50; j++) {
        doc->push_back(make_shared<Field>(L"foo", L"i have no positions", ft));
      }
    }
    iw->addDocument(doc);
    iw->commit();
  }

  if (random()->nextBoolean()) {
    iw->forceMerge(1);
  }

  shared_ptr<DirectoryReader> ir = iw->getReader();
  shared_ptr<FieldInfos> fis = MultiFields::getMergedFieldInfos(ir);
  TestUtil::assertEquals(IndexOptions::DOCS_AND_FREQS,
                         fis->fieldInfo(L"foo")->getIndexOptions());
  assertFalse(fis->fieldInfo(L"foo")->hasPayloads());
  delete iw;
  ir->close();
  delete dir; // checkindex
}
} // namespace org::apache::lucene::index