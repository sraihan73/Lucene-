using namespace std;

#include "MultiLeafFieldComparator.h"

namespace org::apache::lucene::search
{

MultiLeafFieldComparator::MultiLeafFieldComparator(
    std::deque<std::shared_ptr<LeafFieldComparator>> &comparators,
    std::deque<int> &reverseMul)
    : comparators(comparators), reverseMul(reverseMul),
      firstComparator(comparators[0]), firstReverseMul(reverseMul[0])
{
  if (comparators.size() != reverseMul.size()) {
    throw invalid_argument(
        L"Must have the same number of comparators and reverseMul, got " +
        comparators.size() + L" and " + reverseMul.size());
  }
}

void MultiLeafFieldComparator::setBottom(int slot) 
{
  for (auto comparator : comparators) {
    comparator->setBottom(slot);
  }
}

int MultiLeafFieldComparator::compareBottom(int doc) 
{
  int cmp = firstReverseMul * firstComparator->compareBottom(doc);
  if (cmp != 0) {
    return cmp;
  }
  for (int i = 1; i < comparators.size(); ++i) {
    cmp = reverseMul[i] * comparators[i]->compareBottom(doc);
    if (cmp != 0) {
      return cmp;
    }
  }
  return 0;
}

int MultiLeafFieldComparator::compareTop(int doc) 
{
  int cmp = firstReverseMul * firstComparator->compareTop(doc);
  if (cmp != 0) {
    return cmp;
  }
  for (int i = 1; i < comparators.size(); ++i) {
    cmp = reverseMul[i] * comparators[i]->compareTop(doc);
    if (cmp != 0) {
      return cmp;
    }
  }
  return 0;
}

void MultiLeafFieldComparator::copy(int slot, int doc) 
{
  for (auto comparator : comparators) {
    comparator->copy(slot, doc);
  }
}

void MultiLeafFieldComparator::setScorer(shared_ptr<Scorer> scorer) throw(
    IOException)
{
  for (auto comparator : comparators) {
    comparator->setScorer(scorer);
  }
}
} // namespace org::apache::lucene::search