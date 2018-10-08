using namespace std;

#include "FacetLabel.h"

namespace org::apache::lucene::facet::taxonomy
{
//    import static org.apache.lucene.util.ByteBlockPool.BYTE_BLOCK_SIZE;
using LruTaxonomyWriterCache =
    org::apache::lucene::facet::taxonomy::writercache::LruTaxonomyWriterCache;
using NameHashIntCacheLRU =
    org::apache::lucene::facet::taxonomy::writercache::NameHashIntCacheLRU;

FacetLabel::FacetLabel(shared_ptr<FacetLabel> copyFrom, int const prefixLen)
    : components(copyFrom->components), length(prefixLen)
{
  // while the code which calls this method is safe, at some point a test
  // tripped on AIOOBE in toString, but we failed to reproduce. adding the
  // assert as a safety check.
  assert((prefixLen >= 0 && prefixLen <= copyFrom->components.size(),
          L"prefixLen cannot be negative nor larger than the given components' "
          L"length: prefixLen=" +
              to_wstring(prefixLen) + L" components.length=" +
              copyFrom->components.size()));
}

FacetLabel::FacetLabel(deque<wstring> &components)
    : components(components), length(components.size())
{
  checkComponents();
}

FacetLabel::FacetLabel(const wstring &dim, std::deque<wstring> &path)
    : components(std::deque<wstring>(1 + path.size())),
      length(components.size())
{
  components[0] = dim;
  System::arraycopy(path, 0, components, 1, path.size());
  checkComponents();
}

void FacetLabel::checkComponents()
{
  int64_t len = 0;
  for (auto comp : components) {
    if (comp == L"" || comp.isEmpty()) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      throw invalid_argument(L"empty or null components not allowed: " +
                             Arrays->toString(components));
    }
    len += comp.length();
  }
  len += components.size() - 1; // add separators
  if (len > MAX_CATEGORY_PATH_LENGTH) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    throw invalid_argument(
        L"category path exceeds maximum allowed path length: max=" +
        to_wstring(MAX_CATEGORY_PATH_LENGTH) + L" len=" + to_wstring(len) +
        L" path=" + Arrays->toString(components)->substr(0, 30) + L"...");
  }
}

int FacetLabel::compareTo(shared_ptr<FacetLabel> other)
{
  constexpr int len = length < other->length ? length : other->length;
  for (int i = 0, j = 0; i < len; i++, j++) {
    int cmp = components[i].compare(other->components[j]);
    if (cmp < 0) {
      return -1; // this is 'before'
    }
    if (cmp > 0) {
      return 1; // this is 'after'
    }
  }

  // one is a prefix of the other
  return length - other->length;
}

bool FacetLabel::equals(any obj)
{
  if (!(std::dynamic_pointer_cast<FacetLabel>(obj) != nullptr)) {
    return false;
  }

  shared_ptr<FacetLabel> other = any_cast<std::shared_ptr<FacetLabel>>(obj);
  if (length != other->length) {
    return false; // not same length, cannot be equal
  }

  // CategoryPaths are more likely to differ at the last components, so start
  // from last-first
  for (int i = length - 1; i >= 0; i--) {
    if (components[i] != other->components[i]) {
      return false;
    }
  }
  return true;
}

int FacetLabel::hashCode()
{
  if (length == 0) {
    return 0;
  }

  int hash = length;
  for (int i = 0; i < length; i++) {
    hash = hash * 31 + components[i].hashCode();
  }
  return hash;
}

int64_t FacetLabel::longHashCode()
{
  if (length == 0) {
    return 0;
  }

  int64_t hash = length;
  for (int i = 0; i < length; i++) {
    hash = hash * 65599 + components[i].hashCode();
  }
  return hash;
}

shared_ptr<FacetLabel> FacetLabel::subpath(int const length)
{
  if (length >= this->length || length < 0) {
    return shared_from_this();
  } else {
    return make_shared<FacetLabel>(shared_from_this(), length);
  }
}

wstring FacetLabel::toString()
{
  if (length == 0) {
    return L"FacetLabel: []";
  }
  std::deque<wstring> parts(length);
  System::arraycopy(components, 0, parts, 0, length);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"FacetLabel: " + Arrays->toString(parts);
}
} // namespace org::apache::lucene::facet::taxonomy