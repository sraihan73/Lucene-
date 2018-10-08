using namespace std;

#include "WindowsDirectory.h"

namespace org::apache::lucene::store
{
using Directory = org::apache::lucene::store::Directory;

WindowsDirectory::StaticConstructor::StaticConstructor()
{
  System::loadLibrary(L"WindowsDirectory");
}

WindowsDirectory::StaticConstructor WindowsDirectory::staticConstructor;

WindowsDirectory::WindowsDirectory(
    shared_ptr<Path> path,
    shared_ptr<LockFactory> lockFactory) 
    : FSDirectory(path, lockFactory)
{
}

WindowsDirectory::WindowsDirectory(shared_ptr<Path> path) 
    : WindowsDirectory(path, FSLockFactory::getDefault())
{
}

shared_ptr<IndexInput>
WindowsDirectory::openInput(const wstring &name,
                            shared_ptr<IOContext> context) 
{
  ensureOpen();
  return make_shared<WindowsIndexInput>(
      getDirectory()->resolve(name),
      max(BufferedIndexInput::bufferSize(context), DEFAULT_BUFFERSIZE));
}

// C++ TODO: There is no native C++ equivalent to 'toString':
WindowsDirectory::WindowsIndexInput::WindowsIndexInput(
    shared_ptr<Path> file, int bufferSize) 
    : BufferedIndexInput(L"WindowsIndexInput(path=\"" + file + L"\")",
                         bufferSize),
      fd(WindowsDirectory::open(file->toString())),
      length(WindowsDirectory::length(fd))
{
  isOpen = true;
}

void WindowsDirectory::WindowsIndexInput::readInternal(
    std::deque<char> &b, int offset, int length) 
{
  int bytesRead;
  try {
    bytesRead = WindowsDirectory::read(fd, b, offset, length, getFilePointer());
  } catch (const IOException &ioe) {
    throw make_shared<IOException>(
        ioe->getMessage() + L": " + shared_from_this(), ioe);
  }

  if (bytesRead != length) {
    // C++ TODO: The following line could not be converted:
    throw java.io.EOFException(L"read past EOF: " + this);
  }
}

void WindowsDirectory::WindowsIndexInput::seekInternal(int64_t pos) throw(
    IOException)
{
}

// C++ WARNING: The following method was originally marked 'synchronized':
WindowsDirectory::WindowsIndexInput::~WindowsIndexInput()
{
  // NOTE: we synchronize and track "isOpen" because Lucene sometimes closes IIs
  // twice!
  if (!isClone && isOpen) {
    WindowsDirectory::close(fd);
    isOpen = false;
  }
}

int64_t WindowsDirectory::WindowsIndexInput::length() { return length_; }

shared_ptr<WindowsIndexInput> WindowsDirectory::WindowsIndexInput::clone()
{
  shared_ptr<WindowsIndexInput> clone =
      std::static_pointer_cast<WindowsIndexInput>(BufferedIndexInput::clone());
  clone->isClone = true;
  return clone;
}
} // namespace org::apache::lucene::store