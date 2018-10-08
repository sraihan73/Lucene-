using namespace std;

#include "TestCompressingStoredFieldsFormat.h"

namespace org::apache::lucene::codecs::compressing
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using IntPoint = org::apache::lucene::document::IntPoint;
using StoredField = org::apache::lucene::document::StoredField;
using BaseStoredFieldsFormatTestCase =
    org::apache::lucene::index::BaseStoredFieldsFormatTestCase;
using CodecReader = org::apache::lucene::index::CodecReader;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NoMergePolicy = org::apache::lucene::index::NoMergePolicy;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using ByteArrayDataOutput = org::apache::lucene::store::ByteArrayDataOutput;
using Directory = org::apache::lucene::store::Directory;
using com::carrotsearch::randomizedtesting::generators::RandomNumbers;

shared_ptr<Codec> TestCompressingStoredFieldsFormat::getCodec()
{
  return CompressingCodec::randomInstance(random());
}

void TestCompressingStoredFieldsFormat::
    testDeletePartiallyWrittenFilesIfAbort() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwConf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwConf->setMaxBufferedDocs(RandomNumbers::randomIntBetween(random(), 2, 30));
  iwConf->setCodec(CompressingCodec::randomInstance(random()));
  // disable CFS because this test checks file names
  iwConf->setMergePolicy(newLogMergePolicy(false));
  iwConf->setUseCompoundFile(false);

  // Cannot use RIW because this test wants CFS to stay off:
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(dir, iwConf);

  shared_ptr<Document> *const validDoc = make_shared<Document>();
  validDoc->push_back(make_shared<IntPoint>(L"id", 0));
  validDoc->push_back(make_shared<StoredField>(L"id", 0));
  iw->addDocument(validDoc);
  iw->commit();

  // make sure that #writeField will fail to trigger an abort
  shared_ptr<Document> *const invalidDoc = make_shared<Document>();
  shared_ptr<FieldType> fieldType = make_shared<FieldType>();
  fieldType->setStored(true);
  invalidDoc->push_back(
      make_shared<FieldAnonymousInnerClass>(shared_from_this(), fieldType));

  try {
    iw->addDocument(invalidDoc);
    iw->commit();
  } catch (const invalid_argument &iae) {
    // expected
    assertEquals(iae, iw->getTragicException());
  }
  // Writer should be closed by tragedy
  assertFalse(iw->isOpen());
  delete dir;
}

TestCompressingStoredFieldsFormat::FieldAnonymousInnerClass::
    FieldAnonymousInnerClass(
        shared_ptr<TestCompressingStoredFieldsFormat> outerInstance,
        shared_ptr<FieldType> fieldType)
    : org::apache::lucene::document::Field(L"invalid", fieldType)
{
  this->outerInstance = outerInstance;
}

wstring
TestCompressingStoredFieldsFormat::FieldAnonymousInnerClass::stringValue()
{
  // TODO: really bad & scary that this causes IW to
  // abort the segment!!  We should fix this.
  return L"";
}

void TestCompressingStoredFieldsFormat::testZFloat() 
{
  std::deque<char> buffer(5); // we never need more than 5 bytes
  shared_ptr<ByteArrayDataOutput> out =
      make_shared<ByteArrayDataOutput>(buffer);
  shared_ptr<ByteArrayDataInput> in_ = make_shared<ByteArrayDataInput>(buffer);

  // round-trip small integer values
  for (int i = numeric_limits<short>::min(); i < numeric_limits<short>::max();
       i++) {
    float f = static_cast<float>(i);
    CompressingStoredFieldsWriter::writeZFloat(out, f);
    in_->reset(buffer, 0, out->getPosition());
    float g = CompressingStoredFieldsReader::readZFloat(in_);
    assertTrue(in_->eof());
    assertEquals(Float::floatToIntBits(f), Float::floatToIntBits(g));

    // check that compression actually works
    if (i >= -1 && i <= 123) {
      assertEquals(1, out->getPosition()); // single byte compression
    }
    out->reset(buffer);
  }

  // round-trip special values
  std::deque<float> special = {-0.0f,
                                +0.0f,
                                -numeric_limits<float>::infinity(),
                                numeric_limits<float>::infinity(),
                                Float::MIN_VALUE,
                                numeric_limits<float>::max(),
                                NAN};

  for (auto f : special) {
    CompressingStoredFieldsWriter::writeZFloat(out, f);
    in_->reset(buffer, 0, out->getPosition());
    float g = CompressingStoredFieldsReader::readZFloat(in_);
    assertTrue(in_->eof());
    assertEquals(Float::floatToIntBits(f), Float::floatToIntBits(g));
    out->reset(buffer);
  }

  // round-trip random values
  shared_ptr<Random> r = random();
  for (int i = 0; i < 100000; i++) {
    float f = r->nextFloat() * (random()->nextInt(100) - 50);
    CompressingStoredFieldsWriter::writeZFloat(out, f);
    assertTrue(
        L"length=" + to_wstring(out->getPosition()) + L", f=" + to_wstring(f),
        out->getPosition() <=
            ((static_cast<int>(
                 static_cast<unsigned int>(Float::floatToIntBits(f)) >> 31)) ==
                     1
                 ? 5
                 : 4));
    in_->reset(buffer, 0, out->getPosition());
    float g = CompressingStoredFieldsReader::readZFloat(in_);
    assertTrue(in_->eof());
    assertEquals(Float::floatToIntBits(f), Float::floatToIntBits(g));
    out->reset(buffer);
  }
}

