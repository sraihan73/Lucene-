using namespace std;

#include "BaseCompressingDocValuesFormatTestCase.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using Directory = org::apache::lucene::store::Directory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using TestUtil = org::apache::lucene::util::TestUtil;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
using com::carrotsearch::randomizedtesting::generators::RandomPicks;

int64_t BaseCompressingDocValuesFormatTestCase::dirSize(
    shared_ptr<Directory> d) 
{
  int64_t size = 0;
  for (auto file : d->listAll()) {
    size += d->fileLength(file);
  }
  return size;
}

void BaseCompressingDocValuesFormatTestCase::
    testUniqueValuesCompression() 
{
  shared_ptr<Directory> *const dir = make_shared<RAMDirectory>();
  shared_ptr<IndexWriterConfig> *const iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> *const iwriter = make_shared<IndexWriter>(dir, iwc);

  constexpr int uniqueValueCount = TestUtil::nextInt(random(), 1, 256);
  const deque<int64_t> values = deque<int64_t>();

  shared_ptr<Document> *const doc = make_shared<Document>();
  shared_ptr<NumericDocValuesField> *const dvf =
      make_shared<NumericDocValuesField>(L"dv", 0);
  doc->push_back(dvf);
  for (int i = 0; i < 300; ++i) {
    constexpr int64_t value;
    if (values.size() < uniqueValueCount) {
      value = random()->nextLong();
      values.push_back(value);
    } else {
      value = RandomPicks::randomFrom(random(), values);
    }
    dvf->setLongValue(value);
    iwriter->addDocument(doc);
  }
  iwriter->forceMerge(1);
  constexpr int64_t size1 = dirSize(dir);
  for (int i = 0; i < 20; ++i) {
    dvf->setLongValue(RandomPicks::randomFrom(random(), values));
    iwriter->addDocument(doc);
  }
  iwriter->forceMerge(1);
  constexpr int64_t size2 = dirSize(dir);
  // make sure the new longs did not cost 8 bytes each
  assertTrue(size2 < size1 + 8 * 20);
}

void BaseCompressingDocValuesFormatTestCase::testDateCompression() throw(
    IOException)
{
  shared_ptr<Directory> *const dir = make_shared<RAMDirectory>();
  shared_ptr<IndexWriterConfig> *const iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> *const iwriter = make_shared<IndexWriter>(dir, iwc);

  constexpr int64_t base = 13; // prime
  constexpr int64_t day = 1000LL * 60 * 60 * 24;

  shared_ptr<Document> *const doc = make_shared<Document>();
  shared_ptr<NumericDocValuesField> *const dvf =
      make_shared<NumericDocValuesField>(L"dv", 0);
  doc->push_back(dvf);
  for (int i = 0; i < 300; ++i) {
    dvf->setLongValue(base + random()->nextInt(1000) * day);
    iwriter->addDocument(doc);
  }
  iwriter->forceMerge(1);
  constexpr int64_t size1 = dirSize(dir);
  for (int i = 0; i < 50; ++i) {
    dvf->setLongValue(base + random()->nextInt(1000) * day);
    iwriter->addDocument(doc);
  }
  iwriter->forceMerge(1);
  constexpr int64_t size2 = dirSize(dir);
  // make sure the new longs costed less than if they had only been packed
  assertTrue(size2 < size1 + (PackedInts::bitsRequired(day) * 50) / 8);
}

void BaseCompressingDocValuesFormatTestCase::
    testSingleBigValueCompression() 
{
  shared_ptr<Directory> *const dir = make_shared<RAMDirectory>();
  shared_ptr<IndexWriterConfig> *const iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> *const iwriter = make_shared<IndexWriter>(dir, iwc);

  shared_ptr<Document> *const doc = make_shared<Document>();
  shared_ptr<NumericDocValuesField> *const dvf =
      make_shared<NumericDocValuesField>(L"dv", 0);
  doc->push_back(dvf);
  for (int i = 0; i < 20000; ++i) {
    dvf->setLongValue(i & 1023);
    iwriter->addDocument(doc);
  }
  iwriter->forceMerge(1);
  constexpr int64_t size1 = dirSize(dir);
  dvf->setLongValue(numeric_limits<int64_t>::max());
  iwriter->addDocument(doc);
  iwriter->forceMerge(1);
  constexpr int64_t size2 = dirSize(dir);
  // make sure the new value did not grow the bpv for every other value
  assertTrue(size2 < size1 + (20000 * (63 - 10)) / 8);
}
} // namespace org::apache::lucene::index