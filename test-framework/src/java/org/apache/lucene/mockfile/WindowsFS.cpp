using namespace std;

#include "WindowsFS.h"

namespace org::apache::lucene::mockfile
{

WindowsFS::WindowsFS(shared_ptr<FileSystem> delegate_)
    : HandleTrackingFS(L"windows://", delegate_)
{
}

any WindowsFS::getKey(shared_ptr<Path> existing) 
{
  shared_ptr<BasicFileAttributeView> view =
      Files::getFileAttributeView(existing, BasicFileAttributeView::typeid);
  shared_ptr<BasicFileAttributes> attributes = view->readAttributes();
  return attributes->fileKey();
}

void WindowsFS::onOpen(shared_ptr<Path> path, any stream) 
{
  {
    lock_guard<mutex> lock(openFiles);
    constexpr any key = getKey(path);
    // we have to read the key under the lock otherwise me might leak the
    // openFile handle if we concurrently delete or move this file.
    unordered_map<std::shared_ptr<Path>, int> pathMap =
        openFiles.computeIfAbsent(key, [&](any k) { unordered_map<>(); });
    pathMap.emplace(
        path, pathMap.computeIfAbsent(path, [&](any p) { 0; }).intValue() + 1);
  }
}

void WindowsFS::onClose(shared_ptr<Path> path, any stream) 
{
  any key = getKey(path); // here we can read this outside of the lock
  {
    lock_guard<mutex> lock(openFiles);
    unordered_map<std::shared_ptr<Path>, int> pathMap = openFiles[key];
    assert(pathMap.size() > 0);
    assert(pathMap.find(path) != pathMap.end());
    optional<int> v = pathMap[path];
    if (v) {
      if (v.value() == 1) {
        pathMap.erase(path);
      } else {
        v = static_cast<Integer>(v.value() - 1);
        pathMap.emplace(path, v);
      }
    }
    if (pathMap.empty()) {
      openFiles.erase(key);
    }
  }
}

any WindowsFS::getKeyOrNull(shared_ptr<Path> path)
{
  try {
    return getKey(path);
  } catch (const runtime_error &ignore) {
    // we don't care if the file doesn't exist
  }
  return nullptr;
}

void WindowsFS::checkDeleteAccess(shared_ptr<Path> path) 
{
  any key = getKeyOrNull(path);
  if (key != nullptr) {
    {
      lock_guard<mutex> lock(openFiles);
      if (openFiles.find(key) != openFiles.end()) {
        // C++ TODO: The following line could not be converted:
        throw java.io.IOException(L"access denied: " + path);
      }
    }
  }
}

void WindowsFS::delete_(shared_ptr<Path> path) 
{
  {
    lock_guard<mutex> lock(openFiles);
    checkDeleteAccess(path);
    HandleTrackingFS::delete (path);
  }
}

void WindowsFS::move(shared_ptr<Path> source, shared_ptr<Path> target,
                     deque<CopyOption> &options) 
{
  {
    lock_guard<mutex> lock(openFiles);
    checkDeleteAccess(source);
    any key = getKeyOrNull(target);
    HandleTrackingFS::move(source, target, {options});
    if (key != nullptr) {
      any newKey = getKey(target);
      if (newKey.equals(key) == false) {
        // we need to transfer ownership here if we have open files on this file
        // since the getKey() method will return a different i-node next time we
        // call it with the target path and our onClose method will trip an
        // assert
        unordered_map<std::shared_ptr<Path>, int> map_obj = openFiles[key];
        if (map_obj.size() > 0) {
          optional<int> v = map_obj.erase(target);
          if (v) {
            unordered_map<std::shared_ptr<Path>, int> pathIntegerMap =
                openFiles.computeIfAbsent(newKey,
                                          [&](any k) { unordered_map<>(); });
            optional<int> existingValue =
                pathIntegerMap.getOrDefault(target, 0);
            pathIntegerMap.emplace(target, existingValue + v);
          }
          if (map_obj.empty()) {
            openFiles.erase(key);
          }
        }
      }
    }
  }
}

bool WindowsFS::deleteIfExists(shared_ptr<Path> path) 
{
  {
    lock_guard<mutex> lock(openFiles);
    checkDeleteAccess(path);
    return HandleTrackingFS::deleteIfExists(path);
  }
}
} // namespace org::apache::lucene::mockfile