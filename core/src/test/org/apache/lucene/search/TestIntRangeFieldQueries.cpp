using namespace std;

#include "TestIntRangeFieldQueries.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using IntRange = org::apache::lucene::document::IntRange;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
using TestUtil = org::apache::lucene::util::TestUtil;
const wstring TestIntRangeFieldQueries::FIELD_NAME = L"intRangeField";

int TestIntRangeFieldQueries::nextIntInternal()
{
  switch (random()->nextInt(5)) {
  case 0:
    return numeric_limits<int>::min();
  case 1:
    return numeric_limits<int>::max();
  default:
    int bpv = random()->nextInt(32);
    switch (bpv) {
    case 32:
      return random()->nextInt();
    default:
      int v = TestUtil::nextInt(random(), 0, (1 << bpv) - 1);
      if (bpv > 0) {
        // negative values sometimes
        v -= 1 << (bpv - 1);
      }
      return v;
    }
  }
}

shared_ptr<Range>
TestIntRangeFieldQueries::nextRange(int dimensions) 
{
  std::deque<int> min(dimensions);
  std::deque<int> max(dimensions);

  int minV, maxV;
  for (int d = 0; d < dimensions; ++d) {
    minV = nextIntInternal();
    maxV = nextIntInternal();
    min[d] = min(minV, maxV);
    max[d] = max(minV, maxV);
  }
  return make_shared<IntTestRange>(shared_from_this(), min, max);
}

shared_ptr<IntRange>
TestIntRangeFieldQueries::newRangeField(shared_ptr<Range> r)
{
  return make_shared<IntRange>(
      FIELD_NAME, (std::static_pointer_cast<IntTestRange>(r))->min,
      (std::static_pointer_cast<IntTestRange>(r))->max);
}

shared_ptr<Query>
TestIntRangeFieldQueries::newIntersectsQuery(shared_ptr<Range> r)
{
  return IntRange::newIntersectsQuery(
      FIELD_NAME, (std::static_pointer_cast<IntTestRange>(r))->min,
      (std::static_pointer_cast<IntTestRange>(r))->max);
}

shared_ptr<Query>
TestIntRangeFieldQueries::newContainsQuery(shared_ptr<Range> r)
{
  return IntRange::newContainsQuery(
      FIELD_NAME, (std::static_pointer_cast<IntTestRange>(r))->min,
      (std::static_pointer_cast<IntTestRange>(r))->max);
}

shared_ptr<Query> TestIntRangeFieldQueries::newWithinQuery(shared_ptr<Range> r)
{
  return IntRange::newWithinQuery(
      FIELD_NAME, (std::static_pointer_cast<IntTestRange>(r))->min,
      (std::static_pointer_cast<IntTestRange>(r))->max);
}

shared_ptr<Query> TestIntRangeFieldQueries::newCrossesQuery(shared_ptr<Range> r)
{
  return IntRange::newCrossesQuery(
      FIELD_NAME, (std::static_pointer_cast<IntTestRange>(r))->min,
      (std::static_pointer_cast<IntTestRange>(r))->max);
}

