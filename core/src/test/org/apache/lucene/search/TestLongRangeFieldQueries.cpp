using namespace std;

#include "TestLongRangeFieldQueries.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using LongRange = org::apache::lucene::document::LongRange;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
using TestUtil = org::apache::lucene::util::TestUtil;
const wstring TestLongRangeFieldQueries::FIELD_NAME = L"longRangeField";

int64_t TestLongRangeFieldQueries::nextLongInternal()
{
  switch (random()->nextInt(5)) {
  case 0:
    return numeric_limits<int64_t>::min();
  case 1:
    return numeric_limits<int64_t>::max();
  default:
    int bpv = random()->nextInt(64);
    switch (bpv) {
    case 64:
      return random()->nextLong();
    default:
      int64_t v = TestUtil::nextLong(random(), 0, (1LL << bpv) - 1);
      if (bpv > 0) {
        // negative values sometimes
        v -= 1LL << (bpv - 1);
      }
      return v;
    }
  }
}

shared_ptr<Range>
TestLongRangeFieldQueries::nextRange(int dimensions) 
{
  std::deque<int64_t> min(dimensions);
  std::deque<int64_t> max(dimensions);

  int64_t minV, maxV;
  for (int d = 0; d < dimensions; ++d) {
    minV = nextLongInternal();
    maxV = nextLongInternal();
    min[d] = min(minV, maxV);
    max[d] = max(minV, maxV);
  }
  return make_shared<LongTestRange>(shared_from_this(), min, max);
}

shared_ptr<LongRange>
TestLongRangeFieldQueries::newRangeField(shared_ptr<Range> r)
{
  return make_shared<LongRange>(
      FIELD_NAME, (std::static_pointer_cast<LongTestRange>(r))->min,
      (std::static_pointer_cast<LongTestRange>(r))->max);
}

shared_ptr<Query>
TestLongRangeFieldQueries::newIntersectsQuery(shared_ptr<Range> r)
{
  return LongRange::newIntersectsQuery(
      FIELD_NAME, (std::static_pointer_cast<LongTestRange>(r))->min,
      (std::static_pointer_cast<LongTestRange>(r))->max);
}

shared_ptr<Query>
TestLongRangeFieldQueries::newContainsQuery(shared_ptr<Range> r)
{
  return LongRange::newContainsQuery(
      FIELD_NAME, (std::static_pointer_cast<LongTestRange>(r))->min,
      (std::static_pointer_cast<LongTestRange>(r))->max);
}

shared_ptr<Query> TestLongRangeFieldQueries::newWithinQuery(shared_ptr<Range> r)
{
  return LongRange::newWithinQuery(
      FIELD_NAME, (std::static_pointer_cast<LongTestRange>(r))->min,
      (std::static_pointer_cast<LongTestRange>(r))->max);
}

shared_ptr<Query>
TestLongRangeFieldQueries::newCrossesQuery(shared_ptr<Range> r)
{
  return LongRange::newCrossesQuery(
      FIELD_NAME, (std::static_pointer_cast<LongTestRange>(r))->min,
      (std::static_pointer_cast<LongTestRange>(r))->max);
}

