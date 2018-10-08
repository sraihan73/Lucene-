using namespace std;

#include "FilesystemResourceLoader.h"

namespace org::apache::lucene::analysis::util
{

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public FilesystemResourceLoader(java.nio.file.Path
// baseDirectory)
FilesystemResourceLoader::FilesystemResourceLoader(
    shared_ptr<Path> baseDirectory)
    : FilesystemResourceLoader(baseDirectory, new ClasspathResourceLoader())
{
}

FilesystemResourceLoader::FilesystemResourceLoader(
    shared_ptr<Path> baseDirectory, shared_ptr<ClassLoader> delegate_)
    : FilesystemResourceLoader(baseDirectory,
                               new ClasspathResourceLoader(delegate_))
{
}

FilesystemResourceLoader::FilesystemResourceLoader(
    shared_ptr<Path> baseDirectory, shared_ptr<ResourceLoader> delegate_)
    : baseDirectory(baseDirectory), delegate_(delegate_)
{
  if (baseDirectory == nullptr) {
    throw make_shared<NullPointerException>();
  }
  if (!Files::isDirectory(baseDirectory)) {
    throw invalid_argument(baseDirectory + L" is not a directory");
  }
  if (delegate_ == nullptr) {
    throw invalid_argument(L"delegate ResourceLoader may not be null");
  }
}

shared_ptr<InputStream> FilesystemResourceLoader::openResource(
    const wstring &resource) 
{
  try {
    return Files::newInputStream(baseDirectory->resolve(resource));
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (FileNotFoundException | NoSuchFileException fnfe) {
    return delegate_->openResource(resource);
  }
}

template <typename T>
T FilesystemResourceLoader::newInstance(const wstring &cname,
                                        type_info<T> &expectedType)
{
  return delegate_->newInstance(cname, expectedType);
}

template <typename T>
type_info FilesystemResourceLoader::findClass(const wstring &cname,
                                              type_info<T> &expectedType)
{
  return delegate_->findClass(cname, expectedType);
}
} // namespace org::apache::lucene::analysis::util