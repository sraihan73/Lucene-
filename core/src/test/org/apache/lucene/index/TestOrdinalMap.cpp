using namespace std;

#include "TestOrdinalMap.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using OrdinalMap = org::apache::lucene::index::OrdinalMap;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LongValues = org::apache::lucene::util::LongValues;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using RamUsageTester = org::apache::lucene::util::RamUsageTester;
using TestUtil = org::apache::lucene::util::TestUtil;
const shared_ptr<Field> TestOrdinalMap::ORDINAL_MAP_OWNER_FIELD;

TestOrdinalMap::StaticConstructor::StaticConstructor()
{
  try {
    ORDINAL_MAP_OWNER_FIELD = OrdinalMap::typeid->getDeclaredField(L"owner");
  } catch (const runtime_error &e) {
    throw make_shared<Error>();
  }
}

TestOrdinalMap::StaticConstructor TestOrdinalMap::staticConstructor;
const shared_ptr<org::apache::lucene::util::RamUsageTester::Accumulator>
    TestOrdinalMap::ORDINAL_MAP_ACCUMULATOR =
        make_shared<AccumulatorAnonymousInnerClass>();

TestOrdinalMap::AccumulatorAnonymousInnerClass::AccumulatorAnonymousInnerClass()
{
}

int64_t TestOrdinalMap::AccumulatorAnonymousInnerClass::accumulateObject(
    any o, int64_t shallowSize,
    unordered_map<std::shared_ptr<Field>, any> &fieldValues,
    shared_ptr<java::util::deque<any>> queue)
{
  if (o == LongValues::IDENTITY) {
    return 0LL;
  }
  if (std::dynamic_pointer_cast<OrdinalMap>(o) != nullptr) {
    fieldValues = unordered_map<>(fieldValues);
    fieldValues.erase(ORDINAL_MAP_OWNER_FIELD);
  }
  return outerInstance->super.accumulateObject(o, shallowSize, fieldValues,
                                               queue);
}

void TestOrdinalMap::testRamBytesUsed() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> cfg =
      (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
          ->setCodec(TestUtil::alwaysDocValuesFormat(
              TestUtil::getDefaultDocValuesFormat()));
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, cfg);
  constexpr int maxDoc = TestUtil::nextInt(random(), 10, 1000);
  constexpr int maxTermLength = TestUtil::nextInt(random(), 1, 4);
  for (int i = 0; i < maxDoc; ++i) {
    shared_ptr<Document> d = make_shared<Document>();
    if (random()->nextBoolean()) {
      d->push_back(make_shared<SortedDocValuesField>(
          L"sdv", make_shared<BytesRef>(
                      TestUtil::randomSimpleString(random(), maxTermLength))));
    }
    constexpr int numSortedSet = random()->nextInt(3);
    for (int j = 0; j < numSortedSet; ++j) {
      d->push_back(make_shared<SortedSetDocValuesField>(
          L"ssdv", make_shared<BytesRef>(
                       TestUtil::randomSimpleString(random(), maxTermLength))));
    }
    iw->addDocument(d);
    if (rarely()) {
      iw->getReader()->close();
    }
  }
  iw->commit();
  shared_ptr<DirectoryReader> r = iw->getReader();
  shared_ptr<SortedDocValues> sdv = MultiDocValues::getSortedValues(r, L"sdv");
  if (std::dynamic_pointer_cast<MultiDocValues::MultiSortedDocValues>(sdv) !=
      nullptr) {
    shared_ptr<OrdinalMap> map_obj =
        (std::static_pointer_cast<MultiDocValues::MultiSortedDocValues>(sdv))
            ->mapping;
    TestUtil::assertEquals(RamUsageTester::sizeOf(map_obj, ORDINAL_MAP_ACCUMULATOR),
                           map_obj->ramBytesUsed());
  }
  shared_ptr<SortedSetDocValues> ssdv =
      MultiDocValues::getSortedSetValues(r, L"ssdv");
  if (std::dynamic_pointer_cast<MultiDocValues::MultiSortedSetDocValues>(
          ssdv) != nullptr) {
    shared_ptr<OrdinalMap> map_obj =
        (std::static_pointer_cast<MultiDocValues::MultiSortedSetDocValues>(
             ssdv))
            ->mapping;
    TestUtil::assertEquals(RamUsageTester::sizeOf(map_obj, ORDINAL_MAP_ACCUMULATOR),
                           map_obj->ramBytesUsed());
  }
  delete iw;
  r->close();
  delete dir;
}
} // namespace org::apache::lucene::index