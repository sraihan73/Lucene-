using namespace std;

#include "IndexCommit.h"

namespace org::apache::lucene::index
{
using Directory = org::apache::lucene::store::Directory;

IndexCommit::IndexCommit() {}

bool IndexCommit::equals(any other)
{
  if (std::dynamic_pointer_cast<IndexCommit>(other) != nullptr) {
    shared_ptr<IndexCommit> otherCommit =
        any_cast<std::shared_ptr<IndexCommit>>(other);
    return otherCommit->getDirectory() == getDirectory() &&
           otherCommit->getGeneration() == getGeneration();
  } else {
    return false;
  }
}

int IndexCommit::hashCode()
{
  return getDirectory()->hashCode() +
         static_cast<int64_t>(getGeneration()).hashCode();
}

int IndexCommit::compareTo(shared_ptr<IndexCommit> commit)
{
  if (getDirectory() != commit->getDirectory()) {
    throw make_shared<UnsupportedOperationException>(
        L"cannot compare IndexCommits from different Directory instances");
  }

  int64_t gen = getGeneration();
  int64_t comgen = commit->getGeneration();
  return Long::compare(gen, comgen);
}

shared_ptr<StandardDirectoryReader> IndexCommit::getReader() { return nullptr; }
} // namespace org::apache::lucene::index