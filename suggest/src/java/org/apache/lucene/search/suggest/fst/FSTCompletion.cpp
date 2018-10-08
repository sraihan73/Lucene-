using namespace std;

#include "FSTCompletion.h"

namespace org::apache::lucene::search::suggest::fst
{
using namespace org::apache::lucene::util;
using FST = org::apache::lucene::util::fst::FST;
using org::apache::lucene::util::fst::FST::Arc;

FSTCompletion::Completion::Completion(shared_ptr<BytesRef> key, int bucket)
    : utf8(BytesRef::deepCopyOf(key)), bucket(bucket)
{
}

wstring FSTCompletion::Completion::toString()
{
  return utf8->utf8ToString() + L"/" + to_wstring(bucket);
}

int FSTCompletion::Completion::compareTo(shared_ptr<Completion> o)
{
  return this->utf8->compareTo(o->utf8);
}

const deque<std::shared_ptr<Completion>> FSTCompletion::EMPTY_RESULT =
    deque<std::shared_ptr<Completion>>();

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) public
// FSTCompletion(org.apache.lucene.util.fst.FST<Object> automaton, bool
// higherWeightsFirst, bool exactFirst)
FSTCompletion::FSTCompletion(shared_ptr<FST<any>> automaton,
                             bool higherWeightsFirst, bool exactFirst)
    : automaton(automaton)
{
  if (automaton != nullptr) {
    this->rootArcs = cacheRootArcs(automaton);
  } else {
    this->rootArcs = std::deque<std::shared_ptr<FST::Arc>>(0);
  }
  this->higherWeightsFirst = higherWeightsFirst;
  this->exactFirst = exactFirst;
}

