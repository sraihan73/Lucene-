using namespace std;

#include "Accountables.h"

namespace org::apache::lucene::util
{

Accountables::Accountables() {}

wstring Accountables::toString(shared_ptr<Accountable> a)
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  toString(sb, a, 0);
  return sb->toString();
}

shared_ptr<StringBuilder> Accountables::toString(shared_ptr<StringBuilder> dest,
                                                 shared_ptr<Accountable> a,
                                                 int depth)
{
  for (int i = 1; i < depth; i++) {
    dest->append(L"    ");
  }

  if (depth > 0) {
    dest->append(L"|-- ");
  }

  // C++ TODO: There is no native C++ equivalent to 'toString':
  dest->append(a->toString());
  dest->append(L": ");
  dest->append(RamUsageEstimator::humanReadableUnits(a->ramBytesUsed()));
  dest->append(L"\r\n");

  for (shared_ptr<Accountable> child : a->getChildResources()) {
    toString(dest, child, depth + 1);
  }

  return dest;
}

shared_ptr<Accountable>
Accountables::namedAccountable(const wstring &description,
                               shared_ptr<Accountable> in_)
{
  return namedAccountable(description + L" [" + in_ + L"]",
                          in_->getChildResources(), in_->ramBytesUsed());
}

shared_ptr<Accountable>
Accountables::namedAccountable(const wstring &description, int64_t bytes)
{
  return namedAccountable(
      description, Collections::emptyList<std::shared_ptr<Accountable>>(),
      bytes);
}

template <typename T1, typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: public static java.util.deque<Accountable>
// namedAccountables(std::wstring prefix, java.util.Map<?,? extends Accountable> in)
shared_ptr<deque<std::shared_ptr<Accountable>>>
Accountables::namedAccountables(const wstring &prefix, unordered_map<T1> in_)
{
  deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>();
  for (auto kv : in_) {
    resources.push_back(
        namedAccountable(prefix + L" '" + kv.first + L"'", kv.second));
  }
  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(resources, new
  // java.util.Comparator<Accountable>()
  sort(resources.begin(), resources.end(),
       make_shared<ComparatorAnonymousInnerClass>());
  return Collections::unmodifiableList(resources);
}

Accountables::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass() {}

int Accountables::ComparatorAnonymousInnerClass::compare(
    shared_ptr<Accountable> o1, shared_ptr<Accountable> o2)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return o1->toString()->compareTo(o2->toString());
}

shared_ptr<Accountable> Accountables::namedAccountable(
    const wstring &description,
    shared_ptr<deque<std::shared_ptr<Accountable>>> children,
    int64_t const bytes)
{
  return make_shared<AccountableAnonymousInnerClass>(description, children,
                                                     bytes);
}

Accountables::AccountableAnonymousInnerClass::AccountableAnonymousInnerClass(
    const wstring &description,
    shared_ptr<deque<std::shared_ptr<Accountable>>> children,
    int64_t bytes)
{
  this->description = description;
  this->children = children;
  this->bytes = bytes;
}

int64_t Accountables::AccountableAnonymousInnerClass::ramBytesUsed()
{
  return bytes;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
Accountables::AccountableAnonymousInnerClass::getChildResources()
{
  return children;
}

wstring Accountables::AccountableAnonymousInnerClass::toString()
{
  return description;
}
} // namespace org::apache::lucene::util