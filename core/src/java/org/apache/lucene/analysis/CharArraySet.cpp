using namespace std;

#include "CharArraySet.h"

namespace org::apache::lucene::analysis
{

const shared_ptr<CharArraySet> CharArraySet::EMPTY_SET =
    make_shared<CharArraySet>(CharArrayMap::emptyMap<any>());
const any CharArraySet::PLACEHOLDER = nullptr;

CharArraySet::CharArraySet(int startSize, bool ignoreCase)
    : CharArraySet(new CharArrayMap<>(startSize, ignoreCase))
{
}

template <typename T1>
CharArraySet::CharArraySet(shared_ptr<deque<T1>> c, bool ignoreCase)
    : CharArraySet(c->size(), ignoreCase)
{
  addAll(c);
}

CharArraySet::CharArraySet(shared_ptr<CharArrayMap<any>> map_obj) : map_obj(map_obj) {}

void CharArraySet::clear() { map_obj->clear(); }

bool CharArraySet::contains(std::deque<wchar_t> &text, int off, int len)
{
  return map_obj->containsKey(text, off, len);
}

bool CharArraySet::contains(shared_ptr<std::wstring> cs)
{
  return map_obj->containsKey(cs);
}

bool CharArraySet::contains(any o) { return map_obj->containsKey(o); }

bool CharArraySet::add(any o) { return map_obj->put(o, PLACEHOLDER) == nullptr; }

bool CharArraySet::add(shared_ptr<std::wstring> text)
{
  return map_obj->put(text, PLACEHOLDER) == nullptr;
}

bool CharArraySet::add(const wstring &text)
{
  return map_obj->put(text, PLACEHOLDER) == nullptr;
}

bool CharArraySet::add(std::deque<wchar_t> &text)
{
  return map_obj->put(text, PLACEHOLDER) == nullptr;
}

int CharArraySet::size() { return map_obj->size(); }

shared_ptr<CharArraySet>
CharArraySet::unmodifiableSet(shared_ptr<CharArraySet> set)
{
  if (set == nullptr) {
    throw make_shared<NullPointerException>(L"Given set is null");
  }
  if (set == EMPTY_SET) {
    return EMPTY_SET;
  }
  if (std::dynamic_pointer_cast<CharArrayMap::UnmodifiableCharArrayMap>(
          set->map_obj) != nullptr) {
    return set;
  }
  return make_shared<CharArraySet>(CharArrayMap::unmodifiableMap(set->map_obj));
}

template <typename T1>
shared_ptr<CharArraySet> CharArraySet::copy(shared_ptr<Set<T1>> set)
{
  if (set == EMPTY_SET) {
    return EMPTY_SET;
  }
  if (std::dynamic_pointer_cast<CharArraySet>(set) != nullptr) {
    shared_ptr<CharArraySet> *const source =
        std::static_pointer_cast<CharArraySet>(set);
    return make_shared<CharArraySet>(CharArrayMap::copy(source->map_obj));
  }
  return make_shared<CharArraySet>(set, false);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @SuppressWarnings("unchecked") public
// java.util.Iterator<Object> iterator()
shared_ptr<Iterator<any>> CharArraySet::iterator()
{
  // use the AbstractSet#keySet()'s iterator (to not produce endless recursion)
  return map_obj->originalKeySet()->begin();
}

wstring CharArraySet::toString()
{
  shared_ptr<StringBuilder> *const sb = make_shared<StringBuilder>(L"[");
  for (auto item : shared_from_this()) {
    if (sb->length() > 1) {
      sb->append(L", ");
    }
    if (dynamic_cast<std::deque<wchar_t>>(item) != nullptr) {
      sb->append(static_cast<std::deque<wchar_t>>(item));
    } else {
      sb->append(item);
    }
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return sb->append(L']')->toString();
}
} // namespace org::apache::lucene::analysis