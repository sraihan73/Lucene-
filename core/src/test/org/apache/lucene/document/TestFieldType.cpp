using namespace std;

#include "TestFieldType.h"

namespace org::apache::lucene::document
{
using DocValuesType = org::apache::lucene::index::DocValuesType;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using PointValues = org::apache::lucene::index::PointValues;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using com::carrotsearch::randomizedtesting::generators::RandomPicks;

void TestFieldType::testEquals() 
{
  shared_ptr<FieldType> ft = make_shared<FieldType>();
  assertEquals(ft, ft);
  assertFalse(ft->equals(nullptr));

  shared_ptr<FieldType> ft2 = make_shared<FieldType>();
  assertEquals(ft, ft2);
  assertEquals(ft->hashCode(), ft2->hashCode());

  shared_ptr<FieldType> ft3 = make_shared<FieldType>();
  ft3->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
  assertFalse(ft3->equals(ft));

  shared_ptr<FieldType> ft4 = make_shared<FieldType>();
  ft4->setDocValuesType(DocValuesType::BINARY);
  assertFalse(ft4->equals(ft));

  shared_ptr<FieldType> ft5 = make_shared<FieldType>();
  ft5->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);
  assertFalse(ft5->equals(ft));

  shared_ptr<FieldType> ft6 = make_shared<FieldType>();
  ft6->setStored(true);
  assertFalse(ft6->equals(ft));

  shared_ptr<FieldType> ft7 = make_shared<FieldType>();
  ft7->setOmitNorms(true);
  assertFalse(ft7->equals(ft));

  shared_ptr<FieldType> ft10 = make_shared<FieldType>();
  ft10->setStoreTermVectors(true);
  assertFalse(ft10->equals(ft));

  shared_ptr<FieldType> ft11 = make_shared<FieldType>();
  ft11->setDimensions(1, 4);
  assertFalse(ft11->equals(ft));
}

void TestFieldType::testPointsToString()
{
  shared_ptr<FieldType> ft = make_shared<FieldType>();
  ft->setDimensions(1, Integer::BYTES);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"pointDimensionCount=1,pointNumBytes=4", ft->toString());
}

any TestFieldType::randomValue(type_info clazz)
{
  if (clazz.isEnum()) {
    return RandomPicks::randomFrom(random(), clazz.getEnumConstants());
  } else if (clazz == bool ::typeid) {
    return random()->nextBoolean();
  } else if (clazz == int ::typeid) {
    return 1 + random()->nextInt(100);
  }
  throw make_shared<AssertionError>(L"Don't know how to generate a " + clazz);
}

shared_ptr<FieldType> TestFieldType::randomFieldType() 
{
  // setDimensions handled special as values must be in-bounds.
  shared_ptr<Method> setDimensionsMethod = FieldType::typeid->getMethod(
      L"setDimensions", int ::typeid, int ::typeid);
  shared_ptr<FieldType> ft = make_shared<FieldType>();
  for (shared_ptr<Method> method : FieldType::typeid->getMethods()) {
    if (method->getName()->startsWith(L"set")) {
      const std::deque<type_info> parameterTypes = method->getParameterTypes();
      const std::deque<any> args = std::deque<any>(parameterTypes.size());
      if (method->equals(setDimensionsMethod)) {
        args[0] = 1 + random()->nextInt(PointValues::MAX_DIMENSIONS);
        args[1] = 1 + random()->nextInt(PointValues::MAX_NUM_BYTES);
      } else {
        for (int i = 0; i < args.size(); ++i) {
          args[i] = randomValue(parameterTypes[i]);
        }
      }
      method->invoke(ft, args);
    }
  }
  return ft;
}

void TestFieldType::testCopyConstructor() 
{
  constexpr int iters = 10;
  for (int iter = 0; iter < iters; ++iter) {
    shared_ptr<FieldType> ft = randomFieldType();
    shared_ptr<FieldType> ft2 = make_shared<FieldType>(ft);
    assertEquals(ft, ft2);
  }
}
} // namespace org::apache::lucene::document