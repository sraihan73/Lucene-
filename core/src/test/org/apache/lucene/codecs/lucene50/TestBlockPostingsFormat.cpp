using namespace std;

#include "TestBlockPostingsFormat.h"

namespace org::apache::lucene::codecs::lucene50
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using FieldReader = org::apache::lucene::codecs::blocktree::FieldReader;
using Stats = org::apache::lucene::codecs::blocktree::Stats;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using BasePostingsFormatTestCase =
    org::apache::lucene::index::BasePostingsFormatTestCase;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using Directory = org::apache::lucene::store::Directory;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<Codec> TestBlockPostingsFormat::getCodec() { return codec; }

void TestBlockPostingsFormat::testFinalBlock() 
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      d, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));
  for (int i = 0; i < 25; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(newStringField(
        L"field", Character::toString(static_cast<wchar_t>(97 + i)),
        Field::Store::NO));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(newStringField(
        L"field", L"z" + Character::toString(static_cast<wchar_t>(97 + i)),
        Field::Store::NO));
    w->addDocument(doc);
  }
  w->forceMerge(1);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  TestUtil::assertEquals(1, r->leaves()->size());
  shared_ptr<FieldReader> field = std::static_pointer_cast<FieldReader>(
      r->leaves()->get(0).reader().terms(L"field"));
  // We should see exactly two blocks: one root block (prefix empty string) and
  // one block for z* terms (prefix z):
  shared_ptr<Stats> stats = field->getStats();
  TestUtil::assertEquals(0, stats->floorBlockCount);
  TestUtil::assertEquals(2, stats->nonFloorBlockCount);
  r->close();
  delete w;
  delete d;
}

void TestBlockPostingsFormat::shouldFail(int minItemsInBlock,
                                         int maxItemsInBlock)
{
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<Lucene50PostingsFormat>(minItemsInBlock, maxItemsInBlock);
  });
}

void TestBlockPostingsFormat::testInvalidBlockSizes() 
{
  shouldFail(0, 0);
  shouldFail(10, 8);
  shouldFail(-1, 10);
  shouldFail(10, -1);
  shouldFail(10, 12);
}
} // namespace org::apache::lucene::codecs::lucene50