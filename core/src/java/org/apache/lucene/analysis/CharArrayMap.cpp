using namespace std;

#include "CharArrayMap.h"

namespace org::apache::lucene::analysis
{
using CharacterUtils = org::apache::lucene::analysis::CharacterUtils;

// C++ TODO: You cannot use 'shared_from_this' in a constructor:
CharArrayMap<V>::CharArraySetAnonymousInnerClass::
    CharArraySetAnonymousInnerClass(
        shared_ptr<CharArrayMap<std::shared_ptr<V>>> outerInstance,
        CharArrayMap(CharArrayMap) shared_from_this())
    : CharArraySet((CharArrayMap)this)
{
  this->outerInstance = outerInstance;
}

bool CharArrayMap<V>::CharArraySetAnonymousInnerClass::add(any o)
{
  throw make_shared<UnsupportedOperationException>();
}

bool CharArrayMap<V>::CharArraySetAnonymousInnerClass::add(
    shared_ptr<std::wstring> text)
{
  throw make_shared<UnsupportedOperationException>();
}

bool CharArrayMap<V>::CharArraySetAnonymousInnerClass::add(const wstring &text)
{
  throw make_shared<UnsupportedOperationException>();
}

bool CharArrayMap<V>::CharArraySetAnonymousInnerClass::add(
    std::deque<wchar_t> &text)
{
  throw make_shared<UnsupportedOperationException>();
}

CharArrayMap<V>::EntryIterator::EntryIterator(
    shared_ptr<CharArrayMap<V>> outerInstance, bool allowModify)
    : allowModify(allowModify), outerInstance(outerInstance)
{
  goNext();
}

void CharArrayMap<V>::EntryIterator::goNext()
{
  lastPos = pos;
  pos++;
  while (pos < outerInstance->keys.size() &&
         outerInstance->keys[pos] == nullptr) {
    pos++;
  }
}

bool CharArrayMap<V>::EntryIterator::hasNext()
{
  return pos < outerInstance->keys.size();
}

std::deque<wchar_t> CharArrayMap<V>::EntryIterator::nextKey()
{
  goNext();
  return outerInstance->keys[lastPos];
}

wstring CharArrayMap<V>::EntryIterator::nextKeyString()
{
  return wstring(nextKey());
}

shared_ptr<V> CharArrayMap<V>::EntryIterator::currentValue()
{
  return outerInstance->values[lastPos];
}

shared_ptr<V> CharArrayMap<V>::EntryIterator::setValue(shared_ptr<V> value)
{
  if (!allowModify) {
    throw make_shared<UnsupportedOperationException>();
  }
  shared_ptr<V> old = outerInstance->values[lastPos];
  outerInstance->values[lastPos] = value;
  return old;
}

unordered_map::Entry<any, std::shared_ptr<V>>
CharArrayMap<V>::EntryIterator::next()
{
  goNext();
  return make_shared<MapEntry>(outerInstance, lastPos, allowModify);
}

void CharArrayMap<V>::EntryIterator::remove()
{
  throw make_shared<UnsupportedOperationException>();
}

CharArrayMap<V>::MapEntry::MapEntry(shared_ptr<CharArrayMap<V>> outerInstance,
                                    int pos, bool allowModify)
    : pos(pos), allowModify(allowModify), outerInstance(outerInstance)
{
}

any CharArrayMap<V>::MapEntry::getKey()
{
  // we must clone here, as putAll to another CharArrayMap
  // with other case sensitivity flag would corrupt the keys
  return outerInstance->keys[pos].clone();
}

shared_ptr<V> CharArrayMap<V>::MapEntry::getValue()
{
  return outerInstance->values[pos];
}

shared_ptr<V> CharArrayMap<V>::MapEntry::setValue(shared_ptr<V> value)
{
  if (!allowModify) {
    throw make_shared<UnsupportedOperationException>();
  }
  shared_ptr<V> *const old = outerInstance->values[pos];
  outerInstance->values[pos] = value;
  return old;
}

wstring CharArrayMap<V>::MapEntry::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return (make_shared<StringBuilder>())
      ->append(outerInstance->keys[pos])
      ->append(L'=')
      ->append((outerInstance->values[pos] == outerInstance)
                   ? L"(this Map)"
                   : outerInstance->values[pos])
      ->toString();
}

CharArrayMap<V>::EntrySet::EntrySet(shared_ptr<CharArrayMap<V>> outerInstance,
                                    bool allowModify)
    : allowModify(allowModify), outerInstance(outerInstance)
{
}

shared_ptr<EntryIterator> CharArrayMap<V>::EntrySet::iterator()
{
  return make_shared<EntryIterator>(outerInstance, allowModify);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @SuppressWarnings("unchecked") public bool
// contains(Object o)
bool CharArrayMap<V>::EntrySet::contains(any o)
{
  if (!(dynamic_cast<unordered_map::Entry>(o) != nullptr)) {
    return false;
  }
  const unordered_map::Entry<any, std::shared_ptr<V>> e =
      any_cast<unordered_map::Entry<any, std::shared_ptr<V>>>(o);
  constexpr any key = e.getKey();
  constexpr any val = e.getValue();
  constexpr any v = outerInstance->get(key);
  return v == nullptr ? val == nullptr : v.equals(val);
}

bool CharArrayMap<V>::EntrySet::remove(any o)
{
  throw make_shared<UnsupportedOperationException>();
}

int CharArrayMap<V>::EntrySet::size() { return outerInstance->count; }

void CharArrayMap<V>::EntrySet::clear()
{
  if (!allowModify) {
    throw make_shared<UnsupportedOperationException>();
  }
  outerInstance->clear();
}
} // namespace org::apache::lucene::analysis