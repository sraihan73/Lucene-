using namespace std;

#include "NoMergeScheduler.h"

namespace org::apache::lucene::index
{
using OneMerge = org::apache::lucene::index::MergePolicy::OneMerge;
using Directory = org::apache::lucene::store::Directory;
const shared_ptr<MergeScheduler> NoMergeScheduler::INSTANCE =
    make_shared<NoMergeScheduler>();

NoMergeScheduler::NoMergeScheduler()
{
  // prevent instantiation
}

NoMergeScheduler::~NoMergeScheduler() {}

void NoMergeScheduler::merge(shared_ptr<IndexWriter> writer,
                             MergeTrigger trigger, bool newMergesFound)
{
}

shared_ptr<Directory> NoMergeScheduler::wrapForMerge(shared_ptr<OneMerge> merge,
                                                     shared_ptr<Directory> in_)
{
  return in_;
}

shared_ptr<MergeScheduler> NoMergeScheduler::clone()
{
  return shared_from_this();
}
} // namespace org::apache::lucene::index