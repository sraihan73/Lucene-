using namespace std;

#include "SortedIntSet.h"

namespace org::apache::lucene::util::automaton
{
using ArrayUtil = org::apache::lucene::util::ArrayUtil;

SortedIntSet::SortedIntSet(int capacity)
{
  values = std::deque<int>(capacity);
  counts = std::deque<int>(capacity);
}

void SortedIntSet::incr(int num)
{
  if (useTreeMap) {
    const optional<int> key = num;
    optional<int> val = map_obj[key];
    if (!val) {
      map_obj.emplace(key, 1);
    } else {
      map_obj.emplace(key, 1 + val);
    }
    return;
  }

  if (upto == values.size()) {
    values = ArrayUtil::grow(values, 1 + upto);
    counts = ArrayUtil::grow(counts, 1 + upto);
  }

  for (int i = 0; i < upto; i++) {
    if (values[i] == num) {
      counts[i]++;
      return;
    } else if (num < values[i]) {
      // insert here
      int j = upto - 1;
      while (j >= i) {
        values[1 + j] = values[j];
        counts[1 + j] = counts[j];
        j--;
      }
      values[i] = num;
      counts[i] = 1;
      upto++;
      return;
    }
  }

  // append
  values[upto] = num;
  counts[upto] = 1;
  upto++;

  if (upto == TREE_MAP_CUTOVER) {
    useTreeMap = true;
    for (int i = 0; i < upto; i++) {
      map_obj.emplace(values[i], counts[i]);
    }
  }
}

void SortedIntSet::decr(int num)
{

  if (useTreeMap) {
    constexpr int count = map_obj[num];
    if (count == 1) {
      map_obj.erase(num);
    } else {
      map_obj.emplace(num, count - 1);
    }
    // Fall back to simple arrays once we touch zero again
    if (map_obj.empty()) {
      useTreeMap = false;
      upto = 0;
    }
    return;
  }

  for (int i = 0; i < upto; i++) {
    if (values[i] == num) {
      counts[i]--;
      if (counts[i] == 0) {
        constexpr int limit = upto - 1;
        while (i < limit) {
          values[i] = values[i + 1];
          counts[i] = counts[i + 1];
          i++;
        }
        upto = limit;
      }
      return;
    }
  }
  assert(false);
}

void SortedIntSet::computeHash()
{
  if (useTreeMap) {
    if (map_obj.size() > values.size()) {
      constexpr int size = ArrayUtil::oversize(map_obj.size(), Integer::BYTES);
      values = std::deque<int>(size);
      counts = std::deque<int>(size);
    }
    hashCode_ = map_obj.size();
    upto = 0;
    for (auto state : map_obj) {
      hashCode_ = 683 * hashCode_ + state.first;
      values[upto++] = state.first;
    }
  } else {
    hashCode_ = upto;
    for (int i = 0; i < upto; i++) {
      hashCode_ = 683 * hashCode_ + values[i];
    }
  }
}

shared_ptr<FrozenIntSet> SortedIntSet::freeze(int state)
{
  const std::deque<int> c = std::deque<int>(upto);
  System::arraycopy(values, 0, c, 0, upto);
  return make_shared<FrozenIntSet>(c, hashCode_, state);
}

int SortedIntSet::hashCode() { return hashCode_; }

bool SortedIntSet::equals(any _other)
{
  if (_other == nullptr) {
    return false;
  }
  if (!(std::dynamic_pointer_cast<FrozenIntSet>(_other) != nullptr)) {
    return false;
  }
  shared_ptr<FrozenIntSet> other =
      any_cast<std::shared_ptr<FrozenIntSet>>(_other);
  if (hashCode_ != other->hashCode_) {
    return false;
  }
  if (other->values.size() != upto) {
    return false;
  }
  for (int i = 0; i < upto; i++) {
    if (other->values[i] != values[i]) {
      return false;
    }
  }

  return true;
}

wstring SortedIntSet::toString()
{
  shared_ptr<StringBuilder> sb = (make_shared<StringBuilder>())->append(L'[');
  for (int i = 0; i < upto; i++) {
    if (i > 0) {
      sb->append(L' ');
    }
    sb->append(values[i])->append(L':')->append(counts[i]);
  }
  sb->append(L']');
  return sb->toString();
}

SortedIntSet::FrozenIntSet::FrozenIntSet(std::deque<int> &values, int hashCode,
                                         int state)
    : values(values), hashCode(hashCode), state(state)
{
}

SortedIntSet::FrozenIntSet::FrozenIntSet(int num, int state)
    : values(std::deque<int>{num}), hashCode(683 + num), state(state)
{
}

int SortedIntSet::FrozenIntSet::hashCode() { return hashCode_; }

bool SortedIntSet::FrozenIntSet::equals(any _other)
{
  if (_other == nullptr) {
    return false;
  }
  if (std::dynamic_pointer_cast<FrozenIntSet>(_other) != nullptr) {
    shared_ptr<FrozenIntSet> other =
        any_cast<std::shared_ptr<FrozenIntSet>>(_other);
    if (hashCode_ != other->hashCode_) {
      return false;
    }
    if (other->values.size() != values.size()) {
      return false;
    }
    for (int i = 0; i < values.size(); i++) {
      if (other->values[i] != values[i]) {
        return false;
      }
    }
    return true;
  } else if (std::dynamic_pointer_cast<SortedIntSet>(_other) != nullptr) {
    shared_ptr<SortedIntSet> other =
        any_cast<std::shared_ptr<SortedIntSet>>(_other);
    if (hashCode_ != other->hashCode) {
      return false;
    }
    if (other->values.size() != values.size()) {
      return false;
    }
    for (int i = 0; i < values.size(); i++) {
      if (other->values[i] != values[i]) {
        return false;
      }
    }
    return true;
  }

  return false;
}

wstring SortedIntSet::FrozenIntSet::toString()
{
  shared_ptr<StringBuilder> sb = (make_shared<StringBuilder>())->append(L'[');
  for (int i = 0; i < values.size(); i++) {
    if (i > 0) {
      sb->append(L' ');
    }
    sb->append(values[i]);
  }
  sb->append(L']');
  return sb->toString();
}
} // namespace org::apache::lucene::util::automaton