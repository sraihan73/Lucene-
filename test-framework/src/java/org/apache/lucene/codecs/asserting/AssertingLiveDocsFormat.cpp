using namespace std;

#include "AssertingLiveDocsFormat.h"

namespace org::apache::lucene::codecs::asserting
{
using LiveDocsFormat = org::apache::lucene::codecs::LiveDocsFormat;
using SegmentCommitInfo = org::apache::lucene::index::SegmentCommitInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using Bits = org::apache::lucene::util::Bits;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<Bits> AssertingLiveDocsFormat::readLiveDocs(
    shared_ptr<Directory> dir, shared_ptr<SegmentCommitInfo> info,
    shared_ptr<IOContext> context) 
{
  shared_ptr<Bits> raw = in_->readLiveDocs(dir, info, context);
  assert(raw != nullptr);
  check(raw, info->info->maxDoc(), info->getDelCount());
  return make_shared<AssertingBits>(raw);
}

void AssertingLiveDocsFormat::writeLiveDocs(
    shared_ptr<Bits> bits, shared_ptr<Directory> dir,
    shared_ptr<SegmentCommitInfo> info, int newDelCount,
    shared_ptr<IOContext> context) 
{
  check(bits, info->info->maxDoc(), info->getDelCount() + newDelCount);
  in_->writeLiveDocs(bits, dir, info, newDelCount, context);
}

void AssertingLiveDocsFormat::check(shared_ptr<Bits> bits, int expectedLength,
                                    int expectedDeleteCount)
{
  assert(bits->length() == expectedLength);
  int deletedCount = 0;
  for (int i = 0; i < bits->length(); i++) {
    if (!bits->get(i)) {
      deletedCount++;
    }
  }
  assert((deletedCount == expectedDeleteCount,
          L"deleted: " + to_wstring(deletedCount) + L" != expected: " +
              to_wstring(expectedDeleteCount)));
}

void AssertingLiveDocsFormat::files(
    shared_ptr<SegmentCommitInfo> info,
    shared_ptr<deque<wstring>> files) 
{
  in_->files(info, files);
}

wstring AssertingLiveDocsFormat::toString()
{
  return L"Asserting(" + in_ + L")";
}

AssertingLiveDocsFormat::AssertingBits::AssertingBits(shared_ptr<Bits> in_)
    : in_(in_)
{
  assert(in_->length() >= 0);
}

bool AssertingLiveDocsFormat::AssertingBits::get(int index)
{
  assert(index >= 0);
  assert((index < in_->length(), L"index=" + to_wstring(index) +
                                     L" vs in.length()=" +
                                     to_wstring(in_->length())));
  return in_->get(index);
}

int AssertingLiveDocsFormat::AssertingBits::length() { return in_->length(); }

wstring AssertingLiveDocsFormat::AssertingBits::toString()
{
  return L"Asserting(" + in_ + L")";
}
} // namespace org::apache::lucene::codecs::asserting