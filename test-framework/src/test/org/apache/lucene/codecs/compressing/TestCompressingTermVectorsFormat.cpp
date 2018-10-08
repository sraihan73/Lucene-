using namespace std;

#include "TestCompressingTermVectorsFormat.h"

namespace org::apache::lucene::codecs::compressing
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using BaseTermVectorsFormatTestCase =
    org::apache::lucene::index::BaseTermVectorsFormatTestCase;
using CodecReader = org::apache::lucene::index::CodecReader;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NoMergePolicy = org::apache::lucene::index::NoMergePolicy;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Terms = org::apache::lucene::index::Terms;
using SeekStatus = org::apache::lucene::index::TermsEnum::SeekStatus;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;

shared_ptr<Codec> TestCompressingTermVectorsFormat::getCodec()
{
  return CompressingCodec::randomInstance(random());
}

void TestCompressingTermVectorsFormat::testNoOrds() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setStoreTermVectors(true);
  doc->push_back(make_shared<Field>(L"foo", L"this is a test", ft));
  iw->addDocument(doc);
  shared_ptr<LeafReader> ir = getOnlyLeafReader(iw->getReader());
  shared_ptr<Terms> terms = ir->getTermVector(0, L"foo");
  assertNotNull(terms);
  shared_ptr<TermsEnum> termsEnum = terms->begin();
  assertEquals(SeekStatus::FOUND,
               termsEnum->seekCeil(make_shared<BytesRef>(L"this")));
  try {
    termsEnum->ord();
    fail();
  } catch (const UnsupportedOperationException &expected) {
    // expected exception
  }

  try {
    termsEnum->seekExact(0);
    fail();
  } catch (const UnsupportedOperationException &expected) {
    // expected exception
  }
  delete ir;
  delete iw;
  delete dir;
}

void TestCompressingTermVectorsFormat::testChunkCleanup() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwConf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwConf->setMergePolicy(NoMergePolicy::INSTANCE);

  // we have to enforce certain things like maxDocsPerChunk to cause dirty
  // chunks to be created by this test.
  iwConf->setCodec(
      CompressingCodec::randomInstance(random(), 4 * 1024, 100, false, 8));
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, iwConf);
  shared_ptr<DirectoryReader> ir = DirectoryReader::open(iw);
  for (int i = 0; i < 5; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<FieldType> ft =
        make_shared<FieldType>(TextField::TYPE_NOT_STORED);
    ft->setStoreTermVectors(true);
    doc->push_back(make_shared<Field>(L"text", L"not very long at all", ft));
    iw->addDocument(doc);
    // force flush
    shared_ptr<DirectoryReader> ir2 = DirectoryReader::openIfChanged(ir);
    assertNotNull(ir2);
    ir->close();
    ir = ir2;
    // examine dirty counts:
    for (shared_ptr<LeafReaderContext> leaf : ir2->leaves()) {
      shared_ptr<CodecReader> sr =
          std::static_pointer_cast<CodecReader>(leaf->reader());
      shared_ptr<CompressingTermVectorsReader> reader =
          std::static_pointer_cast<CompressingTermVectorsReader>(
              sr->getTermVectorsReader());
      assertEquals(1, reader->getNumChunks());
      assertEquals(1, reader->getNumDirtyChunks());
    }
  }
  iw->getConfig()->setMergePolicy(newLogMergePolicy());
  iw->forceMerge(1);
  shared_ptr<DirectoryReader> ir2 = DirectoryReader::openIfChanged(ir);
  assertNotNull(ir2);
  ir->close();
  ir = ir2;
  shared_ptr<CodecReader> sr =
      std::static_pointer_cast<CodecReader>(getOnlyLeafReader(ir));
  shared_ptr<CompressingTermVectorsReader> reader =
      std::static_pointer_cast<CompressingTermVectorsReader>(
          sr->getTermVectorsReader());
  // we could get lucky, and have zero, but typically one.
  assertTrue(reader->getNumDirtyChunks() <= 1);
  ir->close();
  delete iw;
  delete dir;
}
} // namespace org::apache::lucene::codecs::compressing