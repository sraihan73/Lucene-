using namespace std;

#include "FSTCompletionBuilder.h"

namespace org::apache::lucene::search::suggest::fst
{
using InMemorySorter = org::apache::lucene::search::suggest::InMemorySorter;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using namespace org::apache::lucene::util::fst;

FSTCompletionBuilder::FSTCompletionBuilder()
    : FSTCompletionBuilder(DEFAULT_BUCKETS,
                           new InMemorySorter(Comparator::naturalOrder()),
                           numeric_limits<int>::max())
{
}

FSTCompletionBuilder::FSTCompletionBuilder(int buckets,
                                           shared_ptr<BytesRefSorter> sorter,
                                           int shareMaxTailLength)
    : buckets(buckets), sorter(sorter), shareMaxTailLength(shareMaxTailLength)
{
  if (buckets < 1 || buckets > 255) {
    throw invalid_argument(L"Buckets must be >= 1 and <= 255: " +
                           to_wstring(buckets));
  }

  if (sorter == nullptr) {
    throw invalid_argument(L"BytesRefSorter must not be null.");
  }
}

void FSTCompletionBuilder::add(shared_ptr<BytesRef> utf8,
                               int bucket) 
{
  if (bucket < 0 || bucket >= buckets) {
    throw invalid_argument(L"Bucket outside of the allowed range [0, " +
                           to_wstring(buckets) + L"): " + to_wstring(bucket));
  }

  scratch->grow(utf8->length + 10);
  scratch->clear();
  scratch->append(static_cast<char>(bucket));
  scratch->append(utf8);
  sorter->add(scratch->get());
}

shared_ptr<FSTCompletion> FSTCompletionBuilder::build() 
{
  this->automaton = buildAutomaton(sorter);

  if (std::dynamic_pointer_cast<Closeable>(sorter) != nullptr) {
    (std::static_pointer_cast<Closeable>(sorter))->close();
  }

  return make_shared<FSTCompletion>(automaton);
}

shared_ptr<FST<any>> FSTCompletionBuilder::buildAutomaton(
    shared_ptr<BytesRefSorter> sorter) 
{
  // Build the automaton.
  shared_ptr<Outputs<any>> *const outputs = NoOutputs::getSingleton();
  constexpr any empty = outputs->getNoOutput();
  shared_ptr<Builder<any>> *const builder =
      make_shared<Builder<any>>(FST::INPUT_TYPE::BYTE1, 0, 0, true, true,
                                shareMaxTailLength, outputs, true, 15);

  shared_ptr<BytesRefBuilder> scratch = make_shared<BytesRefBuilder>();
  shared_ptr<BytesRef> entry;
  shared_ptr<IntsRefBuilder> *const scratchIntsRef =
      make_shared<IntsRefBuilder>();
  int count = 0;
  shared_ptr<BytesRefIterator> iter = sorter->begin();
  while ((entry = iter->next()) != nullptr) {
    count++;
    if (scratch->get()->compareTo(entry) != 0) {
      builder->add(Util::toIntsRef(entry, scratchIntsRef), empty);
      scratch->copyBytes(entry);
    }
  }

  return count == 0 ? nullptr : builder->finish();
}
} // namespace org::apache::lucene::search::suggest::fst