FSTCompletion::FSTCompletion(shared_ptr<FST<any>> automaton)
    : FSTCompletion(automaton, true, true)
{
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) private static
// org.apache.lucene.util.fst.FST.Arc<Object>[]
// cacheRootArcs(org.apache.lucene.util.fst.FST<Object> automaton)
std::deque<FST::Arc<any>>
FSTCompletion::cacheRootArcs(shared_ptr<FST<any>> automaton)
{
  try {
    deque<FST::Arc<any>> rootArcs = deque<FST::Arc<any>>();
    shared_ptr<FST::Arc<any>> arc =
        automaton->getFirstArc(make_shared<FST::Arc<any>>());
    shared_ptr<FST::BytesReader> fstReader = automaton->getBytesReader();
    automaton->readFirstTargetArc(arc, arc, fstReader);
    while (true) {
      rootArcs.push_back((make_shared<FST::Arc<>>())->copyFrom(arc));
      if (arc->isLast()) {
        break;
      }
      automaton->readNextArc(arc, fstReader);
    }

    reverse(rootArcs.begin(), rootArcs.end()); // we want highest weights first.
    return rootArcs.toArray(
        std::deque<std::shared_ptr<FST::Arc>>(rootArcs.size()));
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

int FSTCompletion::getExactMatchStartingFromRootArc(int rootArcIndex,
                                                    shared_ptr<BytesRef> utf8)
{
  // Get the UTF-8 bytes representation of the input key.
  try {
    shared_ptr<FST::Arc<any>> *const scratch = make_shared<FST::Arc<any>>();
    shared_ptr<FST::BytesReader> fstReader = automaton->getBytesReader();
    for (; rootArcIndex < rootArcs.size(); rootArcIndex++) {
      shared_ptr<FST::Arc<any>> *const rootArc = rootArcs[rootArcIndex];
      shared_ptr<FST::Arc<any>> *const arc = scratch->copyFrom(rootArc);

      // Descend into the automaton using the key as prefix.
      if (descendWithPrefix(arc, utf8)) {
        automaton->readFirstTargetArc(arc, arc, fstReader);
        if (arc->label == FST::END_LABEL) {
          // Normalize prefix-encoded weight.
          return rootArc->label;
        }
      }
    }
  } catch (const IOException &e) {
    // Should never happen, but anyway.
    throw runtime_error(e);
  }

  // No match.
  return -1;
}

deque<std::shared_ptr<Completion>>
FSTCompletion::lookup(shared_ptr<std::wstring> key, int num)
{
  if (key->length() == 0 || automaton == nullptr) {
    return EMPTY_RESULT;
  }

  try {
    shared_ptr<BytesRef> keyUtf8 = make_shared<BytesRef>(key);
    if (!higherWeightsFirst && rootArcs.size() > 1) {
      // We could emit a warning here (?). An optimal strategy for
      // alphabetically sorted
      // suggestions would be to add them with a constant weight -- this saves
      // unnecessary
      // traversals and sorting.
      return lookupSortedAlphabetically(keyUtf8, num);
    } else {
      return lookupSortedByWeight(keyUtf8, num, false);
    }
  } catch (const IOException &e) {
    // Should never happen, but anyway.
    throw runtime_error(e);
  }
}

deque<std::shared_ptr<Completion>>
FSTCompletion::lookupSortedAlphabetically(shared_ptr<BytesRef> key,
                                          int num) 
{
  // Greedily get num results from each weight branch.
  deque<std::shared_ptr<Completion>> res =
      lookupSortedByWeight(key, num, true);

  // Sort and trim.
  sort(res.begin(), res.end());
  if (res.size() > num) {
    res = res.subList(0, num);
  }
  return res;
}

deque<std::shared_ptr<Completion>>
FSTCompletion::lookupSortedByWeight(shared_ptr<BytesRef> key, int num,
                                    bool collectAll) 
{
  // Don't overallocate the results buffers. This also serves the purpose of
  // allowing the user of this class to request all matches using
  // Integer.MAX_VALUE as the number of results.
  const deque<std::shared_ptr<Completion>> res =
      deque<std::shared_ptr<Completion>>(min(10, num));

  shared_ptr<BytesRef> *const output = BytesRef::deepCopyOf(key);
  for (int i = 0; i < rootArcs.size(); i++) {
    shared_ptr<FST::Arc<any>> *const rootArc = rootArcs[i];
    shared_ptr<FST::Arc<any>> *const arc =
        (make_shared<FST::Arc<any>>())->copyFrom(rootArc);

    // Descend into the automaton using the key as prefix.
    if (descendWithPrefix(arc, key)) {
      // A subgraph starting from the current node has the completions
      // of the key prefix. The arc we're at is the last key's byte,
      // so we will collect it too.
      output->length = key->length - 1;
      if (collect(res, num, rootArc->label, output, arc) && !collectAll) {
        // We have enough suggestions to return immediately. Keep on looking
        // for an
        // exact match, if requested.
        if (exactFirst) {
          if (!checkExistingAndReorder(res, key)) {
            int exactMatchBucket = getExactMatchStartingFromRootArc(i, key);
            if (exactMatchBucket != -1) {
              // Insert as the first result and truncate at num.
              while (res.size() >= num) {
                res.pop_back();
              }
              res.push_back(0, make_shared<Completion>(key, exactMatchBucket));
            }
          }
        }
        break;
      }
    }
  }
  return res;
}

bool FSTCompletion::checkExistingAndReorder(
    deque<std::shared_ptr<Completion>> &deque, shared_ptr<BytesRef> key)
{
  // We assume deque does not have duplicates (because of how the FST is
  // created).
  for (int i = deque.size(); --i >= 0;) {
    if (key->equals(deque[i]->utf8)) {
      // Key found. Unless already at i==0, remove it and push up front so
      // that the ordering
      // remains identical with the exception of the exact match.
      deque.push_back(0, deque.erase(deque.begin() + i));
      return true;
    }
  }
  return false;
}

bool FSTCompletion::descendWithPrefix(
    shared_ptr<FST::Arc<any>> arc, shared_ptr<BytesRef> utf8) 
{
  constexpr int max = utf8->offset + utf8->length;
  // Cannot save as instance var since multiple threads
  // can use FSTCompletion at once...
  shared_ptr<FST::BytesReader> *const fstReader = automaton->getBytesReader();
  for (int i = utf8->offset; i < max; i++) {
    if (automaton->findTargetArc(utf8->bytes[i] & 0xff, arc, arc, fstReader) ==
        nullptr) {
      // No matching prefixes, return an empty result.
      return false;
    }
  }
  return true;
}

bool FSTCompletion::collect(deque<std::shared_ptr<Completion>> &res, int num,
                            int bucket, shared_ptr<BytesRef> output,
                            shared_ptr<FST::Arc<any>> arc) 
{
  if (output->length == output->bytes.size()) {
    output->bytes = ArrayUtil::grow(output->bytes);
  }
  assert(output->offset == 0);
  output->bytes[output->length++] = static_cast<char>(arc->label);
  shared_ptr<FST::BytesReader> fstReader = automaton->getBytesReader();
  automaton->readFirstTargetArc(arc, arc, fstReader);
  while (true) {
    if (arc->label == FST::END_LABEL) {
      res.push_back(make_shared<Completion>(output, bucket));
      if (res.size() >= num) {
        return true;
      }
    } else {
      int save = output->length;
      if (collect(res, num, bucket, output,
                  (make_shared<FST::Arc<any>>())->copyFrom(arc))) {
        return true;
      }
      output->length = save;
    }

    if (arc->isLast()) {
      break;
    }
    automaton->readNextArc(arc, fstReader);
  }
  return false;
}

int FSTCompletion::getBucketCount() { return rootArcs.size(); }

int FSTCompletion::getBucket(shared_ptr<std::wstring> key)
{
  return getExactMatchStartingFromRootArc(0, make_shared<BytesRef>(key));
}

shared_ptr<FST<any>> FSTCompletion::getFST() { return automaton; }
} // namespace org::apache::lucene::search::suggest::fst