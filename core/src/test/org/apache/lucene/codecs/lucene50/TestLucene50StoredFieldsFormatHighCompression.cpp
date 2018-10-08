using namespace std;

#include "TestLucene50StoredFieldsFormatHighCompression.h"

namespace org::apache::lucene::codecs::lucene50
{
using Codec = org::apache::lucene::codecs::Codec;
using Mode =
    org::apache::lucene::codecs::lucene50::Lucene50StoredFieldsFormat::Mode;
using Lucene70Codec = org::apache::lucene::codecs::lucene70::Lucene70Codec;
using Document = org::apache::lucene::document::Document;
using StoredField = org::apache::lucene::document::StoredField;
using BaseStoredFieldsFormatTestCase =
    org::apache::lucene::index::BaseStoredFieldsFormatTestCase;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using Directory = org::apache::lucene::store::Directory;
using com::carrotsearch::randomizedtesting::generators::RandomPicks;

shared_ptr<Codec> TestLucene50StoredFieldsFormatHighCompression::getCodec()
{
  return make_shared<Lucene70Codec>(Mode::BEST_COMPRESSION);
}

void TestLucene50StoredFieldsFormatHighCompression::
    testMixedCompressions() 
{
  shared_ptr<Directory> dir = newDirectory();
  for (int i = 0; i < 10; i++) {
    shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
    iwc->setCodec(make_shared<Lucene70Codec>(
        RandomPicks::randomFrom(random(), Mode::values())));
    shared_ptr<IndexWriter> iw =
        make_shared<IndexWriter>(dir, newIndexWriterConfig());
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StoredField>(L"field1", L"value1"));
    doc->push_back(make_shared<StoredField>(L"field2", L"value2"));
    iw->addDocument(doc);
    if (random()->nextInt(4) == 0) {
      iw->forceMerge(1);
    }
    iw->commit();
    delete iw;
  }

  shared_ptr<DirectoryReader> ir = DirectoryReader::open(dir);
  assertEquals(10, ir->numDocs());
  for (int i = 0; i < 10; i++) {
    shared_ptr<Document> doc = ir->document(i);
    assertEquals(L"value1", doc[L"field1"]);
    assertEquals(L"value2", doc[L"field2"]);
  }
  ir->close();
  // checkindex
  delete dir;
}

void TestLucene50StoredFieldsFormatHighCompression::testInvalidOptions() throw(
    runtime_error)
{
  expectThrows(NullPointerException::typeid,
               [&]() { make_shared<Lucene70Codec>(nullptr); });

  expectThrows(NullPointerException::typeid,
               [&]() { make_shared<Lucene50StoredFieldsFormat>(nullptr); });
}
} // namespace org::apache::lucene::codecs::lucene50