void TestIntRangeFieldQueries::testBasics() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  // intersects (within)
  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(make_shared<IntRange>(
      FIELD_NAME, std::deque<int>{-10, -10}, std::deque<int>{9, 10}));
  writer->addDocument(document);

  // intersects (crosses)
  document = make_shared<Document>();
  document->push_back(make_shared<IntRange>(
      FIELD_NAME, std::deque<int>{10, -10}, std::deque<int>{20, 10}));
  writer->addDocument(document);

  // intersects (contains / crosses)
  document = make_shared<Document>();
  document->push_back(make_shared<IntRange>(
      FIELD_NAME, std::deque<int>{-20, -20}, std::deque<int>{30, 30}));
  writer->addDocument(document);

  // intersects (within)
  document = make_shared<Document>();
  document->push_back(make_shared<IntRange>(
      FIELD_NAME, std::deque<int>{-11, -11}, std::deque<int>{1, 11}));
  writer->addDocument(document);

  // intersects (crosses)
  document = make_shared<Document>();
  document->push_back(make_shared<IntRange>(FIELD_NAME, std::deque<int>{12, 1},
                                            std::deque<int>{15, 29}));
  writer->addDocument(document);

  // disjoint
  document = make_shared<Document>();
  document->push_back(make_shared<IntRange>(
      FIELD_NAME, std::deque<int>{-122, 1}, std::deque<int>{-115, 29}));
  writer->addDocument(document);

  // intersects (crosses)
  document = make_shared<Document>();
  document->push_back(make_shared<IntRange>(
      FIELD_NAME, std::deque<int>{numeric_limits<int>::min(), 1},
      std::deque<int>{-11, 29}));
  writer->addDocument(document);

  // equal (within, contains, intersects)
  document = make_shared<Document>();
  document->push_back(make_shared<IntRange>(
      FIELD_NAME, std::deque<int>{-11, -15}, std::deque<int>{15, 20}));
  writer->addDocument(document);

  // search
  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  TestUtil::assertEquals(7, searcher->count(IntRange::newIntersectsQuery(
                                FIELD_NAME, std::deque<int>{-11, -15},
                                std::deque<int>{15, 20})));
  TestUtil::assertEquals(3, searcher->count(IntRange::newWithinQuery(
                                FIELD_NAME, std::deque<int>{-11, -15},
                                std::deque<int>{15, 20})));
  TestUtil::assertEquals(2, searcher->count(IntRange::newContainsQuery(
                                FIELD_NAME, std::deque<int>{-11, -15},
                                std::deque<int>{15, 20})));
  TestUtil::assertEquals(4, searcher->count(IntRange::newCrossesQuery(
                                FIELD_NAME, std::deque<int>{-11, -15},
                                std::deque<int>{15, 20})));

  delete reader;
  delete writer;
  delete dir;
}

TestIntRangeFieldQueries::IntTestRange::IntTestRange(
    shared_ptr<TestIntRangeFieldQueries> outerInstance, std::deque<int> &min,
    std::deque<int> &max)
    : outerInstance(outerInstance)
{
  assert((min.size() > 0 && max.size() > 0 && min.size() > 0 && max.size() > 0,
          L"test box: min/max cannot be null or empty"));
  assert((min.size() == max.size(), L"test box: min/max length do not agree"));
  this->min = min;
  this->max = max;
}

int TestIntRangeFieldQueries::IntTestRange::numDimensions()
{
  return min.size();
}

optional<int> TestIntRangeFieldQueries::IntTestRange::getMin(int dim)
{
  return min[dim];
}

void TestIntRangeFieldQueries::IntTestRange::setMin(int dim, any val)
{
  int v = any_cast<optional<int>>(val);
  if (min[dim] < v) {
    max[dim] = v;
  } else {
    min[dim] = v;
  }
}

optional<int> TestIntRangeFieldQueries::IntTestRange::getMax(int dim)
{
  return max[dim];
}

void TestIntRangeFieldQueries::IntTestRange::setMax(int dim, any val)
{
  int v = any_cast<optional<int>>(val);
  if (max[dim] > v) {
    min[dim] = v;
  } else {
    max[dim] = v;
  }
}

bool TestIntRangeFieldQueries::IntTestRange::isEqual(shared_ptr<Range> other)
{
  shared_ptr<IntTestRange> o = std::static_pointer_cast<IntTestRange>(other);
  return Arrays::equals(min, o->min) && Arrays::equals(max, o->max);
}

bool TestIntRangeFieldQueries::IntTestRange::isDisjoint(shared_ptr<Range> o)
{
  shared_ptr<IntTestRange> other = std::static_pointer_cast<IntTestRange>(o);
  for (int d = 0; d < this->min.size(); ++d) {
    if (this->min[d] > other->max[d] || this->max[d] < other->min[d]) {
      // disjoint:
      return true;
    }
  }
  return false;
}

bool TestIntRangeFieldQueries::IntTestRange::isWithin(shared_ptr<Range> o)
{
  shared_ptr<IntTestRange> other = std::static_pointer_cast<IntTestRange>(o);
  for (int d = 0; d < this->min.size(); ++d) {
    if ((this->min[d] >= other->min[d] && this->max[d] <= other->max[d]) ==
        false) {
      // not within:
      return false;
    }
  }
  return true;
}

bool TestIntRangeFieldQueries::IntTestRange::contains(shared_ptr<Range> o)
{
  shared_ptr<IntTestRange> other = std::static_pointer_cast<IntTestRange>(o);
  for (int d = 0; d < this->min.size(); ++d) {
    if ((this->min[d] <= other->min[d] && this->max[d] >= other->max[d]) ==
        false) {
      // not contains:
      return false;
    }
  }
  return true;
}

wstring TestIntRangeFieldQueries::IntTestRange::toString()
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