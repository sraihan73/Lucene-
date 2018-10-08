using namespace std;

#include "Directory.h"

namespace org::apache::lucene::store
{
using IOUtils = org::apache::lucene::util::IOUtils;

shared_ptr<ChecksumIndexInput>
Directory::openChecksumInput(const wstring &name,
                             shared_ptr<IOContext> context) 
{
  return make_shared<BufferedChecksumIndexInput>(openInput(name, context));
}

wstring Directory::toString()
{
  return getClass().getSimpleName() + L'@' + Integer::toHexString(hashCode());
}

void Directory::copyFrom(shared_ptr<Directory> from, const wstring &src,
                         const wstring &dest,
                         shared_ptr<IOContext> context) 
{
  bool success = false;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (IndexInput is = from.openInput(src,
  // context); IndexOutput os = createOutput(dest, context))
  {
    IndexInput is = from->openInput(src, context);
    IndexOutput os = createOutput(dest, context);
    try {
      os->copyBytes(is, is->length());
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!success) {
        IOUtils::deleteFilesIgnoringExceptions(shared_from_this(), {dest});
      }
    }
  }
}

void Directory::ensureOpen()  {}

shared_ptr<Set<wstring>> Directory::getPendingDeletions() 
{
  return Collections::emptySet();
}
} // namespace org::apache::lucene::store