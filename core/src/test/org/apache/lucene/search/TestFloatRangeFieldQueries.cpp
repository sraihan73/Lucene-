using namespace std;

#include "TestFloatRangeFieldQueries.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using FloatRange = org::apache::lucene::document::FloatRange;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
const wstring TestFloatRangeFieldQueries::FIELD_NAME = L"floatRangeField";

float TestFloatRangeFieldQueries::nextFloatInternal()
{
  switch (random()->nextInt(5)) {
  case 0:
    return -numeric_limits<float>::infinity();
  case 1:
    return numeric_limits<float>::infinity();
  default:
    if (random()->nextBoolean()) {
      return random()->nextFloat();
    } else {
      return (random()->nextInt(15) - 7) / 3.0f;
    }
  }
}

shared_ptr<Range>
TestFloatRangeFieldQueries::nextRange(int dimensions) 
{
  std::deque<float> min(dimensions);
  std::deque<float> max(dimensions);

  float minV, maxV;
  for (int d = 0; d < dimensions; ++d) {
    minV = nextFloatInternal();
    maxV = nextFloatInternal();
    min[d] = min(minV, maxV);
    max[d] = max(minV, maxV);
  }
  return make_shared<FloatTestRange>(shared_from_this(), min, max);
}

shared_ptr<FloatRange>
TestFloatRangeFieldQueries::newRangeField(shared_ptr<Range> r)
{
  return make_shared<FloatRange>(
      FIELD_NAME, (std::static_pointer_cast<FloatTestRange>(r))->min,
      (std::static_pointer_cast<FloatTestRange>(r))->max);
}

shared_ptr<Query>
TestFloatRangeFieldQueries::newIntersectsQuery(shared_ptr<Range> r)
{
  return FloatRange::newIntersectsQuery(
      FIELD_NAME, (std::static_pointer_cast<FloatTestRange>(r))->min,
      (std::static_pointer_cast<FloatTestRange>(r))->max);
}

shared_ptr<Query>
TestFloatRangeFieldQueries::newContainsQuery(shared_ptr<Range> r)
{
  return FloatRange::newContainsQuery(
      FIELD_NAME, (std::static_pointer_cast<FloatTestRange>(r))->min,
      (std::static_pointer_cast<FloatTestRange>(r))->max);
}

shared_ptr<Query>
TestFloatRangeFieldQueries::newWithinQuery(shared_ptr<Range> r)
{
  return FloatRange::newWithinQuery(
      FIELD_NAME, (std::static_pointer_cast<FloatTestRange>(r))->min,
      (std::static_pointer_cast<FloatTestRange>(r))->max);
}

shared_ptr<Query>
TestFloatRangeFieldQueries::newCrossesQuery(shared_ptr<Range> r)
{
  return FloatRange::newCrossesQuery(
      FIELD_NAME, (std::static_pointer_cast<FloatTestRange>(r))->min,
      (std::static_pointer_cast<FloatTestRange>(r))->max);
}

void TestFloatRangeFieldQueries::testBasics() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  // intersects (within)
  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(
      make_shared<FloatRange>(FIELD_NAME, std::deque<float>{-10.0f, -10.0f},
                              std::deque<float>{9.1f, 10.1f}));
  writer->addDocument(document);

  // intersects (crosses)
  document = make_shared<Document>();
  document->push_back(
      make_shared<FloatRange>(FIELD_NAME, std::deque<float>{10.0f, -10.0f},
                              std::deque<float>{20.0f, 10.0f}));
  writer->addDocument(document);

  // intersects (contains, crosses)
  document = make_shared<Document>();
  document->push_back(
      make_shared<FloatRange>(FIELD_NAME, std::deque<float>{-20.0f, -20.0f},
                              std::deque<float>{30.0f, 30.1f}));
  writer->addDocument(document);

  // intersects (crosses)
  document = make_shared<Document>();
  document->push_back(
      make_shared<FloatRange>(FIELD_NAME, std::deque<float>{-11.1f, -11.2f},
                              std::deque<float>{1.23f, 11.5f}));
  writer->addDocument(document);

  // intersects (crosses)
  document = make_shared<Document>();
  document->push_back(
      make_shared<FloatRange>(FIELD_NAME, std::deque<float>{12.33f, 1.2f},
                              std::deque<float>{15.1f, 29.9f}));
  writer->addDocument(document);

  // disjoint
  document = make_shared<Document>();
  document->push_back(
      make_shared<FloatRange>(FIELD_NAME, std::deque<float>{-122.33f, 1.2f},
                              std::deque<float>{-115.1f, 29.9f}));
  writer->addDocument(document);

  // intersects (crosses)
  document = make_shared<Document>();
  document->push_back(make_shared<FloatRange>(
      FIELD_NAME, std::deque<float>{-numeric_limits<float>::infinity(), 1.2f},
      std::deque<float>{-11.0f, 29.9f}));
  writer->addDocument(document);

  // equal (within, contains, intersects)
  document = make_shared<Document>();
  document->push_back(
      make_shared<FloatRange>(FIELD_NAME, std::deque<float>{-11.0f, -15.0f},
                              std::deque<float>{15.0f, 20.0f}));
  writer->addDocument(document);

  // search
  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  assertEquals(7, searcher->count(FloatRange::newIntersectsQuery(
                      FIELD_NAME, std::deque<float>{-11.0f, -15.0f},
                      std::deque<float>{15.0f, 20.0f})));
  assertEquals(2, searcher->count(FloatRange::newWithinQuery(
                      FIELD_NAME, std::deque<float>{-11.0f, -15.0f},
                      std::deque<float>{15.0f, 20.0f})));
  assertEquals(2, searcher->count(FloatRange::newContainsQuery(
                      FIELD_NAME, std::deque<float>{-11.0f, -15.0f},
                      std::deque<float>{15.0f, 20.0f})));
  assertEquals(5, searcher->count(FloatRange::newCrossesQuery(
                      FIELD_NAME, std::deque<float>{-11.0f, -15.0f},
                      std::deque<float>{15.0f, 20.0f})));

  delete reader;
  delete writer;
  delete dir;
}

