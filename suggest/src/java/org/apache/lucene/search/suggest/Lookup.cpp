using namespace std;

#include "Lookup.h"

namespace org::apache::lucene::search::suggest
{
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using Dictionary = org::apache::lucene::search::spell::Dictionary;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using InputStreamDataInput = org::apache::lucene::store::InputStreamDataInput;
using OutputStreamDataOutput =
    org::apache::lucene::store::OutputStreamDataOutput;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

Lookup::LookupResult::LookupResult(shared_ptr<std::wstring> key,
                                   int64_t value)
    : LookupResult(key, nullptr, value, nullptr, nullptr)
{
}

Lookup::LookupResult::LookupResult(shared_ptr<std::wstring> key,
                                   int64_t value,
                                   shared_ptr<BytesRef> payload)
    : LookupResult(key, nullptr, value, payload, nullptr)
{
}

Lookup::LookupResult::LookupResult(shared_ptr<std::wstring> key,
                                   any highlightKey, int64_t value,
                                   shared_ptr<BytesRef> payload)
    : LookupResult(key, highlightKey, value, payload, nullptr)
{
}

Lookup::LookupResult::LookupResult(
    shared_ptr<std::wstring> key, int64_t value, shared_ptr<BytesRef> payload,
    shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts)
    : LookupResult(key, nullptr, value, payload, contexts)
{
}

Lookup::LookupResult::LookupResult(
    shared_ptr<std::wstring> key, int64_t value,
    shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts)
    : LookupResult(key, nullptr, value, nullptr, contexts)
{
}

Lookup::LookupResult::LookupResult(
    shared_ptr<std::wstring> key, any highlightKey, int64_t value,
    shared_ptr<BytesRef> payload,
    shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts)
    : key(key), highlightKey(highlightKey), value(value), payload(payload),
      contexts(contexts)
{
}

wstring Lookup::LookupResult::toString()
{
  return key + L"/" + to_wstring(value);
}

int Lookup::LookupResult::compareTo(shared_ptr<LookupResult> o)
{
  return CHARSEQUENCE_COMPARATOR->compare(key, o->key);
}

const shared_ptr<java::util::Comparator<std::shared_ptr<std::wstring>>>
    Lookup::CHARSEQUENCE_COMPARATOR = make_shared<CharSequenceComparator>();

int Lookup::CharSequenceComparator::compare(shared_ptr<std::wstring> o1,
                                            shared_ptr<std::wstring> o2)
{
  constexpr int l1 = o1->length();
  constexpr int l2 = o2->length();

  constexpr int aStop = min(l1, l2);
  for (int i = 0; i < aStop; i++) {
    int diff = o1->charAt(i) - o2->charAt(i);
    if (diff != 0) {
      return diff;
    }
  }
  // One is a prefix of the other, or, they are equal:
  return l1 - l2;
}

Lookup::LookupPriorityQueue::LookupPriorityQueue(int size)
    : org::apache::lucene::util::PriorityQueue<LookupResult>(size)
{
}

bool Lookup::LookupPriorityQueue::lessThan(shared_ptr<LookupResult> a,
                                           shared_ptr<LookupResult> b)
{
  return a->value < b->value;
}

std::deque<std::shared_ptr<LookupResult>>
Lookup::LookupPriorityQueue::getResults()
{
  int size = this->size();
  std::deque<std::shared_ptr<LookupResult>> res(size);
  for (int i = size - 1; i >= 0; i--) {
    res[i] = pop();
  }
  return res;
}

Lookup::Lookup() {}

void Lookup::build(shared_ptr<Dictionary> dict) 
{
  build(dict->getEntryIterator());
}

bool Lookup::load(shared_ptr<InputStream> input) 
{
  shared_ptr<DataInput> dataIn = make_shared<InputStreamDataInput>(input);
  try {
    return load(dataIn);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({input});
  }
}

bool Lookup::store(shared_ptr<OutputStream> output) 
{
  shared_ptr<DataOutput> dataOut = make_shared<OutputStreamDataOutput>(output);
  try {
    return store(dataOut);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({output});
  }
}

deque<std::shared_ptr<LookupResult>>
Lookup::lookup(shared_ptr<std::wstring> key, bool onlyMorePopular,
               int num) 
{
  return lookup(key, nullptr, onlyMorePopular, num);
}

deque<std::shared_ptr<LookupResult>>
Lookup::lookup(shared_ptr<std::wstring> key,
               shared_ptr<BooleanQuery> contextFilerQuery, int num,
               bool allTermsRequired, bool doHighlight) 
{
  return nullptr;
}
} // namespace org::apache::lucene::search::suggest