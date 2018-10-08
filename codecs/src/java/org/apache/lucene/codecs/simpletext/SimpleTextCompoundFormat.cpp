using namespace std;

#include "SimpleTextCompoundFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/CorruptIndexException.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IOContext.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexOutput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/Lock.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/StringHelper.h"
#include "SimpleTextUtil.h"

namespace org::apache::lucene::codecs::simpletext
{
using CompoundFormat = org::apache::lucene::codecs::CompoundFormat;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using Lock = org::apache::lucene::store::Lock;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using StringHelper = org::apache::lucene::util::StringHelper;

SimpleTextCompoundFormat::SimpleTextCompoundFormat() {}

shared_ptr<Directory> SimpleTextCompoundFormat::getCompoundReader(
    shared_ptr<Directory> dir, shared_ptr<SegmentInfo> si,
    shared_ptr<IOContext> context) 
{
  wstring dataFile =
      IndexFileNames::segmentFileName(si->name, L"", DATA_EXTENSION);
  shared_ptr<IndexInput> *const in_ = dir->openInput(dataFile, context);

  shared_ptr<BytesRefBuilder> scratch = make_shared<BytesRefBuilder>();

  // first get to TOC:
  shared_ptr<DecimalFormat> df = make_shared<DecimalFormat>(
      OFFSETPATTERN, DecimalFormatSymbols::getInstance(Locale::ROOT));
  int64_t pos = in_->length() - TABLEPOS->length - OFFSETPATTERN.length() - 1;
  in_->seek(pos);
  SimpleTextUtil::readLine(in_, scratch);
  assert((StringHelper::startsWith(scratch->get(), TABLEPOS)));
  int64_t tablePos = -1;
  try {
    tablePos = df->parse(stripPrefix(scratch, TABLEPOS)).longValue();
  } catch (const ParseException &e) {
    throw make_shared<CorruptIndexException>(
        L"can't parse CFS trailer, got: " + scratch->get().utf8ToString(), in_);
  }

  // seek to TOC and read it
  in_->seek(tablePos);
  SimpleTextUtil::readLine(in_, scratch);
  assert((StringHelper::startsWith(scratch->get(), TABLE)));
  int numEntries = stoi(stripPrefix(scratch, TABLE));

  const std::deque<wstring> fileNames = std::deque<wstring>(numEntries);
  const std::deque<int64_t> startOffsets =
      std::deque<int64_t>(numEntries);
  const std::deque<int64_t> endOffsets = std::deque<int64_t>(numEntries);

  for (int i = 0; i < numEntries; i++) {
    SimpleTextUtil::readLine(in_, scratch);
    assert((StringHelper::startsWith(scratch->get(), TABLENAME)));
    fileNames[i] = si->name + IndexFileNames::stripSegmentName(
                                  stripPrefix(scratch, TABLENAME));

    if (i > 0) {
      // files must be unique and in sorted order
      assert(fileNames[i].compare(fileNames[i - 1]) > 0);
    }

    SimpleTextUtil::readLine(in_, scratch);
    assert((StringHelper::startsWith(scratch->get(), TABLESTART)));
    startOffsets[i] =
        StringHelper::fromString<int64_t>(stripPrefix(scratch, TABLESTART));

    SimpleTextUtil::readLine(in_, scratch);
    assert((StringHelper::startsWith(scratch->get(), TABLEEND)));
    endOffsets[i] =
        StringHelper::fromString<int64_t>(stripPrefix(scratch, TABLEEND));
  }

  return make_shared<DirectoryAnonymousInnerClass>(
      shared_from_this(), context, in_, fileNames, startOffsets, endOffsets);
}

SimpleTextCompoundFormat::DirectoryAnonymousInnerClass::
    DirectoryAnonymousInnerClass(
        shared_ptr<SimpleTextCompoundFormat> outerInstance,
        shared_ptr<IOContext> context, shared_ptr<IndexInput> in_,
        deque<wstring> &fileNames, deque<int64_t> &startOffsets,
        deque<int64_t> &endOffsets)
{
  this->outerInstance = outerInstance;
  this->context = context;
  this->in_ = in_;
  this->fileNames = fileNames;
  this->startOffsets = startOffsets;
  this->endOffsets = endOffsets;
}

int SimpleTextCompoundFormat::DirectoryAnonymousInnerClass::getIndex(
    const wstring &name) 
{
  int index = Arrays::binarySearch(fileNames, name);
  if (index < 0) {
    // C++ TODO: The following line could not be converted:
    throw java.io.FileNotFoundException(
        L"No sub-file found (fileName=" + name + L" files: " +
        java.util.Arrays.toString(fileNames) + L")");
  }
  return index;
}

std::deque<wstring>
SimpleTextCompoundFormat::DirectoryAnonymousInnerClass::listAll() throw(
    IOException)
{
  ensureOpen();
  return fileNames.clone();
}

int64_t SimpleTextCompoundFormat::DirectoryAnonymousInnerClass::fileLength(
    const wstring &name) 
{
  ensureOpen();
  int index = getIndex(name);
  return endOffsets[index] - startOffsets[index];
}

shared_ptr<IndexInput>
SimpleTextCompoundFormat::DirectoryAnonymousInnerClass::openInput(
    const wstring &name, shared_ptr<IOContext> context) 
{
  ensureOpen();
  int index = getIndex(name);
  return in_->slice(name, startOffsets[index],
                    endOffsets[index] - startOffsets[index]);
}

SimpleTextCompoundFormat::DirectoryAnonymousInnerClass::
    ~DirectoryAnonymousInnerClass()
{
  delete in_;
}

shared_ptr<IndexOutput>
SimpleTextCompoundFormat::DirectoryAnonymousInnerClass::createOutput(
    const wstring &name, shared_ptr<IOContext> context)
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<IndexOutput>
SimpleTextCompoundFormat::DirectoryAnonymousInnerClass::createTempOutput(
    const wstring &prefix, const wstring &suffix, shared_ptr<IOContext> context)
{
  throw make_shared<UnsupportedOperationException>();
}

void SimpleTextCompoundFormat::DirectoryAnonymousInnerClass::sync(
    shared_ptr<deque<wstring>> names)
{
  throw make_shared<UnsupportedOperationException>();
}

void SimpleTextCompoundFormat::DirectoryAnonymousInnerClass::deleteFile(
    const wstring &name)
{
  throw make_shared<UnsupportedOperationException>();
}

void SimpleTextCompoundFormat::DirectoryAnonymousInnerClass::rename(
    const wstring &source, const wstring &dest)
{
  throw make_shared<UnsupportedOperationException>();
}

void SimpleTextCompoundFormat::DirectoryAnonymousInnerClass::syncMetaData()
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<Lock>
SimpleTextCompoundFormat::DirectoryAnonymousInnerClass::obtainLock(
    const wstring &name)
{
  throw make_shared<UnsupportedOperationException>();
}

void SimpleTextCompoundFormat::write(
    shared_ptr<Directory> dir, shared_ptr<SegmentInfo> si,
    shared_ptr<IOContext> context) 
{
  wstring dataFile =
      IndexFileNames::segmentFileName(si->name, L"", DATA_EXTENSION);

  int numFiles = si->files()->size();
  std::deque<wstring> names =
      si->files()->toArray(std::deque<wstring>(numFiles));
  Arrays::sort(names);
  std::deque<int64_t> startOffsets(numFiles);
  std::deque<int64_t> endOffsets(numFiles);

  shared_ptr<BytesRefBuilder> scratch = make_shared<BytesRefBuilder>();

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput out =
  // dir.createOutput(dataFile, context))
  {
    org::apache::lucene::store::IndexOutput out =
        dir->createOutput(dataFile, context);
    for (int i = 0; i < names.size(); i++) {
      // write header for file
      SimpleTextUtil::write(out, HEADER);
      SimpleTextUtil::write(out, names[i], scratch);
      SimpleTextUtil::writeNewline(out);

      // write bytes for file
      startOffsets[i] = out->getFilePointer();
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexInput
      // in = dir.openInput(names[i],
      // org.apache.lucene.store.IOContext.READONCE))
      {
        org::apache::lucene::store::IndexInput in_ = dir->openInput(
            names[i], org::apache::lucene::store::IOContext::READONCE);
        out->copyBytes(in_, in_->length());
      }
      endOffsets[i] = out->getFilePointer();
    }

    int64_t tocPos = out->getFilePointer();

    // write CFS table
    SimpleTextUtil::write(out, TABLE);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    SimpleTextUtil::write(out, Integer::toString(numFiles), scratch);
    SimpleTextUtil::writeNewline(out);

    for (int i = 0; i < names.size(); i++) {
      SimpleTextUtil::write(out, TABLENAME);
      SimpleTextUtil::write(out, names[i], scratch);
      SimpleTextUtil::writeNewline(out);

      SimpleTextUtil::write(out, TABLESTART);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      SimpleTextUtil::write(out, Long::toString(startOffsets[i]), scratch);
      SimpleTextUtil::writeNewline(out);

      SimpleTextUtil::write(out, TABLEEND);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      SimpleTextUtil::write(out, Long::toString(endOffsets[i]), scratch);
      SimpleTextUtil::writeNewline(out);
    }

    shared_ptr<DecimalFormat> df = make_shared<DecimalFormat>(
        OFFSETPATTERN, DecimalFormatSymbols::getInstance(Locale::ROOT));
    SimpleTextUtil::write(out, TABLEPOS);
    SimpleTextUtil::write(out, df->format(tocPos), scratch);
    SimpleTextUtil::writeNewline(out);
  }
}

wstring
SimpleTextCompoundFormat::stripPrefix(shared_ptr<BytesRefBuilder> scratch,
                                      shared_ptr<BytesRef> prefix)
{
  return wstring(scratch->bytes(), prefix->length,
                 scratch->length() - prefix->length, StandardCharsets::UTF_8);
}

const wstring SimpleTextCompoundFormat::DATA_EXTENSION = L"scf";
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextCompoundFormat::HEADER =
        make_shared<org::apache::lucene::util::BytesRef>(L"cfs entry for: ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextCompoundFormat::TABLE =
        make_shared<org::apache::lucene::util::BytesRef>(
            L"table of contents, size: ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextCompoundFormat::TABLENAME =
        make_shared<org::apache::lucene::util::BytesRef>(L"  filename: ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextCompoundFormat::TABLESTART =
        make_shared<org::apache::lucene::util::BytesRef>(L"    start: ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextCompoundFormat::TABLEEND =
        make_shared<org::apache::lucene::util::BytesRef>(L"    end: ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextCompoundFormat::TABLEPOS =
        make_shared<org::apache::lucene::util::BytesRef>(
            L"table of contents begins at offset: ");
const wstring SimpleTextCompoundFormat::OFFSETPATTERN;

SimpleTextCompoundFormat::StaticConstructor::StaticConstructor()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  int numDigits = Long::toString(numeric_limits<int64_t>::max())->length();
  std::deque<wchar_t> pattern(numDigits);
  Arrays::fill(pattern, L'0');
  OFFSETPATTERN = wstring(pattern);
}

SimpleTextCompoundFormat::StaticConstructor
    SimpleTextCompoundFormat::staticConstructor;
} // namespace org::apache::lucene::codecs::simpletext