void TestLongRangeFieldQueries::testBasics() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  // intersects (within)
  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(make_shared<LongRange>(FIELD_NAME,
                                             std::deque<int64_t>{-10, -10},
                                             std::deque<int64_t>{9, 10}));
  writer->addDocument(document);

  // intersects (crosses)
  document = make_shared<Document>();
  document->push_back(make_shared<LongRange>(FIELD_NAME,
                                             std::deque<int64_t>{10, -10},
                                             std::deque<int64_t>{20, 10}));
  writer->addDocument(document);

  // intersects (contains, crosses)
  document = make_shared<Document>();
  document->push_back(make_shared<LongRange>(FIELD_NAME,
                                             std::deque<int64_t>{-20, -20},
                                             std::deque<int64_t>{30, 30}));
  writer->addDocument(document);

  // intersects (within)
  document = make_shared<Document>();
  document->push_back(make_shared<LongRange>(FIELD_NAME,
                                             std::deque<int64_t>{-11, -11},
                                             std::deque<int64_t>{1, 11}));
  writer->addDocument(document);

  // intersects (crosses)
  document = make_shared<Document>();
  document->push_back(make_shared<LongRange>(FIELD_NAME,
                                             std::deque<int64_t>{12, 1},
                                             std::deque<int64_t>{15, 29}));
  writer->addDocument(document);

  // disjoint
  document = make_shared<Document>();
  document->push_back(make_shared<LongRange>(FIELD_NAME,
                                             std::deque<int64_t>{-122, 1},
                                             std::deque<int64_t>{-115, 29}));
  writer->addDocument(document);

  // intersects (crosses)
  document = make_shared<Document>();
  document->push_back(make_shared<LongRange>(
      FIELD_NAME, std::deque<int64_t>{numeric_limits<int64_t>::min(), 1},
      std::deque<int64_t>{-11, 29}));
  writer->addDocument(document);

  // equal (within, contains, intersects)
  document = make_shared<Document>();
  document->push_back(make_shared<LongRange>(FIELD_NAME,
                                             std::deque<int64_t>{-11, -15},
                                             std::deque<int64_t>{15, 20}));
  writer->addDocument(document);

  // search
  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  TestUtil::assertEquals(7, searcher->count(LongRange::newIntersectsQuery(
                                FIELD_NAME, std::deque<int64_t>{-11, -15},
                                std::deque<int64_t>{15, 20})));
  TestUtil::assertEquals(3, searcher->count(LongRange::newWithinQuery(
                                FIELD_NAME, std::deque<int64_t>{-11, -15},
                                std::deque<int64_t>{15, 20})));
  TestUtil::assertEquals(2, searcher->count(LongRange::newContainsQuery(
                                FIELD_NAME, std::deque<int64_t>{-11, -15},
                                std::deque<int64_t>{15, 20})));
  TestUtil::assertEquals(4, searcher->count(LongRange::newCrossesQuery(
                                FIELD_NAME, std::deque<int64_t>{-11, -15},
                                std::deque<int64_t>{15, 20})));

  delete reader;
  delete writer;
  delete dir;
}

TestLongRangeFieldQueries::LongTestRange::LongTestRange(
    shared_ptr<TestLongRangeFieldQueries> outerInstance,
    std::deque<int64_t> &min, std::deque<int64_t> &max)
    : outerInstance(outerInstance)
{
  assert((min.size() > 0 && max.size() > 0 && min.size() > 0 && max.size() > 0,
          L"test box: min/max cannot be null or empty"));
  assert((min.size() == max.size(), L"test box: min/max length do not agree"));
  this->min = min;
  this->max = max;
}

int TestLongRangeFieldQueries::LongTestRange::numDimensions()
{
  return min.size();
}

optional<int64_t> TestLongRangeFieldQueries::LongTestRange::getMin(int dim)
{
  return min[dim];
}

void TestLongRangeFieldQueries::LongTestRange::setMin(int dim, any val)
{
  int64_t v = any_cast<optional<int64_t>>(val);
  if (min[dim] < v) {
    max[dim] = v;
  } else {
    min[dim] = v;
  }
}

optional<int64_t> TestLongRangeFieldQueries::LongTestRange::getMax(int dim)
{
  return max[dim];
}

void TestLongRangeFieldQueries::LongTestRange::setMax(int dim, any val)
{
  int64_t v = any_cast<optional<int64_t>>(val);
  if (max[dim] > v) {
    min[dim] = v;
  } else {
    max[dim] = v;
  }
}

bool TestLongRangeFieldQueries::LongTestRange::isEqual(shared_ptr<Range> other)
{
  shared_ptr<LongTestRange> o = std::static_pointer_cast<LongTestRange>(other);
  return Arrays::equals(min, o->min) && Arrays::equals(max, o->max);
}

bool TestLongRangeFieldQueries::LongTestRange::isDisjoint(shared_ptr<Range> o)
{
  shared_ptr<LongTestRange> other = std::static_pointer_cast<LongTestRange>(o);
  for (int d = 0; d < this->min.size(); ++d) {
    if (this->min[d] > other->max[d] || this->max[d] < other->min[d]) {
      // disjoint:
      return true;
    }
  }
  return false;
}

bool TestLongRangeFieldQueries::LongTestRange::isWithin(shared_ptr<Range> o)
{
  shared_ptr<LongTestRange> other = std::static_pointer_cast<LongTestRange>(o);
  for (int d = 0; d < this->min.size(); ++d) {
    if ((this->min[d] >= other->min[d] && this->max[d] <= other->max[d]) ==
        false) {
      // not within:
      return false;
    }
  }
  return true;
}

bool TestLongRangeFieldQueries::LongTestRange::contains(shared_ptr<Range> o)
{
  shared_ptr<LongTestRange> other = std::static_pointer_cast<LongTestRange>(o);
  for (int d = 0; d < this->min.size(); ++d) {
    if ((this->min[d] <= other->min[d] && this->max[d] >= other->max[d]) ==
        false) {
      // not contains:
      return false;
    }
  }
  return true;
}

wstring TestLongRangeFieldQueries::LongTestRange::toString()
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