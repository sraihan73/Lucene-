using namespace std;

#include "TermsHash.h"

namespace org::apache::lucene::index
{
using ByteBlockPool = org::apache::lucene::util::ByteBlockPool;
using Counter = org::apache::lucene::util::Counter;
using IntBlockPool = org::apache::lucene::util::IntBlockPool;

TermsHash::TermsHash(shared_ptr<DocumentsWriterPerThread> docWriter,
                     bool trackAllocations, shared_ptr<TermsHash> nextTermsHash)
    : nextTermsHash(nextTermsHash),
      intPool(make_shared<IntBlockPool>(docWriter->intBlockAllocator)),
      bytePool(make_shared<ByteBlockPool>(docWriter->byteBlockAllocator)),
      bytesUsed(trackAllocations ? docWriter->bytesUsed_
                                 : Counter::newCounter()),
      docState(docWriter->docState), trackAllocations(trackAllocations)
{

  if (nextTermsHash != nullptr) {
    // We are primary
    termBytePool = bytePool;
    nextTermsHash->termBytePool = bytePool;
  }
}

void TermsHash::abort()
{
  try {
    reset();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (nextTermsHash != nullptr) {
      nextTermsHash->abort();
    }
  }
}

void TermsHash::reset()
{
  // we don't reuse so we drop everything and don't fill with 0
  intPool->reset(false, false);
  bytePool->reset(false, false);
}

void TermsHash::flush(
    unordered_map<wstring, std::shared_ptr<TermsHashPerField>> &fieldsToFlush,
    shared_ptr<SegmentWriteState> state,
    shared_ptr<Sorter::DocMap> sortMap) 
{
  if (nextTermsHash != nullptr) {
    unordered_map<wstring, std::shared_ptr<TermsHashPerField>> nextChildFields =
        unordered_map<wstring, std::shared_ptr<TermsHashPerField>>();
    for (auto entry : fieldsToFlush) {
      nextChildFields.emplace(entry.first, entry.second::nextPerField);
    }
    nextTermsHash->flush(nextChildFields, state, sortMap);
  }
}

void TermsHash::finishDocument() 
{
  if (nextTermsHash != nullptr) {
    nextTermsHash->finishDocument();
  }
}

void TermsHash::startDocument() 
{
  if (nextTermsHash != nullptr) {
    nextTermsHash->startDocument();
  }
}
} // namespace org::apache::lucene::index