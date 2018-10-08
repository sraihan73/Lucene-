using namespace std;

#include "TestDoubleRangeFieldQueries.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using DoubleRange = org::apache::lucene::document::DoubleRange;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
const wstring TestDoubleRangeFieldQueries::FIELD_NAME = L"doubleRangeField";

double TestDoubleRangeFieldQueries::nextDoubleInternal()
{
  switch (random()->nextInt(5)) {
  case 0:
    return -numeric_limits<double>::infinity();
  case 1:
    return numeric_limits<double>::infinity();
  default:
    if (random()->nextBoolean()) {
      return random()->nextDouble();
    } else {
      return (random()->nextInt(15) - 7) / 3;
    }
  }
}

shared_ptr<Range>
TestDoubleRangeFieldQueries::nextRange(int dimensions) 
{
  std::deque<double> min(dimensions);
  std::deque<double> max(dimensions);

  double minV, maxV;
  for (int d = 0; d < dimensions; ++d) {
    minV = nextDoubleInternal();
    maxV = nextDoubleInternal();
    min[d] = min(minV, maxV);
    max[d] = max(minV, maxV);
  }
  return make_shared<DoubleTestRange>(shared_from_this(), min, max);
}

shared_ptr<DoubleRange>
TestDoubleRangeFieldQueries::newRangeField(shared_ptr<Range> r)
{
  return make_shared<DoubleRange>(
      FIELD_NAME, (std::static_pointer_cast<DoubleTestRange>(r))->min,
      (std::static_pointer_cast<DoubleTestRange>(r))->max);
}

shared_ptr<Query>
TestDoubleRangeFieldQueries::newIntersectsQuery(shared_ptr<Range> r)
{
  return DoubleRange::newIntersectsQuery(
      FIELD_NAME, (std::static_pointer_cast<DoubleTestRange>(r))->min,
      (std::static_pointer_cast<DoubleTestRange>(r))->max);
}

shared_ptr<Query>
TestDoubleRangeFieldQueries::newContainsQuery(shared_ptr<Range> r)
{
  return DoubleRange::newContainsQuery(
      FIELD_NAME, (std::static_pointer_cast<DoubleTestRange>(r))->min,
      (std::static_pointer_cast<DoubleTestRange>(r))->max);
}

shared_ptr<Query>
TestDoubleRangeFieldQueries::newWithinQuery(shared_ptr<Range> r)
{
  return DoubleRange::newWithinQuery(
      FIELD_NAME, (std::static_pointer_cast<DoubleTestRange>(r))->min,
      (std::static_pointer_cast<DoubleTestRange>(r))->max);
}

shared_ptr<Query>
TestDoubleRangeFieldQueries::newCrossesQuery(shared_ptr<Range> r)
{
  return DoubleRange::newCrossesQuery(
      FIELD_NAME, (std::static_pointer_cast<DoubleTestRange>(r))->min,
      (std::static_pointer_cast<DoubleTestRange>(r))->max);
}

void TestDoubleRangeFieldQueries::testBasics() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  // intersects (within)
  shared_ptr<Document> document = make_shared<Document>();
  document->push_back(
      make_shared<DoubleRange>(FIELD_NAME, std::deque<double>{-10.0, -10.0},
                               std::deque<double>{9.1, 10.1}));
  writer->addDocument(document);

  // intersects (crosses)
  document = make_shared<Document>();
  document->push_back(
      make_shared<DoubleRange>(FIELD_NAME, std::deque<double>{10.0, -10.0},
                               std::deque<double>{20.0, 10.0}));
  writer->addDocument(document);

  // intersects (contains, crosses)
  document = make_shared<Document>();
  document->push_back(
      make_shared<DoubleRange>(FIELD_NAME, std::deque<double>{-20.0, -20.0},
                               std::deque<double>{30.0, 30.1}));
  writer->addDocument(document);

  // intersects (crosses)
  document = make_shared<Document>();
  document->push_back(
      make_shared<DoubleRange>(FIELD_NAME, std::deque<double>{-11.1, -11.2},
                               std::deque<double>{1.23, 11.5}));
  writer->addDocument(document);

  // intersects (crosses)
  document = make_shared<Document>();
  document->push_back(
      make_shared<DoubleRange>(FIELD_NAME, std::deque<double>{12.33, 1.2},
                               std::deque<double>{15.1, 29.9}));
  writer->addDocument(document);

  // disjoint
  document = make_shared<Document>();
  document->push_back(
      make_shared<DoubleRange>(FIELD_NAME, std::deque<double>{-122.33, 1.2},
                               std::deque<double>{-115.1, 29.9}));
  writer->addDocument(document);

  // intersects (crosses)
  document = make_shared<Document>();
  document->push_back(make_shared<DoubleRange>(
      FIELD_NAME, std::deque<double>{-numeric_limits<double>::infinity(), 1.2},
      std::deque<double>{-11.0, 29.9}));
  writer->addDocument(document);

  // equal (within, contains, intersects)
  document = make_shared<Document>();
  document->push_back(make_shared<DoubleRange>(
      FIELD_NAME, std::deque<double>{-11, -15}, std::deque<double>{15, 20}));
  writer->addDocument(document);

  // search
  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  assertEquals(7, searcher->count(DoubleRange::newIntersectsQuery(
                      FIELD_NAME, std::deque<double>{-11.0, -15.0},
                      std::deque<double>{15.0, 20.0})));
  assertEquals(2, searcher->count(DoubleRange::newWithinQuery(
                      FIELD_NAME, std::deque<double>{-11.0, -15.0},
                      std::deque<double>{15.0, 20.0})));
  assertEquals(2, searcher->count(DoubleRange::newContainsQuery(
                      FIELD_NAME, std::deque<double>{-11.0, -15.0},
                      std::deque<double>{15.0, 20.0})));
  assertEquals(5, searcher->count(DoubleRange::newCrossesQuery(
                      FIELD_NAME, std::deque<double>{-11.0, -15.0},
                      std::deque<double>{15.0, 20.0})));

  delete reader;
  delete writer;
  delete dir;
}

