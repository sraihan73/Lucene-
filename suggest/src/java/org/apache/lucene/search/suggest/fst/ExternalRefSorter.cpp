using namespace std;

#include "ExternalRefSorter.h"

namespace org::apache::lucene::search::suggest::fst
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;
using IOUtils = org::apache::lucene::util::IOUtils;
using OfflineSorter = org::apache::lucene::util::OfflineSorter;

ExternalRefSorter::ExternalRefSorter(shared_ptr<OfflineSorter> sorter) throw(
    IOException)
    : sorter(sorter)
{
  this->input = sorter->getDirectory()->createTempOutput(
      sorter->getTempFileNamePrefix(), L"RefSorterRaw", IOContext::DEFAULT);
  this->writer = make_shared<OfflineSorter::ByteSequencesWriter>(this->input);
}

void ExternalRefSorter::add(shared_ptr<BytesRef> utf8) 
{
  if (writer == nullptr) {
    throw make_shared<IllegalStateException>();
  }
  writer->write(utf8);
}

shared_ptr<BytesRefIterator> ExternalRefSorter::iterator() 
{
  if (sortedFileName == L"") {
    closeWriter();

    bool success = false;
    try {
      sortedFileName = sorter->sort(input->getName());
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (success) {
        sorter->getDirectory()->deleteFile(input->getName());
      } else {
        IOUtils::deleteFilesIgnoringExceptions(sorter->getDirectory(),
                                               {input->getName()});
      }
    }

    input.reset();
  }

  return make_shared<ByteSequenceIterator>(
      make_shared<OfflineSorter::ByteSequencesReader>(
          sorter->getDirectory()->openChecksumInput(sortedFileName,
                                                    IOContext::READONCE),
          sortedFileName));
}

void ExternalRefSorter::closeWriter() 
{
  if (writer != nullptr) {
    CodecUtil::writeFooter(input);
    delete writer;
    writer.reset();
  }
}

ExternalRefSorter::~ExternalRefSorter()
{
  try {
    closeWriter();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::deleteFilesIgnoringExceptions(
        sorter->getDirectory(),
        {input == nullptr ? L"" : input->getName(), sortedFileName});
  }
}

ExternalRefSorter::ByteSequenceIterator::ByteSequenceIterator(
    shared_ptr<OfflineSorter::ByteSequencesReader> reader)
    : reader(reader)
{
}

shared_ptr<BytesRef>
ExternalRefSorter::ByteSequenceIterator::next() 
{
  bool success = false;
  try {
    scratch = reader->next();
    if (scratch == nullptr) {
      delete reader;
    }
    success = true;
    return scratch;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException({reader});
    }
  }
}

shared_ptr<Comparator<std::shared_ptr<BytesRef>>>
ExternalRefSorter::getComparator()
{
  return sorter->getComparator();
}
} // namespace org::apache::lucene::search::suggest::fst