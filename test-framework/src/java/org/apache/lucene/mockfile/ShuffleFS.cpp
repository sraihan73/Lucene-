using namespace std;

#include "ShuffleFS.h"

namespace org::apache::lucene::mockfile
{

ShuffleFS::ShuffleFS(shared_ptr<FileSystem> delegate_, int64_t seed)
    : FilterFileSystemProvider(L"shuffle://", delegate_), seed(seed)
{
}

template <typename T1>
// C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
// ORIGINAL LINE: @Override public
// java.nio.file.DirectoryStream<java.nio.file.Path>
// newDirectoryStream(java.nio.file.Path dir,
// java.nio.file.DirectoryStream.Filter<? super java.nio.file.Path> filter)
// throws java.io.IOException
shared_ptr<DirectoryStream<std::shared_ptr<Path>>>
ShuffleFS::newDirectoryStream(
    shared_ptr<Path> dir,
    shared_ptr<DirectoryStream::Filter<T1>> filter) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (java.nio.file.DirectoryStream<java.nio.file.Path> stream =
  // super.newDirectoryStream(dir, filter))
  {
    shared_ptr<java::nio::file::DirectoryStream<
        std::shared_ptr<java::nio::file::Path>>>
        stream = __super::newDirectoryStream(dir, filter);
    // read complete directory listing
    deque<std::shared_ptr<Path>> contents = deque<std::shared_ptr<Path>>();
    for (auto path : stream) {
      contents.push_back(path);
    }
    // sort first based only on filename
    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(contents, (path1, path2) ->
    // path1.getFileName().toString().compareTo(path2.getFileName().toString()));
    sort(contents.begin(), contents.end(), [&](path1, path2) {
      path1::getFileName()->toString()->compareTo(
          path2::getFileName()->toString());
    });
    // sort based on current class seed
    Collections::shuffle(contents, make_shared<Random>(seed));
    return make_shared<DirectoryStreamAnonymousInnerClass>(shared_from_this(),
                                                           contents);
  }
}

ShuffleFS::DirectoryStreamAnonymousInnerClass::
    DirectoryStreamAnonymousInnerClass(shared_ptr<ShuffleFS> outerInstance,
                                       deque<std::shared_ptr<Path>> &contents)
{
  this->outerInstance = outerInstance;
  this->contents = contents;
}

shared_ptr<Iterator<std::shared_ptr<Path>>>
ShuffleFS::DirectoryStreamAnonymousInnerClass::iterator()
{
  return contents.begin();
}

ShuffleFS::DirectoryStreamAnonymousInnerClass::
    ~DirectoryStreamAnonymousInnerClass()
{
}
} // namespace org::apache::lucene::mockfile