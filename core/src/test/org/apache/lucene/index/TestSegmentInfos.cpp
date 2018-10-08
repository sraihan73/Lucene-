using namespace std;

#include "TestSegmentInfos.h"

namespace org::apache::lucene::index
{
using Codec = org::apache::lucene::codecs::Codec;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using IOContext = org::apache::lucene::store::IOContext;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using StringHelper = org::apache::lucene::util::StringHelper;
using Version = org::apache::lucene::util::Version;

void TestSegmentInfos::testIllegalCreatedVersion()
{
  invalid_argument e = expectThrows(invalid_argument::typeid,
                                    [&]() { make_shared<SegmentInfos>(5); });
  assertEquals(L"indexCreatedVersionMajor must be >= 6, got: 5", e.what());
  e = expectThrows(invalid_argument::typeid,
                   [&]() { make_shared<SegmentInfos>(8); });
  assertEquals(L"indexCreatedVersionMajor is in the future: 8", e.what());
}

void TestSegmentInfos::testVersionsNoSegments() 
{
  shared_ptr<SegmentInfos> sis =
      make_shared<SegmentInfos>(Version::LATEST->major);
  shared_ptr<BaseDirectoryWrapper> dir = newDirectory();
  dir->setCheckIndexOnClose(false);
  sis->commit(dir);
  sis = SegmentInfos::readLatestCommit(dir);
  assertNull(sis->getMinSegmentLuceneVersion());
  assertEquals(Version::LATEST, sis->getCommitLuceneVersion());
  delete dir;
}

void TestSegmentInfos::testVersionsOneSegment() 
{
  shared_ptr<BaseDirectoryWrapper> dir = newDirectory();
  dir->setCheckIndexOnClose(false);
  std::deque<char> id = StringHelper::randomId();
  shared_ptr<Codec> codec = Codec::getDefault();

  shared_ptr<SegmentInfos> sis =
      make_shared<SegmentInfos>(Version::LATEST->major);
  shared_ptr<SegmentInfo> info = make_shared<SegmentInfo>(
      dir, Version::LUCENE_7_0_0, Version::LUCENE_7_0_0, L"_0", 1, false,
      Codec::getDefault(), Collections::emptyMap<wstring, wstring>(), id,
      Collections::emptyMap<wstring, wstring>(), nullptr);
  info->setFiles(Collections::emptySet<wstring>());
  codec->segmentInfoFormat()->write(dir, info, IOContext::DEFAULT);
  shared_ptr<SegmentCommitInfo> commitInfo =
      make_shared<SegmentCommitInfo>(info, 0, 0, -1, -1, -1);

  sis->push_back(commitInfo);
  sis->commit(dir);
  sis = SegmentInfos::readLatestCommit(dir);
  assertEquals(Version::LUCENE_7_0_0, sis->getMinSegmentLuceneVersion());
  assertEquals(Version::LATEST, sis->getCommitLuceneVersion());
  delete dir;
}

void TestSegmentInfos::testVersionsTwoSegments() 
{
  shared_ptr<BaseDirectoryWrapper> dir = newDirectory();
  dir->setCheckIndexOnClose(false);
  std::deque<char> id = StringHelper::randomId();
  shared_ptr<Codec> codec = Codec::getDefault();

  shared_ptr<SegmentInfos> sis =
      make_shared<SegmentInfos>(Version::LATEST->major);
  shared_ptr<SegmentInfo> info = make_shared<SegmentInfo>(
      dir, Version::LUCENE_7_0_0, Version::LUCENE_7_0_0, L"_0", 1, false,
      Codec::getDefault(), Collections::emptyMap<wstring, wstring>(), id,
      Collections::emptyMap<wstring, wstring>(), nullptr);
  info->setFiles(Collections::emptySet<wstring>());
  codec->segmentInfoFormat()->write(dir, info, IOContext::DEFAULT);
  shared_ptr<SegmentCommitInfo> commitInfo =
      make_shared<SegmentCommitInfo>(info, 0, 0, -1, -1, -1);
  sis->push_back(commitInfo);

  info = make_shared<SegmentInfo>(
      dir, Version::LUCENE_7_0_0, Version::LUCENE_7_0_0, L"_1", 1, false,
      Codec::getDefault(), Collections::emptyMap<wstring, wstring>(), id,
      Collections::emptyMap<wstring, wstring>(), nullptr);
  info->setFiles(Collections::emptySet<wstring>());
  codec->segmentInfoFormat()->write(dir, info, IOContext::DEFAULT);
  commitInfo = make_shared<SegmentCommitInfo>(info, 0, 0, -1, -1, -1);
  sis->push_back(commitInfo);

  sis->commit(dir);
  sis = SegmentInfos::readLatestCommit(dir);
  assertEquals(Version::LUCENE_7_0_0, sis->getMinSegmentLuceneVersion());
  assertEquals(Version::LATEST, sis->getCommitLuceneVersion());
  delete dir;
}
} // namespace org::apache::lucene::index