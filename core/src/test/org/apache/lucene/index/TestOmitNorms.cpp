using namespace std;

#include "TestOmitNorms.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

void TestOmitNorms::testOmitNorms() 
{
  shared_ptr<Directory> ram = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(ram, newIndexWriterConfig(analyzer));
  shared_ptr<Document> d = make_shared<Document>();

  // this field will have norms
  shared_ptr<Field> f1 =
      newTextField(L"f1", L"This field has norms", Field::Store::NO);
  d->push_back(f1);

  // this field will NOT have norms
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setOmitNorms(true);
  shared_ptr<Field> f2 =
      newField(L"f2", L"This field has NO norms in all docs", customType);
  d->push_back(f2);

  writer->addDocument(d);
  writer->forceMerge(1);
  // now we add another document which has term freq for field f2 and not for f1
  // and verify if the SegmentMerger keep things constant
  d = make_shared<Document>();

  // Reverse
  d->push_back(newField(L"f1", L"This field has norms", customType));

  d->push_back(newTextField(L"f2", L"This field has NO norms in all docs",
                            Field::Store::NO));

  writer->addDocument(d);

  // force merge
  writer->forceMerge(1);
  // flush
  delete writer;

  shared_ptr<LeafReader> reader = getOnlyLeafReader(DirectoryReader::open(ram));
  shared_ptr<FieldInfos> fi = reader->getFieldInfos();
  assertTrue(L"OmitNorms field bit should be set.",
             fi->fieldInfo(L"f1")->omitsNorms());
  assertTrue(L"OmitNorms field bit should be set.",
             fi->fieldInfo(L"f2")->omitsNorms());

  delete reader;
  delete ram;
}

void TestOmitNorms::testMixedMerge() 
{
  shared_ptr<Directory> ram = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      ram,
      newIndexWriterConfig(analyzer)->setMaxBufferedDocs(3)->setMergePolicy(
          newLogMergePolicy(2)));
  shared_ptr<Document> d = make_shared<Document>();

  // this field will have norms
  shared_ptr<Field> f1 =
      newTextField(L"f1", L"This field has norms", Field::Store::NO);
  d->push_back(f1);

  // this field will NOT have norms
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setOmitNorms(true);
  shared_ptr<Field> f2 =
      newField(L"f2", L"This field has NO norms in all docs", customType);
  d->push_back(f2);

  for (int i = 0; i < 30; i++) {
    writer->addDocument(d);
  }

  // now we add another document which has norms for field f2 and not for f1 and
  // verify if the SegmentMerger keep things constant
  d = make_shared<Document>();

  // Reverese
  d->push_back(newField(L"f1", L"This field has norms", customType));

  d->push_back(newTextField(L"f2", L"This field has NO norms in all docs",
                            Field::Store::NO));

  for (int i = 0; i < 30; i++) {
    writer->addDocument(d);
  }

  // force merge
  writer->forceMerge(1);
  // flush
  delete writer;

  shared_ptr<LeafReader> reader = getOnlyLeafReader(DirectoryReader::open(ram));
  shared_ptr<FieldInfos> fi = reader->getFieldInfos();
  assertTrue(L"OmitNorms field bit should be set.",
             fi->fieldInfo(L"f1")->omitsNorms());
  assertTrue(L"OmitNorms field bit should be set.",
             fi->fieldInfo(L"f2")->omitsNorms());

  delete reader;
  delete ram;
}

void TestOmitNorms::testMixedRAM() 
{
  shared_ptr<Directory> ram = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      ram,
      newIndexWriterConfig(analyzer)->setMaxBufferedDocs(10)->setMergePolicy(
          newLogMergePolicy(2)));
  shared_ptr<Document> d = make_shared<Document>();

  // this field will have norms
  shared_ptr<Field> f1 =
      newTextField(L"f1", L"This field has norms", Field::Store::NO);
  d->push_back(f1);

  // this field will NOT have norms

  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setOmitNorms(true);
  shared_ptr<Field> f2 =
      newField(L"f2", L"This field has NO norms in all docs", customType);
  d->push_back(f2);

  for (int i = 0; i < 5; i++) {
    writer->addDocument(d);
  }

  for (int i = 0; i < 20; i++) {
    writer->addDocument(d);
  }

  // force merge
  writer->forceMerge(1);

  // flush
  delete writer;

  shared_ptr<LeafReader> reader = getOnlyLeafReader(DirectoryReader::open(ram));
  shared_ptr<FieldInfos> fi = reader->getFieldInfos();
  assertTrue(L"OmitNorms field bit should not be set.",
             !fi->fieldInfo(L"f1")->omitsNorms());
  assertTrue(L"OmitNorms field bit should be set.",
             fi->fieldInfo(L"f2")->omitsNorms());

  delete reader;
  delete ram;
}

