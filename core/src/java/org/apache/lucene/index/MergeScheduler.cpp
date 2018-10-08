using namespace std;

#include "MergeScheduler.h"

namespace org::apache::lucene::index
{
using OneMerge = org::apache::lucene::index::MergePolicy::OneMerge;
using Directory = org::apache::lucene::store::Directory;
using RateLimitedIndexOutput =
    org::apache::lucene::store::RateLimitedIndexOutput;
using InfoStream = org::apache::lucene::util::InfoStream;

MergeScheduler::MergeScheduler() {}

shared_ptr<Directory> MergeScheduler::wrapForMerge(shared_ptr<OneMerge> merge,
                                                   shared_ptr<Directory> in_)
{
  // A no-op by default.
  return in_;
}

void MergeScheduler::setInfoStream(shared_ptr<InfoStream> infoStream)
{
  this->infoStream = infoStream;
}

bool MergeScheduler::verbose()
{
  return infoStream != nullptr && infoStream->isEnabled(L"MS");
}

void MergeScheduler::message(const wstring &message)
{
  infoStream->message(L"MS", message);
}
} // namespace org::apache::lucene::index