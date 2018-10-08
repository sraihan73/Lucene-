using namespace std;

#include "LongHashSet.h"

namespace org::apache::lucene::search
{
using PackedInts = org::apache::lucene::util::packed::PackedInts;

LongHashSet::LongHashSet(deque<int64_t> &values)
    : table(std::deque<int64_t>(tableSize)), mask(tableSize - 1),
      size(size_), hashCode(hashCode_)
{
  int tableSize = Math::toIntExact(values->length * 3LL / 2);
  tableSize = 1 << PackedInts::bitsRequired(tableSize); // make it a power of 2
  assert(tableSize >= values->length * 3LL / 2);
  Arrays::fill(table, MISSING);
  bool hasMissingValue = false;
  int size = 0;
  int hashCode = 0;
  for (int64_t value : values) {
    if (value == MISSING || add(value)) {
      if (value == MISSING) {
        hasMissingValue = true;
      }
      ++size;
      hashCode += Long::hashCode(value);
    }
  }
  this->hasMissingValue = hasMissingValue;
}

bool LongHashSet::add(int64_t l)
{
  assert(l != MISSING);
  constexpr int slot = Long::hashCode(l) & mask;
  for (int i = slot;; i = (i + 1) & mask) {
    if (table[i] == MISSING) {
      table[i] = l;
      return true;
    } else if (table[i] == l) {
      // already added
      return false;
    }
  }
}

bool LongHashSet::contains(int64_t l)
{
  if (l == MISSING) {
    return hasMissingValue;
  }
  constexpr int slot = Long::hashCode(l) & mask;
  for (int i = slot;; i = (i + 1) & mask) {
    if (table[i] == MISSING) {
      return false;
    } else if (table[i] == l) {
      return true;
    }
  }
}

int LongHashSet::size() { return size_; }

int LongHashSet::hashCode() { return hashCode_; }

bool LongHashSet::equals(any obj)
{
  if (obj != nullptr && obj.type() == LongHashSet::typeid) {
    shared_ptr<LongHashSet> that = any_cast<std::shared_ptr<LongHashSet>>(obj);
    if (hashCode_ != that->hashCode_ || size_ != that->size_ ||
        hasMissingValue != that->hasMissingValue) {
      return false;
    }
    for (auto v : table) {
      if (v != MISSING && that->contains(v) == false) {
        return false;
      }
    }
    return true;
  }
  return AbstractSet<long>::equals(obj);
}

bool LongHashSet::contains(any o)
{
  return dynamic_cast<optional<int64_t>>(o) != nullptr &&
         contains((any_cast<optional<int64_t>>(o)).value());
}

shared_ptr<Iterator<int64_t>> LongHashSet::iterator()
{
  return make_shared<IteratorAnonymousInnerClass>(shared_from_this());
}

LongHashSet::IteratorAnonymousInnerClass::IteratorAnonymousInnerClass(
    shared_ptr<LongHashSet> outerInstance)
{
  this->outerInstance = outerInstance;
  hasNext = outerInstance->hasMissingValue;
  i = -1;
  value = MISSING;
}

bool LongHashSet::IteratorAnonymousInnerClass::hasNext()
{
  if (hasNext) {
    return true;
  }
  while (++i < outerInstance->table.size()) {
    value = outerInstance->table[i];
    if (value != MISSING) {
      return hasNext = true;
    }
  }
  return false;
}

optional<int64_t> LongHashSet::IteratorAnonymousInnerClass::next()
{
  if (hasNext() == false) {
    // C++ TODO: The following line could not be converted:
    throw java.util.NoSuchElementException();
  }
  hasNext = false;
  return value;
}
} // namespace org::apache::lucene::search