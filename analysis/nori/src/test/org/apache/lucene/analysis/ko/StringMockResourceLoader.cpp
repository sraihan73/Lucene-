using namespace std;

#include "StringMockResourceLoader.h"

namespace org::apache::lucene::analysis::ko
{
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;

StringMockResourceLoader::StringMockResourceLoader(const wstring &text)
{
  this->text = text;
}

template <typename T>
type_info StringMockResourceLoader::findClass(const wstring &cname,
                                              type_info<T> &expectedType)
{
  try {
    return type_info::forName(cname).asSubclass(expectedType);
  } catch (const runtime_error &e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("Cannot load class: " + cname,
    // e);
    throw runtime_error(L"Cannot load class: " + cname);
  }
}

template <typename T>
T StringMockResourceLoader::newInstance(const wstring &cname,
                                        type_info<T> &expectedType)
{
  type_info clazz = findClass(cname, expectedType);
  try {
    return clazz.newInstance();
  } catch (const runtime_error &e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("Cannot create instance: " +
    // cname, e);
    throw runtime_error(L"Cannot create instance: " + cname);
  }
}

shared_ptr<InputStream> StringMockResourceLoader::openResource(
    const wstring &resource) 
{
  return make_shared<ByteArrayInputStream>(
      text.getBytes(StandardCharsets::UTF_8));
}
} // namespace org::apache::lucene::analysis::ko