using namespace std;

#include "InterpolatedProperties.h"

namespace org::apache::lucene::dependencies
{

const shared_ptr<java::util::regex::Pattern>
    InterpolatedProperties::PROPERTY_REFERENCE_PATTERN =
        java::util::regex::Pattern::compile(L"\\$\\{(?<name>[^}]+)\\}");

void InterpolatedProperties::load(shared_ptr<InputStream> inStream) throw(
    IOException)
{
  throw make_shared<UnsupportedOperationException>(
      L"InterpolatedProperties.load(InputStream) is not supported.");
}

void InterpolatedProperties::load(shared_ptr<Reader> reader) 
{
  shared_ptr<Properties> p = make_shared<Properties>();
  p->load(reader);

  shared_ptr<LinkedHashMap<wstring, wstring>> props =
      make_shared<LinkedHashMap<wstring, wstring>>();
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: java.util.Iterator<?> e = p.propertyNames();
  java::util::Iterator < ? > e = p->propertyNames();
  while (e->hasMoreElements()) {
    wstring key = static_cast<wstring>(e->nextElement());
    props->put(key, p->getProperty(key));
  }

  resolve(props).forEach([&](k, v) { this->setProperty(k, v); });
}

unordered_map<wstring, wstring>
InterpolatedProperties::resolve(unordered_map<wstring, wstring> &props)
{
  shared_ptr<LinkedHashMap<wstring, wstring>> resolved =
      make_shared<LinkedHashMap<wstring, wstring>>();
  unordered_set<wstring> recursive = unordered_set<wstring>();
  props.forEach([&](k, v) { resolve(props, resolved, recursive, k, v); });
  return resolved;
}

wstring InterpolatedProperties::resolve(
    unordered_map<wstring, wstring> &props,
    shared_ptr<LinkedHashMap<wstring, wstring>> resolved,
    shared_ptr<Set<wstring>> recursive, const wstring &key,
    const wstring &value)
{
  if (value == L"") {
    throw invalid_argument(L"Missing replaced property key: " + key);
  }

  if (recursive->contains(key)) {
    throw invalid_argument(L"Circular recursive property resolution: " +
                           recursive);
  }

  if (!resolved->containsKey(key)) {
    recursive->add(key);
    shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
    shared_ptr<Matcher> matcher = PROPERTY_REFERENCE_PATTERN->matcher(value);
    while (matcher->find()) {
      wstring referenced = matcher->group(L"name");
      wstring concrete =
          resolve(props, resolved, recursive, referenced, props[referenced]);
      matcher->appendReplacement(buffer, Matcher::quoteReplacement(concrete));
    }
    matcher->appendTail(buffer);
    resolved->put(key, buffer->toString());
    recursive->remove(key);
  }
  assert(resolved->get(key).equals(value));
  return resolved->get(key);
}

void InterpolatedProperties::main(std::deque<wstring> &args)
{
  {
    unordered_map<wstring, wstring> props =
        make_shared<LinkedHashMap<wstring, wstring>>();
    props.emplace(L"a", L"${b}");
    props.emplace(L"b", L"${c}");
    props.emplace(L"c", L"foo");
    props.emplace(L"d", L"${a}/${b}/${c}");
    assertEquals(resolve(props),
                 {L"a=foo", L"b=foo", L"c=foo", L"d=foo/foo/foo"});
  }

  {
    unordered_map<wstring, wstring> props =
        make_shared<LinkedHashMap<wstring, wstring>>();
    props.emplace(L"a", L"foo");
    props.emplace(L"b", L"${a}");
    assertEquals(resolve(props), {L"a=foo", L"b=foo"});
  }

  {
    unordered_map<wstring, wstring> props =
        make_shared<LinkedHashMap<wstring, wstring>>();
    props.emplace(L"a", L"${b}");
    props.emplace(L"b", L"${c}");
    props.emplace(L"c", L"${a}");
    try {
      resolve(props);
    } catch (const invalid_argument &e) {
      // Expected, circular reference.
      if (!e.what()->contains(L"Circular recursive")) {
        throw make_shared<AssertionError>();
      }
    }
  }

  {
    unordered_map<wstring, wstring> props =
        make_shared<LinkedHashMap<wstring, wstring>>();
    props.emplace(L"a", L"${b}");
    try {
      resolve(props);
    } catch (const invalid_argument &e) {
      // Expected, no referenced value.
      if (!e.what()->contains(L"Missing replaced")) {
        throw make_shared<AssertionError>();
      }
    }
  }
}

void InterpolatedProperties::assertEquals(
    unordered_map<wstring, wstring> &resolved, deque<wstring> &keyValuePairs)
{
  deque<wstring> result =
      resolved.entrySet()
          .stream()
          .sorted([&](a, b) { a::getKey()->compareTo(b::getKey()); })
          .map_obj([&](any e) { return e::getKey() + L"=" + e::getValue(); })
          .collect(Collectors::toList());
  if (!result.equals(Arrays::asList(keyValuePairs))) {
    throw make_shared<AssertionError>(L"Mismatch: \n" + result +
                                      L"\nExpected: " +
                                      Arrays::asList(keyValuePairs));
  }
}
} // namespace org::apache::lucene::dependencies