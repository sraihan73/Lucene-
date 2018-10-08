using namespace std;

#include "HardlinkCopyDirectoryWrapper.h"

namespace org::apache::lucene::store
{

HardlinkCopyDirectoryWrapper::HardlinkCopyDirectoryWrapper(
    shared_ptr<Directory> in_)
    : FilterDirectory(in_)
{
}

void HardlinkCopyDirectoryWrapper::copyFrom(
    shared_ptr<Directory> from, const wstring &srcFile, const wstring &destFile,
    shared_ptr<IOContext> context) 
{
  shared_ptr<Directory> *const fromUnwrapped = FilterDirectory::unwrap(from);
  shared_ptr<Directory> *const toUnwrapped =
      FilterDirectory::unwrap(shared_from_this());
  // try to unwrap to FSDirectory - we might be able to just create hard-links
  // of these files and save copying the entire file.
  runtime_error suppressedException = nullptr;
  bool tryCopy = true;
  if (std::dynamic_pointer_cast<FSDirectory>(fromUnwrapped) != nullptr &&
      std::dynamic_pointer_cast<FSDirectory>(toUnwrapped) != nullptr) {
    shared_ptr<Path> *const fromPath =
        (std::static_pointer_cast<FSDirectory>(fromUnwrapped))->getDirectory();
    shared_ptr<Path> *const toPath =
        (std::static_pointer_cast<FSDirectory>(toUnwrapped))->getDirectory();

    if (Files::isReadable(fromPath->resolve(srcFile)) &&
        Files::isWritable(toPath)) {
      // only try hardlinks if we have permission to access the files
      // if not super.copyFrom() will give us the right exceptions
      suppressedException = AccessController::doPrivileged(
          std::static_pointer_cast<PrivilegedAction<runtime_error>>[&]() {
            try {
              Files::createLink(toPath->resolve(destFile),
                                fromPath->resolve(srcFile));
            } catch (FileNotFoundException | NoSuchFileException |
                     FileAlreadyExistsException ex) {
              return ex;
            } catch (IOException | UnsupportedOperationException |
                     SecurityException ex) {
              return ex;
            }
            return nullptr;
          });
      tryCopy = suppressedException != nullptr;
    }
  }
  if (tryCopy) {
    try {
      FilterDirectory::copyFrom(from, srcFile, destFile, context);
    } catch (const runtime_error &ex) {
      if (suppressedException != nullptr) {
        ex.addSuppressed(suppressedException);
      }
      throw ex;
    }
  }
}
} // namespace org::apache::lucene::store