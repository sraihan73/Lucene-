using namespace std;

#include "TestBlockPostingsFormat2.h"

namespace org::apache::lucene::codecs::lucene50
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using IndexableField = org::apache::lucene::index::IndexableField;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestBlockPostingsFormat2::setUp() 
{
  LuceneTestCase::setUp();
  dir = newFSDirectory(createTempDir(L"testDFBlockSize"));
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setCodec(
      TestUtil::alwaysPostingsFormat(make_shared<Lucene50PostingsFormat>()));
  iw = make_shared<RandomIndexWriter>(random(), dir, iwc);
  iw->setDoRandomForceMerge(false); // we will ourselves
}

void TestBlockPostingsFormat2::tearDown() 
{
  delete iw;
  TestUtil::checkIndex(
      dir); // for some extra coverage, checkIndex before we forceMerge
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setCodec(
      TestUtil::alwaysPostingsFormat(make_shared<Lucene50PostingsFormat>()));
  iwc->setOpenMode(OpenMode::APPEND);
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, iwc);
  iw->forceMerge(1);
  delete iw;
  delete dir; // just force a checkindex for now
  LuceneTestCase::tearDown();
}

shared_ptr<Document> TestBlockPostingsFormat2::newDocument()
{
  shared_ptr<Document> doc = make_shared<Document>();
  for (IndexOptions option : IndexOptions::values()) {
    if (option == IndexOptions::NONE) {
      continue;
    }
    shared_ptr<FieldType> ft =
        make_shared<FieldType>(TextField::TYPE_NOT_STORED);
    // turn on tvs for a cross-check, since we rely upon checkindex in this test
    // (for now)
    ft->setStoreTermVectors(true);
    ft->setStoreTermVectorOffsets(true);
    ft->setStoreTermVectorPositions(true);
    ft->setStoreTermVectorPayloads(true);
    ft->setIndexOptions(option);
    doc->push_back(make_shared<Field>(option.toString(), L"", ft));
  }
  return doc;
}

void TestBlockPostingsFormat2::testDFBlockSize() 
{
  shared_ptr<Document> doc = newDocument();
  for (int i = 0; i < Lucene50PostingsFormat::BLOCK_SIZE; i++) {
    for (auto f : doc->getFields()) {
      (std::static_pointer_cast<Field>(f))
          ->setStringValue(f->name() + L" " + f->name() + L"_2");
    }
    iw->addDocument(doc);
  }
}

void TestBlockPostingsFormat2::testDFBlockSizeMultiple() 
{
  shared_ptr<Document> doc = newDocument();
  for (int i = 0; i < Lucene50PostingsFormat::BLOCK_SIZE * 16; i++) {
    for (auto f : doc->getFields()) {
      (std::static_pointer_cast<Field>(f))
          ->setStringValue(f->name() + L" " + f->name() + L"_2");
    }
    iw->addDocument(doc);
  }
}

void TestBlockPostingsFormat2::testTTFBlockSize() 
{
  shared_ptr<Document> doc = newDocument();
  for (int i = 0; i < Lucene50PostingsFormat::BLOCK_SIZE / 2; i++) {
    for (auto f : doc->getFields()) {
      (std::static_pointer_cast<Field>(f))
          ->setStringValue(f->name() + L" " + f->name() + L" " + f->name() +
                           L"_2 " + f->name() + L"_2");
    }
    iw->addDocument(doc);
  }
}

void TestBlockPostingsFormat2::testTTFBlockSizeMultiple() 
{
  shared_ptr<Document> doc = newDocument();
  for (int i = 0; i < Lucene50PostingsFormat::BLOCK_SIZE / 2; i++) {
    for (auto f : doc->getFields()) {
      wstring proto = (f->name() + L" " + f->name() + L" " + f->name() + L" " +
                       f->name() + L" " + f->name() + L"_2 " + f->name() +
                       L"_2 " + f->name() + L"_2 " + f->name() + L"_2");
      shared_ptr<StringBuilder> val = make_shared<StringBuilder>();
      for (int j = 0; j < 16; j++) {
        val->append(proto);
        val->append(L" ");
      }
      (std::static_pointer_cast<Field>(f))->setStringValue(val->toString());
    }
    iw->addDocument(doc);
  }
}
} // namespace org::apache::lucene::codecs::lucene50