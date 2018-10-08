using namespace std;

#include "AssertingBulkScorer.h"

namespace org::apache::lucene::search
{
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Bits = org::apache::lucene::util::Bits;
using com::carrotsearch::randomizedtesting::generators::RandomNumbers;

shared_ptr<BulkScorer> AssertingBulkScorer::wrap(shared_ptr<Random> random,
                                                 shared_ptr<BulkScorer> other,
                                                 int maxDoc)
{
  if (other == nullptr ||
      std::dynamic_pointer_cast<AssertingBulkScorer>(other) != nullptr) {
    return other;
  }
  return make_shared<AssertingBulkScorer>(random, other, maxDoc);
}

AssertingBulkScorer::AssertingBulkScorer(shared_ptr<Random> random,
                                         shared_ptr<BulkScorer> in_, int maxDoc)
    : random(random), in_(in_), maxDoc(maxDoc)
{
}

shared_ptr<BulkScorer> AssertingBulkScorer::getIn() { return in_; }

int64_t AssertingBulkScorer::cost() { return in_->cost(); }

void AssertingBulkScorer::score(shared_ptr<LeafCollector> collector,
                                shared_ptr<Bits> acceptDocs) 
{
  assert(max == 0);
  collector = make_shared<AssertingLeafCollector>(random, collector, 0,
                                                  PostingsEnum::NO_MORE_DOCS);
  if (random->nextBoolean()) {
    try {
      constexpr int next =
          score(collector, acceptDocs, 0, PostingsEnum::NO_MORE_DOCS);
      assert(next == DocIdSetIterator::NO_MORE_DOCS);
    } catch (const UnsupportedOperationException &e) {
      in_->score(collector, acceptDocs);
    }
  } else {
    in_->score(collector, acceptDocs);
  }
}

int AssertingBulkScorer::score(shared_ptr<LeafCollector> collector,
                               shared_ptr<Bits> acceptDocs, int min,
                               int const max) 
{
  assert((min >= this->max, L"Scoring backward: min=" + to_wstring(min) +
                                L" while previous max was max=" +
                                to_wstring(this->max)));
  assert((min <= max, L"max must be greater than min, got min=" +
                          to_wstring(min) + L", and max=" + to_wstring(max)));
  this->max = max;
  collector = make_shared<AssertingLeafCollector>(random, collector, min, max);
  constexpr int next = in_->score(collector, acceptDocs, min, max);
  assert(next >= max);
  if (max >= maxDoc || next >= maxDoc) {
    assert(next == DocIdSetIterator::NO_MORE_DOCS);
    return DocIdSetIterator::NO_MORE_DOCS;
  } else {
    return RandomNumbers::randomIntBetween(random, max, next);
  }
}

wstring AssertingBulkScorer::toString()
{
  return L"AssertingBulkScorer(" + in_ + L")";
}
} // namespace org::apache::lucene::search