void TestOmitNorms::assertNoNrm(shared_ptr<Directory> dir) 
{
  const std::deque<wstring> files = dir->listAll();
  for (int i = 0; i < files.size(); i++) {
    // TODO: this relies upon filenames
    assertFalse(StringHelper::endsWith(files[i], L".nrm") ||
                StringHelper::endsWith(files[i], L".len"));
  }
}

void TestOmitNorms::testNoNrmFile() 
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

  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setOmitNorms(true);
  shared_ptr<Field> f1 =
      newField(L"f1", L"This field has no norms", customType);
  d->push_back(f1);

  for (int i = 0; i < 30; i++) {
    writer->addDocument(d);
  }

  writer->commit();

  assertNoNrm(ram);

  // force merge
  writer->forceMerge(1);
  // flush
  delete writer;

  assertNoNrm(ram);
  delete ram;
}

void TestOmitNorms::testOmitNormsCombos() 
{
  // indexed with norms
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  shared_ptr<Field> norms = make_shared<Field>(L"foo", L"a", customType);
  // indexed without norms
  shared_ptr<FieldType> customType1 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType1->setOmitNorms(true);
  shared_ptr<Field> noNorms = make_shared<Field>(L"foo", L"a", customType1);
  // not indexed, but stored
  shared_ptr<FieldType> customType2 = make_shared<FieldType>();
  customType2->setStored(true);
  shared_ptr<Field> noIndex = make_shared<Field>(L"foo", L"a", customType2);
  // not indexed but stored, omitNorms is set
  shared_ptr<FieldType> customType3 = make_shared<FieldType>();
  customType3->setStored(true);
  customType3->setOmitNorms(true);
  shared_ptr<Field> noNormsNoIndex =
      make_shared<Field>(L"foo", L"a", customType3);
  // not indexed nor stored (doesnt exist at all, we index a different field
  // instead)
  shared_ptr<Field> emptyNorms = make_shared<Field>(L"bar", L"a", customType);

  assertNotNull(getNorms(L"foo", norms, norms));
  assertNull(getNorms(L"foo", norms, noNorms));
  assertNotNull(getNorms(L"foo", norms, noIndex));
  assertNotNull(getNorms(L"foo", norms, noNormsNoIndex));
  assertNotNull(getNorms(L"foo", norms, emptyNorms));
  assertNull(getNorms(L"foo", noNorms, noNorms));
  assertNull(getNorms(L"foo", noNorms, noIndex));
  assertNull(getNorms(L"foo", noNorms, noNormsNoIndex));
  assertNull(getNorms(L"foo", noNorms, emptyNorms));
  assertNull(getNorms(L"foo", noIndex, noIndex));
  assertNull(getNorms(L"foo", noIndex, noNormsNoIndex));
  assertNull(getNorms(L"foo", noIndex, emptyNorms));
  assertNull(getNorms(L"foo", noNormsNoIndex, noNormsNoIndex));
  assertNull(getNorms(L"foo", noNormsNoIndex, emptyNorms));
  assertNull(getNorms(L"foo", emptyNorms, emptyNorms));
}

shared_ptr<NumericDocValues>
TestOmitNorms::getNorms(const wstring &field, shared_ptr<Field> f1,
                        shared_ptr<Field> f2) 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> riw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  // add f1
  shared_ptr<Document> d = make_shared<Document>();
  d->push_back(f1);
  riw->addDocument(d);

  // add f2
  d = make_shared<Document>();
  d->push_back(f2);
  riw->addDocument(d);

  // add a mix of f1's and f2's
  int numExtraDocs = TestUtil::nextInt(random(), 1, 1000);
  for (int i = 0; i < numExtraDocs; i++) {
    d = make_shared<Document>();
    d->push_back(random()->nextBoolean() ? f1 : f2);
    riw->addDocument(d);
  }

  shared_ptr<IndexReader> ir1 = riw->getReader();
  // todo: generalize
  shared_ptr<NumericDocValues> norms1 =
      MultiDocValues::getNormValues(ir1, field);

  // fully merge and validate MultiNorms against single segment.
  riw->forceMerge(1);
  shared_ptr<DirectoryReader> ir2 = riw->getReader();
  shared_ptr<NumericDocValues> norms2 =
      getOnlyLeafReader(ir2)->getNormValues(field);

  if (norms1 == nullptr) {
    assertNull(norms2);
  } else {
    while (true) {
      int norms1DocID = norms1->nextDoc();
      int norms2DocID = norms2->nextDoc();
      while (norms1DocID < norms2DocID) {
        TestUtil::assertEquals(0, norms1->longValue());
        norms1DocID = norms1->nextDoc();
      }
      while (norms2DocID < norms1DocID) {
        TestUtil::assertEquals(0, norms2->longValue());
        norms2DocID = norms2->nextDoc();
      }
      if (norms1->docID() == NO_MORE_DOCS) {
        break;
      }
      TestUtil::assertEquals(norms1->longValue(), norms2->longValue());
    }
  }
  delete ir1;
  ir2->close();
  delete riw;
  delete dir;
  return norms1;
}
} // namespace org::apache::lucene::index