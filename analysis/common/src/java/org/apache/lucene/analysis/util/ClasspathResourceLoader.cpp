using namespace std;

#include "ClasspathResourceLoader.h"

namespace org::apache::lucene::analysis::util
{
using org::apache::lucene::util::SuppressForbidden;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated @SuppressForbidden(reason = "Deprecated method
// uses thread's context classloader, but there for backwards compatibility")
// public ClasspathResourceLoader()
ClasspathResourceLoader::ClasspathResourceLoader()
    : ClasspathResourceLoader(Thread::currentThread().getContextClassLoader())
{
}

ClasspathResourceLoader::ClasspathResourceLoader(shared_ptr<ClassLoader> loader)
    : ClasspathResourceLoader(nullptr, loader)
{
}

ClasspathResourceLoader::ClasspathResourceLoader(type_info clazz)
    : ClasspathResourceLoader(clazz, clazz.getClassLoader())
{
}

ClasspathResourceLoader::ClasspathResourceLoader(type_info clazz,
                                                 shared_ptr<ClassLoader> loader)
    : clazz(clazz), loader(loader)
{
}

shared_ptr<InputStream> ClasspathResourceLoader::openResource(
    const wstring &resource) 
{
  shared_ptr<InputStream> *const stream =
      (clazz != nullptr) ? clazz.getResourceAsStream(resource)
                         : loader->getResourceAsStream(resource);
  if (stream == nullptr) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Resource not found: " + resource);
  }
  return stream;
}

template <typename T>
type_info ClasspathResourceLoader::findClass(const wstring &cname,
                                             type_info<T> &expectedType)
{
  try {
    return type_info::forName(cname, true, loader).asSubclass(expectedType);
  } catch (const runtime_error &e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("Cannot load class: " + cname,
    // e);
    throw runtime_error(L"Cannot load class: " + cname);
  }
}

template <typename T>
T ClasspathResourceLoader::newInstance(const wstring &cname,
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
} // namespace org::apache::lucene::analysis::util