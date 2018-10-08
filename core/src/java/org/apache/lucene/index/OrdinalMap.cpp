using namespace std;

#include "OrdinalMap.h"

namespace org::apache::lucene::index
{
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using InPlaceMergeSorter = org::apache::lucene::util::InPlaceMergeSorter;
using LongValues = org::apache::lucene::util::LongValues;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
using PackedLongValues = org::apache::lucene::util::packed::PackedLongValues;
std::deque<std::shared_ptr<TermsEnumIndex>> const
    OrdinalMap::TermsEnumIndex::EMPTY_ARRAY =
        std::deque<std::shared_ptr<TermsEnumIndex>>(0);

OrdinalMap::TermsEnumIndex::TermsEnumIndex(shared_ptr<TermsEnum> termsEnum,
                                           int subIndex)
    : subIndex(subIndex), termsEnum(termsEnum)
{
}

shared_ptr<BytesRef> OrdinalMap::TermsEnumIndex::next() 
{
  currentTerm = termsEnum->next();
  return currentTerm;
}

std::deque<int> OrdinalMap::SegmentMap::map_obj(std::deque<int64_t> &weights)
{
  const std::deque<int> newToOld = std::deque<int>(weights.size());
  for (int i = 0; i < weights.size(); ++i) {
    newToOld[i] = i;
  }
  make_shared<InPlaceMergeSorterAnonymousInnerClass>(weights, newToOld)
      .sort(0, weights.size());
  return newToOld;
}

OrdinalMap::SegmentMap::InPlaceMergeSorterAnonymousInnerClass::
    InPlaceMergeSorterAnonymousInnerClass(deque<int64_t> &weights,
                                          deque<int> &newToOld)
{
  this->weights = weights;
  this->newToOld = newToOld;
}

void OrdinalMap::SegmentMap::InPlaceMergeSorterAnonymousInnerClass::swap(int i,
                                                                         int j)
{
  constexpr int tmp = newToOld[i];
  newToOld[i] = newToOld[j];
  newToOld[j] = tmp;
}

int OrdinalMap::SegmentMap::InPlaceMergeSorterAnonymousInnerClass::compare(
    int i, int j)
{
  // j first since we actually want higher weights first
  return Long::compare(weights[newToOld[j]], weights[newToOld[i]]);
}

std::deque<int> OrdinalMap::SegmentMap::inverse(std::deque<int> &map_obj)
{
  const std::deque<int> inverse = std::deque<int>(map_obj.size());
  for (int i = 0; i < map_obj.size(); ++i) {
    inverse[map_obj[i]] = i;
  }
  return inverse;
}

OrdinalMap::SegmentMap::SegmentMap(std::deque<int64_t> &weights)
    : newToOld(map_obj(weights)), oldToNew(inverse(newToOld_))
{
  assert((Arrays::equals(newToOld_, inverse(oldToNew_))));
}

int OrdinalMap::SegmentMap::newToOld(int segment) { return newToOld_[segment]; }

int OrdinalMap::SegmentMap::oldToNew(int segment) { return oldToNew_[segment]; }

int64_t OrdinalMap::SegmentMap::ramBytesUsed()
{
  return BASE_RAM_BYTES_USED + RamUsageEstimator::sizeOf(newToOld_) +
         RamUsageEstimator::sizeOf(oldToNew_);
}

shared_ptr<OrdinalMap>
OrdinalMap::build(shared_ptr<IndexReader::CacheKey> owner,
                  std::deque<std::shared_ptr<SortedDocValues>> &values,
                  float acceptableOverheadRatio) 
{
  std::deque<std::shared_ptr<TermsEnum>> subs(values.size());
  const std::deque<int64_t> weights = std::deque<int64_t>(values.size());
  for (int i = 0; i < values.size(); ++i) {
    subs[i] = values[i]->termsEnum();
    weights[i] = values[i]->getValueCount();
  }
  return build(owner, subs, weights, acceptableOverheadRatio);
}

shared_ptr<OrdinalMap>
OrdinalMap::build(shared_ptr<IndexReader::CacheKey> owner,
                  std::deque<std::shared_ptr<SortedSetDocValues>> &values,
                  float acceptableOverheadRatio) 
{
  std::deque<std::shared_ptr<TermsEnum>> subs(values.size());
  const std::deque<int64_t> weights = std::deque<int64_t>(values.size());
  for (int i = 0; i < values.size(); ++i) {
    subs[i] = values[i]->termsEnum();
    weights[i] = values[i]->getValueCount();
  }
  return build(owner, subs, weights, acceptableOverheadRatio);
}

shared_ptr<OrdinalMap>
OrdinalMap::build(shared_ptr<IndexReader::CacheKey> owner,
                  std::deque<std::shared_ptr<TermsEnum>> &subs,
                  std::deque<int64_t> &weights,
                  float acceptableOverheadRatio) 
{
  if (subs.size() != weights.size()) {
    throw invalid_argument(L"subs and weights must have the same length");
  }

  // enums are not sorted, so let's sort to save memory
  shared_ptr<SegmentMap> *const segmentMap = make_shared<SegmentMap>(weights);
  return make_shared<OrdinalMap>(owner, subs, segmentMap,
                                 acceptableOverheadRatio);
}

OrdinalMap::OrdinalMap(shared_ptr<IndexReader::CacheKey> owner,
                       std::deque<std::shared_ptr<TermsEnum>> &subs,
                       shared_ptr<SegmentMap> segmentMap,
                       float acceptableOverheadRatio) 
    : owner(owner), globalOrdDeltas(globalOrdDeltas->build()),
      firstSegments(firstSegments->build()),
      segmentToGlobalOrds(
          std::deque<std::shared_ptr<LongValues>>(subs.size())),
      segmentMap(segmentMap), ramBytesUsed(ramBytesUsed_)
{
  // create the ordinal mappings by pulling a termsenum over each sub's
  // unique terms, and walking a multitermsenum over those
  // even though we accept an overhead ratio, we keep these ones with COMPACT
  // since they are only used to resolve values given a global ord, which is
  // slow anyway
  shared_ptr<PackedLongValues::Builder> globalOrdDeltas =
      PackedLongValues::monotonicBuilder(PackedInts::COMPACT);
  shared_ptr<PackedLongValues::Builder> firstSegments =
      PackedLongValues::packedBuilder(PackedInts::COMPACT);
  std::deque<std::shared_ptr<PackedLongValues::Builder>> ordDeltas(
      subs.size());
  for (int i = 0; i < ordDeltas.size(); i++) {
    ordDeltas[i] = PackedLongValues::monotonicBuilder(acceptableOverheadRatio);
  }
  std::deque<int64_t> ordDeltaBits(subs.size());
  std::deque<int64_t> segmentOrds(subs.size());

  // Just merge-sorts by term:
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  shared_ptr<PriorityQueue<std::shared_ptr<TermsEnumIndex>>> queue =
      make_shared<PriorityQueueAnonymousInnerClass>(shared_from_this(),
                                                    subs.size());

  for (int i = 0; i < subs.size(); i++) {
    shared_ptr<TermsEnumIndex> sub =
        make_shared<TermsEnumIndex>(subs[segmentMap->newToOld(i)], i);
    if (sub->next() != nullptr) {
      queue->push_back(sub);
    }
  }

  shared_ptr<BytesRefBuilder> scratch = make_shared<BytesRefBuilder>();

  int64_t globalOrd = 0;
  while (queue->size() != 0) {
    shared_ptr<TermsEnumIndex> top = queue->top();
    scratch->copyBytes(top->currentTerm);

    int firstSegmentIndex = numeric_limits<int>::max();
    int64_t globalOrdDelta = numeric_limits<int64_t>::max();

    // Advance past this term, recording the per-segment ord deltas:
    while (true) {
      top = queue->top();
      int64_t segmentOrd = top->termsEnum->ord();
      int64_t delta = globalOrd - segmentOrd;
      int segmentIndex = top->subIndex;
      // We compute the least segment where the term occurs. In case the
      // first segment contains most (or better all) values, this will
      // help save significant memory
      if (segmentIndex < firstSegmentIndex) {
        firstSegmentIndex = segmentIndex;
        globalOrdDelta = delta;
      }
      ordDeltaBits[segmentIndex] |= delta;

      // for each per-segment ord, map_obj it back to the global term; the while
      // loop is needed in case the incoming TermsEnums don't have compact
      // ordinals (some ordinal values are skipped), which can happen e.g. with
      // a FilteredTermsEnum:
      assert(segmentOrds[segmentIndex] <= segmentOrd);

      // TODO: we could specialize this case (the while loop is not needed when
      // the ords are compact)
      do {
        ordDeltas[segmentIndex]->add(delta);
        segmentOrds[segmentIndex]++;
      } while (segmentOrds[segmentIndex] <= segmentOrd);

      if (top->next() == nullptr) {
        queue->pop();
        if (queue->empty()) {
          break;
        }
      } else {
        queue->updateTop();
      }
      if (queue->top()->currentTerm->equals(scratch->get()) == false) {
        break;
      }
    }

    // for each unique term, just mark the first segment index/delta where it
    // occurs
    firstSegments->add(firstSegmentIndex);
    globalOrdDeltas->add(globalOrdDelta);
    globalOrd++;
  }

  // ordDeltas is typically the bottleneck, so let's see what we can do to make
  // it faster
  int64_t ramBytesUsed =
      BASE_RAM_BYTES_USED + this->globalOrdDeltas->ramBytesUsed() +
      this->firstSegments->ramBytesUsed() +
      RamUsageEstimator::shallowSizeOf(segmentToGlobalOrds) +
      segmentMap->ramBytesUsed();
  for (int i = 0; i < ordDeltas.size(); ++i) {
    shared_ptr<PackedLongValues> *const deltas = ordDeltas[i]->build();
    if (ordDeltaBits[i] == 0LL) {
      // segment ords perfectly match global ordinals
      // likely in case of low cardinalities and large segments
      segmentToGlobalOrds[i] = LongValues::IDENTITY;
    } else {
      constexpr int bitsRequired =
          ordDeltaBits[i] < 0 ? 64 : PackedInts::bitsRequired(ordDeltaBits[i]);
      constexpr int64_t monotonicBits = deltas->ramBytesUsed() * 8;
      constexpr int64_t packedBits = bitsRequired * deltas->size();
      if (deltas->size() <= numeric_limits<int>::max() &&
          packedBits <= monotonicBits * (1 + acceptableOverheadRatio)) {
        // monotonic compression mostly adds overhead, let's keep the mapping in
        // plain packed ints
        constexpr int size = static_cast<int>(deltas->size());
        shared_ptr<PackedInts::Mutable> *const newDeltas =
            PackedInts::getMutable(size, bitsRequired, acceptableOverheadRatio);
        shared_ptr<PackedLongValues::Iterator> *const it = deltas->begin();
        for (int ord = 0; ord < size; ++ord) {
          newDeltas->set(ord, it->next());
        }
        assert(it->hasNext() == false);
        // C++ TODO: You cannot use 'shared_from_this' in a constructor:
        segmentToGlobalOrds[i] = make_shared<LongValuesAnonymousInnerClass>(
            shared_from_this(), newDeltas);
        ramBytesUsed += newDeltas->ramBytesUsed();
      } else {
        // C++ TODO: You cannot use 'shared_from_this' in a constructor:
        segmentToGlobalOrds[i] = make_shared<LongValuesAnonymousInnerClass2>(
            shared_from_this(), deltas);
        ramBytesUsed += deltas->ramBytesUsed();
      }
      ramBytesUsed += RamUsageEstimator::shallowSizeOf(segmentToGlobalOrds[i]);
    }
  }
}

OrdinalMap::PriorityQueueAnonymousInnerClass::PriorityQueueAnonymousInnerClass(
    shared_ptr<OrdinalMap> outerInstance, shared_ptr<UnknownType> length)
    : org::apache::lucene::util::PriorityQueue<TermsEnumIndex>(length)
{
  this->outerInstance = outerInstance;
}

bool OrdinalMap::PriorityQueueAnonymousInnerClass::lessThan(
    shared_ptr<TermsEnumIndex> a, shared_ptr<TermsEnumIndex> b)
{
  return a->currentTerm->compareTo(b->currentTerm) < 0;
}

OrdinalMap::LongValuesAnonymousInnerClass::LongValuesAnonymousInnerClass(
    shared_ptr<OrdinalMap> outerInstance,
    shared_ptr<PackedInts::Mutable> newDeltas)
{
  this->outerInstance = outerInstance;
  this->newDeltas = newDeltas;
}

int64_t OrdinalMap::LongValuesAnonymousInnerClass::get(int64_t ord)
{
  return ord + newDeltas->get(static_cast<int>(ord));
}

OrdinalMap::LongValuesAnonymousInnerClass2::LongValuesAnonymousInnerClass2(
    shared_ptr<OrdinalMap> outerInstance, shared_ptr<PackedLongValues> deltas)
{
  this->outerInstance = outerInstance;
  this->deltas = deltas;
}

int64_t OrdinalMap::LongValuesAnonymousInnerClass2::get(int64_t ord)
{
  return ord + deltas->get(ord);
}

shared_ptr<LongValues> OrdinalMap::getGlobalOrds(int segmentIndex)
{
  return segmentToGlobalOrds[segmentMap->oldToNew(segmentIndex)];
}

int64_t OrdinalMap::getFirstSegmentOrd(int64_t globalOrd)
{
  return globalOrd - globalOrdDeltas->get(globalOrd);
}

int OrdinalMap::getFirstSegmentNumber(int64_t globalOrd)
{
  return segmentMap->newToOld(static_cast<int>(firstSegments->get(globalOrd)));
}

int64_t OrdinalMap::getValueCount() { return globalOrdDeltas->size(); }

int64_t OrdinalMap::ramBytesUsed() { return ramBytesUsed_; }

shared_ptr<deque<std::shared_ptr<Accountable>>>
OrdinalMap::getChildResources()
{
  deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>();
  resources.push_back(
      Accountables::namedAccountable(L"global ord deltas", globalOrdDeltas));
  resources.push_back(
      Accountables::namedAccountable(L"first segments", firstSegments));
  resources.push_back(
      Accountables::namedAccountable(L"segment map_obj", segmentMap));
  // TODO: would be nice to return actual child segment deltas too, but the
  // optimizations are confusing
  return resources;
}
} // namespace org::apache::lucene::index