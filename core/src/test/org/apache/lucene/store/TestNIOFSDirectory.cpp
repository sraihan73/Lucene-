using namespace std;

#include "TestNIOFSDirectory.h"

namespace org::apache::lucene::store
{

shared_ptr<Directory>
TestNIOFSDirectory::getDirectory(shared_ptr<Path> path) 
{
  return make_shared<NIOFSDirectory>(path);
}
} // namespace org::apache::lucene::store