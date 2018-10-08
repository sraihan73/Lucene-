using namespace std;

#include "PreCopyMergedSegmentWarmer.h"

namespace org::apache::lucene::replicator::nrt
{
using IndexReaderWarmer =
    org::apache::lucene::index::IndexWriter::IndexReaderWarmer;
using LeafReader = org::apache::lucene::index::LeafReader;
using SegmentCommitInfo = org::apache::lucene::index::SegmentCommitInfo;
using SegmentReader = org::apache::lucene::index::SegmentReader;

PreCopyMergedSegmentWarmer::PreCopyMergedSegmentWarmer(
    shared_ptr<PrimaryNode> primary)
    : primary(primary)
{
}

void PreCopyMergedSegmentWarmer::warm(shared_ptr<LeafReader> reader) throw(
    IOException)
{
  int64_t startNS = System::nanoTime();
  shared_ptr<SegmentCommitInfo> *const info =
      (std::static_pointer_cast<SegmentReader>(reader))->getSegmentInfo();
  // System.out.println("TEST: warm merged segment files " + info);
  unordered_map<wstring, std::shared_ptr<FileMetaData>> filesMetaData =
      unordered_map<wstring, std::shared_ptr<FileMetaData>>();
  for (auto fileName : info->files()) {
    shared_ptr<FileMetaData> metaData =
        primary->readLocalFileMetaData(fileName);
    assert(metaData != nullptr);
    assert(filesMetaData.find(fileName) != filesMetaData.end() == false);
    filesMetaData.emplace(fileName, metaData);
  }

  primary->preCopyMergedSegmentFiles(info, filesMetaData);
  primary->message(wstring::format(
      Locale::ROOT,
      L"top: done warm merge " + info + L": took %.3f sec, %.1f MB",
      (System::nanoTime() - startNS) / 1000000000.0,
      info->sizeInBytes() / 1024 / 1024.0));
  primary->finishedMergedFiles->addAll(filesMetaData.keySet());
}
} // namespace org::apache::lucene::replicator::nrt