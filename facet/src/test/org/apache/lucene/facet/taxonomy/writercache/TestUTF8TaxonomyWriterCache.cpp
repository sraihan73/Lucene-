using namespace std;

#include "TestUTF8TaxonomyWriterCache.h"

namespace org::apache::lucene::facet::taxonomy::writercache
{
using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using FacetLabel = org::apache::lucene::facet::taxonomy::FacetLabel;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestUTF8TaxonomyWriterCache::testRandom() 
{
  shared_ptr<LabelToOrdinal> map_obj = make_shared<LabelToOrdinalMap>();

  shared_ptr<UTF8TaxonomyWriterCache> cache =
      make_shared<UTF8TaxonomyWriterCache>();

  constexpr int n = atLeast(10 * 1000);
  constexpr int numUniqueValues = 50 * 1000;

  std::deque<char> buffer(50);

  shared_ptr<Random> random = TestUTF8TaxonomyWriterCache::random();
  shared_ptr<Set<wstring>> uniqueValuesSet = unordered_set<wstring>();
  while (uniqueValuesSet->size() < numUniqueValues) {
    int numParts =
        TestUtil::nextInt(TestUTF8TaxonomyWriterCache::random(), 1, 5);
    shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
    for (int i = 0; i < numParts; i++) {
      wstring part = L"";
      while (true) {
        part = TestUtil::randomRealisticUnicodeString(
            TestUTF8TaxonomyWriterCache::random(), 16);
        part = StringHelper::replace(part, L"/", L"");
        if (part.length() > 0) {
          break;
        }
      }

      if (i > 0) {
        b->append(L'/');
      }
      b->append(part);
    }
    uniqueValuesSet->add(b->toString());
  }
  std::deque<wstring> uniqueValues =
      uniqueValuesSet->toArray(std::deque<wstring>(0));

  int ordUpto = 0;
  for (int i = 0; i < n; i++) {

    int index = random->nextInt(numUniqueValues);
    shared_ptr<FacetLabel> label;
    wstring s = uniqueValues[index];
    if (s.length() == 0) {
      label = make_shared<FacetLabel>();
    } else {
      label = make_shared<FacetLabel>(s.split(L"/"));
    }

    int ord1 = map_obj->getOrdinal(label);
    int ord2 = cache->get(label);

    TestUtil::assertEquals(ord1, ord2);

    if (ord1 == LabelToOrdinal::INVALID_ORDINAL) {
      ord1 = ordUpto++;
      map_obj->addLabel(label, ord1);
      cache->put(label, ord1);
    }
  }

  for (int i = 0; i < numUniqueValues; i++) {
    shared_ptr<FacetLabel> label;
    wstring s = uniqueValues[i];
    if (s.length() == 0) {
      label = make_shared<FacetLabel>();
    } else {
      label = make_shared<FacetLabel>(s.split(L"/"));
    }
    int ord1 = map_obj->getOrdinal(label);
    int ord2 = cache->get(label);
    TestUtil::assertEquals(ord1, ord2);
  }
}

TestUTF8TaxonomyWriterCache::LabelToOrdinalMap::LabelToOrdinalMap() {}

void TestUTF8TaxonomyWriterCache::LabelToOrdinalMap::addLabel(
    shared_ptr<FacetLabel> label, int ordinal)
{
  map_obj.emplace(label, ordinal);
}

int TestUTF8TaxonomyWriterCache::LabelToOrdinalMap::getOrdinal(
    shared_ptr<FacetLabel> label)
{
  optional<int> value = map_obj[label];
  return (value) ? value.value() : LabelToOrdinal::INVALID_ORDINAL;
}
} // namespace org::apache::lucene::facet::taxonomy::writercache