void TestCompressingStoredFieldsFormat::testZDouble() 
{
  std::deque<char> buffer(9); // we never need more than 9 bytes
  shared_ptr<ByteArrayDataOutput> out =
      make_shared<ByteArrayDataOutput>(buffer);
  shared_ptr<ByteArrayDataInput> in_ = make_shared<ByteArrayDataInput>(buffer);

  // round-trip small integer values
  for (int i = numeric_limits<short>::min(); i < numeric_limits<short>::max();
       i++) {
    double x = static_cast<double>(i);
    CompressingStoredFieldsWriter::writeZDouble(out, x);
    in_->reset(buffer, 0, out->getPosition());
    double y = CompressingStoredFieldsReader::readZDouble(in_);
    assertTrue(in_->eof());
    assertEquals(Double::doubleToLongBits(x), Double::doubleToLongBits(y));

    // check that compression actually works
    if (i >= -1 && i <= 124) {
      assertEquals(1, out->getPosition()); // single byte compression
    }
    out->reset(buffer);
  }

  // round-trip special values
  std::deque<double> special = {-0.0,
                                 +0.0,
                                 -numeric_limits<double>::infinity(),
                                 numeric_limits<double>::infinity(),
                                 Double::MIN_VALUE,
                                 numeric_limits<double>::max(),
                                 NAN};

  for (auto x : special) {
    CompressingStoredFieldsWriter::writeZDouble(out, x);
    in_->reset(buffer, 0, out->getPosition());
    double y = CompressingStoredFieldsReader::readZDouble(in_);
    assertTrue(in_->eof());
    assertEquals(Double::doubleToLongBits(x), Double::doubleToLongBits(y));
    out->reset(buffer);
  }

  // round-trip random values
  shared_ptr<Random> r = random();
  for (int i = 0; i < 100000; i++) {
    double x = r->nextDouble() * (random()->nextInt(100) - 50);
    CompressingStoredFieldsWriter::writeZDouble(out, x);
    assertTrue(L"length=" + to_wstring(out->getPosition()) + L", d=" +
                   to_wstring(x),
               out->getPosition() <= (x < 0 ? 9 : 8));
    in_->reset(buffer, 0, out->getPosition());
    double y = CompressingStoredFieldsReader::readZDouble(in_);
    assertTrue(in_->eof());
    assertEquals(Double::doubleToLongBits(x), Double::doubleToLongBits(y));
    out->reset(buffer);
  }

  // same with floats
  for (int i = 0; i < 100000; i++) {
    double x =
        static_cast<double>(r->nextFloat() * (random()->nextInt(100) - 50));
    CompressingStoredFieldsWriter::writeZDouble(out, x);
    assertTrue(L"length=" + to_wstring(out->getPosition()) + L", d=" +
                   to_wstring(x),
               out->getPosition() <= 5);
    in_->reset(buffer, 0, out->getPosition());
    double y = CompressingStoredFieldsReader::readZDouble(in_);
    assertTrue(in_->eof());
    assertEquals(Double::doubleToLongBits(x), Double::doubleToLongBits(y));
    out->reset(buffer);
  }
}

void TestCompressingStoredFieldsFormat::testTLong() 
{
  std::deque<char> buffer(10); // we never need more than 10 bytes
  shared_ptr<ByteArrayDataOutput> out =
      make_shared<ByteArrayDataOutput>(buffer);
  shared_ptr<ByteArrayDataInput> in_ = make_shared<ByteArrayDataInput>(buffer);

  // round-trip small integer values
  for (int i = numeric_limits<short>::min(); i < numeric_limits<short>::max();
       i++) {
    for (auto mul : std::deque<int64_t>{SECOND, HOUR, DAY}) {
      int64_t l1 = static_cast<int64_t>(i) * mul;
      CompressingStoredFieldsWriter::writeTLong(out, l1);
      in_->reset(buffer, 0, out->getPosition());
      int64_t l2 = CompressingStoredFieldsReader::readTLong(in_);
      assertTrue(in_->eof());
      assertEquals(l1, l2);

      // check that compression actually works
      if (i >= -16 && i <= 15) {
        assertEquals(1, out->getPosition()); // single byte compression
      }
      out->reset(buffer);
    }
  }

  // round-trip random values
  shared_ptr<Random> r = random();
  for (int i = 0; i < 100000; i++) {
    constexpr int numBits = r->nextInt(65);
    int64_t l1 = r->nextLong() & ((1LL << numBits) - 1);
    switch (r->nextInt(4)) {
    case 0:
      l1 *= SECOND;
      break;
    case 1:
      l1 *= HOUR;
      break;
    case 2:
      l1 *= DAY;
      break;
    default:
      break;
    }
    CompressingStoredFieldsWriter::writeTLong(out, l1);
    in_->reset(buffer, 0, out->getPosition());
    int64_t l2 = CompressingStoredFieldsReader::readTLong(in_);
    assertTrue(in_->eof());
    assertEquals(l1, l2);
    out->reset(buffer);
  }
}

void TestCompressingStoredFieldsFormat::testChunkCleanup() 
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
    doc->push_back(make_shared<StoredField>(L"text", L"not very long at all"));
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
      shared_ptr<CompressingStoredFieldsReader> reader =
          std::static_pointer_cast<CompressingStoredFieldsReader>(
              sr->getFieldsReader());
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
  shared_ptr<CompressingStoredFieldsReader> reader =
      std::static_pointer_cast<CompressingStoredFieldsReader>(
          sr->getFieldsReader());
  // we could get lucky, and have zero, but typically one.
  assertTrue(reader->getNumDirtyChunks() <= 1);
  ir->close();
  delete iw;
  delete dir;
}
} // namespace org::apache::lucene::codecs::compressing