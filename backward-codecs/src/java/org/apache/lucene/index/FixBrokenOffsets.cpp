using namespace std;

#include "FixBrokenOffsets.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/CodecReader.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/Fields.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/LeafReader.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/SegmentInfos.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/SlowCodecReaderWrapper.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../core/src/java/org/apache/lucene/store/FSDirectory.h"
#include "../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"

namespace org::apache::lucene::index
{
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using IOUtils = org::apache::lucene::util::IOUtils;
using org::apache::lucene::util::SuppressForbidden;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressForbidden(reason = "System.out required: command line
// tool") public static void main(std::wstring[] args) throws java.io.IOException
void FixBrokenOffsets::main(std::deque<wstring> &args) 
{
  if (args.size() < 2) {
    System::err::println(L"Usage: FixBrokenOffsetse <srcDir> <destDir>");
    return;
  }
  shared_ptr<Path> srcPath = Paths->get(args[0]);
  if (!Files::exists(srcPath)) {
    throw runtime_error(L"srcPath " + srcPath->toAbsolutePath() +
                        L" doesn't exist");
  }
  shared_ptr<Path> destPath = Paths->get(args[1]);
  if (Files::exists(destPath)) {
    throw runtime_error(L"destPath " + destPath->toAbsolutePath() +
                        L" already exists; please remove it and re-run");
  }
  shared_ptr<Directory> srcDir = FSDirectory::open(srcPath);
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(srcDir);

  deque<std::shared_ptr<LeafReaderContext>> leaves = reader->leaves();
  std::deque<std::shared_ptr<CodecReader>> filtered(leaves.size());
  for (int i = 0; i < leaves.size(); i++) {
    filtered[i] = SlowCodecReaderWrapper::wrap(
        make_shared<FilterLeafReaderAnonymousInnerClass>(leaves[i]->reader()));
  }

  shared_ptr<Directory> destDir = FSDirectory::open(destPath);
  // We need to maintain the same major version
  int createdMajor =
      SegmentInfos::readLatestCommit(srcDir)->getIndexCreatedVersionMajor();
  (make_shared<SegmentInfos>(createdMajor))->commit(destDir);
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(destDir, make_shared<IndexWriterConfig>());
  writer->addIndexes(filtered);
  IOUtils::close({writer, reader, srcDir, destDir});
}

FixBrokenOffsets::FilterLeafReaderAnonymousInnerClass::
    FilterLeafReaderAnonymousInnerClass(
        shared_ptr<org::apache::lucene::index::LeafReader> reader)
    : FilterLeafReader(reader)
{
}

shared_ptr<Fields>
FixBrokenOffsets::FilterLeafReaderAnonymousInnerClass::getTermVectors(
    int docID) 
{
  shared_ptr<Fields> termVectors = in_::getTermVectors(docID);
  if (termVectors->empty()) {
    return nullptr;
  }
  return make_shared<FilterFieldsAnonymousInnerClass>(shared_from_this(),
                                                      termVectors);
}

FixBrokenOffsets::FilterLeafReaderAnonymousInnerClass::
    FilterFieldsAnonymousInnerClass::FilterFieldsAnonymousInnerClass(
        shared_ptr<FilterLeafReaderAnonymousInnerClass> outerInstance,
        shared_ptr<org::apache::lucene::index::Fields> termVectors)
    : FilterFields(termVectors)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Terms> FixBrokenOffsets::FilterLeafReaderAnonymousInnerClass::
    FilterFieldsAnonymousInnerClass::terms(const wstring &field) throw(
        IOException)
{
  return make_shared<FilterTermsAnonymousInnerClass>(
      shared_from_this(), FilterFields::terms(field));
}

FixBrokenOffsets::FilterLeafReaderAnonymousInnerClass::
    FilterFieldsAnonymousInnerClass::FilterTermsAnonymousInnerClass::
        FilterTermsAnonymousInnerClass(
            shared_ptr<FilterFieldsAnonymousInnerClass> outerInstance,
            shared_ptr<UnknownType> terms)
    : FilterTerms(terms)
{
  this->outerInstance = outerInstance;
}

shared_ptr<TermsEnum> FixBrokenOffsets::FilterLeafReaderAnonymousInnerClass::
    FilterFieldsAnonymousInnerClass::FilterTermsAnonymousInnerClass::
        iterator() 
{
  return make_shared<FilterTermsEnumAnonymousInnerClass>(shared_from_this(),
                                                         FilterTerms::begin());
}

FixBrokenOffsets::FilterLeafReaderAnonymousInnerClass::
    FilterFieldsAnonymousInnerClass::FilterTermsAnonymousInnerClass::
        FilterTermsEnumAnonymousInnerClass::FilterTermsEnumAnonymousInnerClass(
            shared_ptr<FilterTermsAnonymousInnerClass> outerInstance,
            shared_ptr<UnknownType> iterator)
    : FilterTermsEnum(iterator)
{
  this->outerInstance = outerInstance;
}

shared_ptr<PostingsEnum> FixBrokenOffsets::FilterLeafReaderAnonymousInnerClass::
    FilterFieldsAnonymousInnerClass::FilterTermsAnonymousInnerClass::
        FilterTermsEnumAnonymousInnerClass::postings(
            shared_ptr<PostingsEnum> reuse, int flags) 
{
  return make_shared<FilterPostingsEnumAnonymousInnerClass>(
      shared_from_this(), FilterTermsEnum::postings(reuse, flags));
}

FixBrokenOffsets::FilterLeafReaderAnonymousInnerClass::
    FilterFieldsAnonymousInnerClass::FilterTermsAnonymousInnerClass::
        FilterTermsEnumAnonymousInnerClass::
            FilterPostingsEnumAnonymousInnerClass::
                FilterPostingsEnumAnonymousInnerClass(
                    shared_ptr<FilterTermsEnumAnonymousInnerClass>
                        outerInstance,
                    shared_ptr<UnknownType> postings)
    : FilterPostingsEnum(postings)
{
  this->outerInstance = outerInstance;
  nextLastStartOffset = 0;
  lastStartOffset = 0;
}

int FixBrokenOffsets::FilterLeafReaderAnonymousInnerClass::
    FilterFieldsAnonymousInnerClass::FilterTermsAnonymousInnerClass::
        FilterTermsEnumAnonymousInnerClass::
            FilterPostingsEnumAnonymousInnerClass::nextPosition() throw(
                IOException)
{
  int pos = FilterPostingsEnum::nextPosition();
  lastStartOffset = nextLastStartOffset;
  nextLastStartOffset = startOffset();
  return pos;
}

int FixBrokenOffsets::FilterLeafReaderAnonymousInnerClass::
    FilterFieldsAnonymousInnerClass::FilterTermsAnonymousInnerClass::
        FilterTermsEnumAnonymousInnerClass::
            FilterPostingsEnumAnonymousInnerClass::startOffset() throw(
                IOException)
{
  int offset = FilterPostingsEnum::startOffset();
  if (offset < lastStartOffset) {
    offset = lastStartOffset;
  }
  return offset;
}

int FixBrokenOffsets::FilterLeafReaderAnonymousInnerClass::
    FilterFieldsAnonymousInnerClass::FilterTermsAnonymousInnerClass::
        FilterTermsEnumAnonymousInnerClass::
            FilterPostingsEnumAnonymousInnerClass::endOffset() throw(
                IOException)
{
  int offset = FilterPostingsEnum::endOffset();
  if (offset < lastStartOffset) {
    offset = lastStartOffset;
  }
  return offset;
}

shared_ptr<CacheHelper>
FixBrokenOffsets::FilterLeafReaderAnonymousInnerClass::getCoreCacheHelper()
{
  return nullptr;
}

shared_ptr<CacheHelper>
FixBrokenOffsets::FilterLeafReaderAnonymousInnerClass::getReaderCacheHelper()
{
  return nullptr;
}
} // namespace org::apache::lucene::index