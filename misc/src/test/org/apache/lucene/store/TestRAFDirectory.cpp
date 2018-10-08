using namespace std;

#include "TestRAFDirectory.h"

namespace org::apache::lucene::store
{

shared_ptr<Directory>
TestRAFDirectory::getDirectory(shared_ptr<Path> path) 
{
  return make_shared<RAFDirectory>(path);
}
} // namespace org::apache::lucene::store