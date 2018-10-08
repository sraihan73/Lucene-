using namespace std;

#include "Lucene50RWSegmentInfoFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/CorruptIndexException.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IOContext.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"

namespace org::apache::lucene::codecs::lucene50
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using Version = org::apache::lucene::util::Version;

Lucene50RWSegmentInfoFormat::Lucene50RWSegmentInfoFormat() {}

shared_ptr<SegmentInfo> Lucene50RWSegmentInfoFormat::read(
    shared_ptr<Directory> dir, const wstring &segment,
    std::deque<char> &segmentID,
    shared_ptr<IOContext> context) 
{
  const wstring fileName = IndexFileNames::segmentFileName(
      segment, L"", Lucene50SegmentInfoFormat::SI_EXTENSION);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.ChecksumIndexInput
  // input = dir.openChecksumInput(fileName, context))
  {
    org::apache::lucene::store::ChecksumIndexInput input =
        dir->openChecksumInput(fileName, context);
    runtime_error priorE = nullptr;
    shared_ptr<SegmentInfo> si = nullptr;
    try {
      CodecUtil::checkIndexHeader(input, Lucene50SegmentInfoFormat::CODEC_NAME,
                                  Lucene50SegmentInfoFormat::VERSION_START,
                                  Lucene50SegmentInfoFormat::VERSION_CURRENT,
                                  segmentID, L"");
      shared_ptr<Version> *const version = Version::fromBits(
          input->readInt(), input->readInt(), input->readInt());

      constexpr int docCount = input->readInt();
      if (docCount < 0) {
        throw make_shared<CorruptIndexException>(
            L"invalid docCount: " + to_wstring(docCount), input);
      }
      constexpr bool isCompoundFile = input->readByte() == SegmentInfo::YES;

      const unordered_map<wstring, wstring> diagnostics =
          input->readMapOfStrings();
      shared_ptr<Set<wstring>> *const files = input->readSetOfStrings();
      const unordered_map<wstring, wstring> attributes =
          input->readMapOfStrings();

      si = make_shared<SegmentInfo>(dir, version, nullptr, segment, docCount,
                                    isCompoundFile, nullptr, diagnostics,
                                    segmentID, attributes, nullptr);
      si->setFiles(files);
    } catch (const runtime_error &exception) {
      priorE = exception;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      CodecUtil::checkFooter(input, priorE);
    }
    return si;
  }
}

void Lucene50RWSegmentInfoFormat::write(
    shared_ptr<Directory> dir, shared_ptr<SegmentInfo> si,
    shared_ptr<IOContext> ioContext) 
{
  const wstring fileName = IndexFileNames::segmentFileName(
      si->name, L"", Lucene50SegmentInfoFormat::SI_EXTENSION);

  assert(si->getIndexSort() == nullptr);

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput output =
  // dir.createOutput(fileName, ioContext))
  {
    org::apache::lucene::store::IndexOutput output =
        dir->createOutput(fileName, ioContext);
    // Only add the file once we've successfully created it, else IFD assert can
    // trip:
    si->addFile(fileName);
    CodecUtil::writeIndexHeader(output, Lucene50SegmentInfoFormat::CODEC_NAME,
                                Lucene50SegmentInfoFormat::VERSION_CURRENT,
                                si->getId(), L"");
    shared_ptr<Version> version = si->getVersion();
    if (version->major < 5) {
      throw invalid_argument(
          L"invalid major version: should be >= 5 but got: " +
          to_wstring(version->major) + L" segment=" + si);
    }
    // Write the Lucene version that created this segment, since 3.1
    output->writeInt(version->major);
    output->writeInt(version->minor);
    output->writeInt(version->bugfix);
    assert(version->prerelease == 0);
    output->writeInt(si->maxDoc());

    output->writeByte(static_cast<char>(
        si->getUseCompoundFile() ? SegmentInfo::YES : SegmentInfo::NO));
    output->writeMapOfStrings(si->getDiagnostics());
    shared_ptr<Set<wstring>> files = si->files();
    for (auto file : files) {
      if (IndexFileNames::parseSegmentName(file) != si->name) {
        throw invalid_argument(L"invalid files: expected segment=" + si->name +
                               L", got=" + files);
      }
    }
    output->writeSetOfStrings(files);
    output->writeMapOfStrings(si->getAttributes());

    CodecUtil::writeFooter(output);
  }
}

const wstring Lucene50RWSegmentInfoFormat::SI_EXTENSION = L"si";
const wstring Lucene50RWSegmentInfoFormat::CODEC_NAME = L"Lucene50SegmentInfo";
} // namespace org::apache::lucene::codecs::lucene50