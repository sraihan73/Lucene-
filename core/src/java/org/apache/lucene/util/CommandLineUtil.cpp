using namespace std;

#include "CommandLineUtil.h"

namespace org::apache::lucene::util
{
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using FSLockFactory = org::apache::lucene::store::FSLockFactory;
using LockFactory = org::apache::lucene::store::LockFactory;

CommandLineUtil::CommandLineUtil() {}

shared_ptr<FSDirectory>
CommandLineUtil::newFSDirectory(const wstring &clazzName, shared_ptr<Path> path)
{
  return newFSDirectory(clazzName, path, FSLockFactory::getDefault());
}

shared_ptr<FSDirectory>
CommandLineUtil::newFSDirectory(const wstring &clazzName, shared_ptr<Path> path,
                                shared_ptr<LockFactory> lf)
{
  try {
    constexpr type_info clazz = loadFSDirectoryClass(clazzName);
    return newFSDirectory(clazz, path, lf);
  } catch (const ClassNotFoundException &e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new
    // IllegalArgumentException(org.apache.lucene.store.FSDirectory.class.getSimpleName()
    // + " implementation not found: " + clazzName, e);
    throw invalid_argument(FSDirectory::typeid->getSimpleName() +
                           L" implementation not found: " + clazzName);
  } catch (const ClassCastException &e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new IllegalArgumentException(clazzName + " is not a
    // " + org.apache.lucene.store.FSDirectory.class.getSimpleName() + "
    // implementation", e);
    throw invalid_argument(clazzName + L" is not a " +
                           FSDirectory::typeid->getSimpleName() +
                           L" implementation");
  } catch (const NoSuchMethodException &e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new IllegalArgumentException(clazzName + "
    // constructor with " + java.nio.file.Path.class.getSimpleName() + " as
    // parameter not found", e);
    throw invalid_argument(clazzName + L" constructor with " +
                           Path::typeid->getSimpleName() +
                           L" as parameter not found");
  } catch (const runtime_error &e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new IllegalArgumentException("Error creating " +
    // clazzName + " instance", e);
    throw invalid_argument(L"Error creating " + clazzName + L" instance");
  }
}

type_info CommandLineUtil::loadDirectoryClass(const wstring &clazzName) throw(
    ClassNotFoundException)
{
  return type_info::forName(adjustDirectoryClassName(clazzName))
      .asSubclass(Directory::typeid);
}

type_info CommandLineUtil::loadFSDirectoryClass(const wstring &clazzName) throw(
    ClassNotFoundException)
{
  return type_info::forName(adjustDirectoryClassName(clazzName))
      .asSubclass(FSDirectory::typeid);
}

wstring CommandLineUtil::adjustDirectoryClassName(const wstring &clazzName)
{
  if (clazzName == L"" || StringHelper::trim(clazzName)->length() == 0) {
    throw invalid_argument(L"The " + FSDirectory::typeid->getSimpleName() +
                           L" implementation must not be null or empty");
  }

  if (clazzName.find(L".") ==
      wstring::npos) { // if not fully qualified, assume .store
    clazzName = Directory::typeid->getPackage().getName() + L"." + clazzName;
  }
  return clazzName;
}

shared_ptr<FSDirectory> CommandLineUtil::newFSDirectory(
    type_info clazz, shared_ptr<Path> path) 
{
  return newFSDirectory(clazz, path, FSLockFactory::getDefault());
}

shared_ptr<FSDirectory> CommandLineUtil::newFSDirectory(
    type_info clazz, shared_ptr<Path> path,
    shared_ptr<LockFactory> lf) 
{
  // Assuming every FSDirectory has a ctor(Path):
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: Constructor<? extends org.apache.lucene.store.FSDirectory>
  // ctor = clazz.getConstructor(java.nio.file.Path.class,
  // org.apache.lucene.store.LockFactory.class);
  shared_ptr < Constructor <
      ? extends FSDirectory >> ctor =
            clazz.getConstructor(Path::typeid, LockFactory::typeid);
  return ctor->newInstance(path, lf);
}
} // namespace org::apache::lucene::util