TestFloatRangeFieldQueries::FloatTestRange::FloatTestRange(
    shared_ptr<TestFloatRangeFieldQueries> outerInstance,
    std::deque<float> &min, std::deque<float> &max)
    : outerInstance(outerInstance)
{
  assert((min.size() > 0 && max.size() > 0 && min.size() > 0 && max.size() > 0,
          L"test box: min/max cannot be null or empty"));
  assert((min.size() == max.size(), L"test box: min/max length do not agree"));
  this->min = min;
  this->max = max;
}

int TestFloatRangeFieldQueries::FloatTestRange::numDimensions()
{
  return min.size();
}

optional<float> TestFloatRangeFieldQueries::FloatTestRange::getMin(int dim)
{
  return min[dim];
}

void TestFloatRangeFieldQueries::FloatTestRange::setMin(int dim, any val)
{
  float v = any_cast<optional<float>>(val);
  if (min[dim] < v) {
    max[dim] = v;
  } else {
    min[dim] = v;
  }
}

optional<float> TestFloatRangeFieldQueries::FloatTestRange::getMax(int dim)
{
  return max[dim];
}

void TestFloatRangeFieldQueries::FloatTestRange::setMax(int dim, any val)
{
  float v = any_cast<optional<float>>(val);
  if (max[dim] > v) {
    min[dim] = v;
  } else {
    max[dim] = v;
  }
}

bool TestFloatRangeFieldQueries::FloatTestRange::isEqual(
    shared_ptr<Range> other)
{
  shared_ptr<FloatTestRange> o =
      std::static_pointer_cast<FloatTestRange>(other);
  return Arrays::equals(min, o->min) && Arrays::equals(max, o->max);
}

bool TestFloatRangeFieldQueries::FloatTestRange::isDisjoint(shared_ptr<Range> o)
{
  shared_ptr<FloatTestRange> other =
      std::static_pointer_cast<FloatTestRange>(o);
  for (int d = 0; d < this->min.size(); ++d) {
    if (this->min[d] > other->max[d] || this->max[d] < other->min[d]) {
      // disjoint:
      return true;
    }
  }
  return false;
}

bool TestFloatRangeFieldQueries::FloatTestRange::isWithin(shared_ptr<Range> o)
{
  shared_ptr<FloatTestRange> other =
      std::static_pointer_cast<FloatTestRange>(o);
  for (int d = 0; d < this->min.size(); ++d) {
    if ((this->min[d] >= other->min[d] && this->max[d] <= other->max[d]) ==
        false) {
      // not within:
      return false;
    }
  }
  return true;
}

bool TestFloatRangeFieldQueries::FloatTestRange::contains(shared_ptr<Range> o)
{
  shared_ptr<FloatTestRange> other =
      std::static_pointer_cast<FloatTestRange>(o);
  for (int d = 0; d < this->min.size(); ++d) {
    if ((this->min[d] <= other->min[d] && this->max[d] >= other->max[d]) ==
        false) {
      // not contains:
      return false;
    }
  }
  return true;
}

wstring TestFloatRangeFieldQueries::FloatTestRange::toString()
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"Box(");
  b->append(min[0]);
  b->append(L" TO ");
  b->append(max[0]);
  for (int d = 1; d < min.size(); ++d) {
    b->append(L", ");
    b->append(min[d]);
    b->append(L" TO ");
    b->append(max[d]);
  }
  b->append(L")");

  return b->toString();
}
} // namespace org::apache::lucene::search