TestDoubleRangeFieldQueries::DoubleTestRange::DoubleTestRange(
    shared_ptr<TestDoubleRangeFieldQueries> outerInstance,
    std::deque<double> &min, std::deque<double> &max)
    : outerInstance(outerInstance)
{
  assert((min.size() > 0 && max.size() > 0 && min.size() > 0 && max.size() > 0,
          L"test box: min/max cannot be null or empty"));
  assert((min.size() == max.size(), L"test box: min/max length do not agree"));
  this->min = min;
  this->max = max;
}

int TestDoubleRangeFieldQueries::DoubleTestRange::numDimensions()
{
  return min.size();
}

optional<double> TestDoubleRangeFieldQueries::DoubleTestRange::getMin(int dim)
{
  return min[dim];
}

void TestDoubleRangeFieldQueries::DoubleTestRange::setMin(int dim, any val)
{
  double v = any_cast<optional<double>>(val);
  if (min[dim] < v) {
    max[dim] = v;
  } else {
    min[dim] = v;
  }
}

optional<double> TestDoubleRangeFieldQueries::DoubleTestRange::getMax(int dim)
{
  return max[dim];
}

void TestDoubleRangeFieldQueries::DoubleTestRange::setMax(int dim, any val)
{
  double v = any_cast<optional<double>>(val);
  if (max[dim] > v) {
    min[dim] = v;
  } else {
    max[dim] = v;
  }
}

bool TestDoubleRangeFieldQueries::DoubleTestRange::isEqual(
    shared_ptr<Range> other)
{
  shared_ptr<DoubleTestRange> o =
      std::static_pointer_cast<DoubleTestRange>(other);
  return Arrays::equals(min, o->min) && Arrays::equals(max, o->max);
}

bool TestDoubleRangeFieldQueries::DoubleTestRange::isDisjoint(
    shared_ptr<Range> o)
{
  shared_ptr<DoubleTestRange> other =
      std::static_pointer_cast<DoubleTestRange>(o);
  for (int d = 0; d < this->min.size(); ++d) {
    if (this->min[d] > other->max[d] || this->max[d] < other->min[d]) {
      // disjoint:
      return true;
    }
  }
  return false;
}

bool TestDoubleRangeFieldQueries::DoubleTestRange::isWithin(shared_ptr<Range> o)
{
  shared_ptr<DoubleTestRange> other =
      std::static_pointer_cast<DoubleTestRange>(o);
  for (int d = 0; d < this->min.size(); ++d) {
    if ((this->min[d] >= other->min[d] && this->max[d] <= other->max[d]) ==
        false) {
      // not within:
      return false;
    }
  }
  return true;
}

bool TestDoubleRangeFieldQueries::DoubleTestRange::contains(shared_ptr<Range> o)
{
  shared_ptr<DoubleTestRange> other =
      std::static_pointer_cast<DoubleTestRange>(o);
  for (int d = 0; d < this->min.size(); ++d) {
    if ((this->min[d] <= other->min[d] && this->max[d] >= other->max[d]) ==
        false) {
      // not contains:
      return false;
    }
  }
  return true;
}

wstring TestDoubleRangeFieldQueries::DoubleTestRange::toString()
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