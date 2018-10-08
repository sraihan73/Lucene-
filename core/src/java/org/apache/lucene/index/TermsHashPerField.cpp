using namespace std;

#include "TermsHashPerField.h"

namespace org::apache::lucene::index
{
using TermFrequencyAttribute =
    org::apache::lucene::analysis::tokenattributes::TermFrequencyAttribute;
using TermToBytesRefAttribute =
    org::apache::lucene::analysis::tokenattributes::TermToBytesRefAttribute;
using ByteBlockPool = org::apache::lucene::util::ByteBlockPool;
using BytesStartArray =
    org::apache::lucene::util::BytesRefHash::BytesStartArray;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;
using Counter = org::apache::lucene::util::Counter;
using IntBlockPool = org::apache::lucene::util::IntBlockPool;

TermsHashPerField::TermsHashPerField(int streamCount,
                                     shared_ptr<FieldInvertState> fieldState,
                                     shared_ptr<TermsHash> termsHash,
                                     shared_ptr<TermsHashPerField> nextPerField,
                                     shared_ptr<FieldInfo> fieldInfo)
    : termsHash(termsHash), nextPerField(nextPerField),
      docState(termsHash->docState), fieldState(fieldState),
      intPool(termsHash->intPool), bytePool(termsHash->bytePool),
      termBytePool(termsHash->termBytePool), streamCount(streamCount),
      numPostingInt(2 * streamCount), fieldInfo(fieldInfo),
      bytesHash(
          make_shared<BytesRefHash>(termBytePool, HASH_INIT_SIZE, byteStarts)),
      bytesUsed(termsHash->bytesUsed)
{
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  shared_ptr<PostingsBytesStartArray> byteStarts =
      make_shared<PostingsBytesStartArray>(shared_from_this(), bytesUsed);
}

void TermsHashPerField::reset()
{
  bytesHash->clear(false);
  if (nextPerField != nullptr) {
    nextPerField->reset();
  }
}

void TermsHashPerField::initReader(shared_ptr<ByteSliceReader> reader,
                                   int termID, int stream)
{
  assert(stream < streamCount);
  int intStart = postingsArray->intStarts[termID];
  const std::deque<int> ints =
      intPool->buffers[intStart >> IntBlockPool::INT_BLOCK_SHIFT];
  constexpr int upto = intStart & IntBlockPool::INT_BLOCK_MASK;
  reader->init(bytePool,
               postingsArray->byteStarts[termID] +
                   stream * ByteBlockPool::FIRST_LEVEL_SIZE,
               ints[upto + stream]);
}

std::deque<int> TermsHashPerField::sortPostings()
{
  sortedTermIDs = bytesHash->sort();
  return sortedTermIDs;
}

void TermsHashPerField::add(int textStart) 
{
  int termID = bytesHash->addByPoolOffset(textStart);
  if (termID >= 0) { // New posting
    // First time we are seeing this token since we last
    // flushed the hash.
    // Init stream slices
    if (numPostingInt + intPool->intUpto > IntBlockPool::INT_BLOCK_SIZE) {
      intPool->nextBuffer();
    }

    if (ByteBlockPool::BYTE_BLOCK_SIZE - bytePool->byteUpto <
        numPostingInt * ByteBlockPool::FIRST_LEVEL_SIZE) {
      bytePool->nextBuffer();
    }

    intUptos = intPool->buffer;
    intUptoStart = intPool->intUpto;
    intPool->intUpto += streamCount;

    postingsArray->intStarts[termID] = intUptoStart + intPool->intOffset;

    for (int i = 0; i < streamCount; i++) {
      constexpr int upto = bytePool->newSlice(ByteBlockPool::FIRST_LEVEL_SIZE);
      intUptos[intUptoStart + i] = upto + bytePool->byteOffset;
    }
    postingsArray->byteStarts[termID] = intUptos[intUptoStart];

    newTerm(termID);

  } else {
    termID = (-termID) - 1;
    int intStart = postingsArray->intStarts[termID];
    intUptos = intPool->buffers[intStart >> IntBlockPool::INT_BLOCK_SHIFT];
    intUptoStart = intStart & IntBlockPool::INT_BLOCK_MASK;
    addTerm(termID);
  }
}

void TermsHashPerField::add() 
{
  // We are first in the chain so we must "intern" the
  // term text into textStart address
  // Get the text & hash of this term.
  int termID = bytesHash->add(termAtt->getBytesRef());

  // System.out.println("add term=" + termBytesRef.utf8ToString() + " doc=" +
  // docState.docID + " termID=" + termID);

  if (termID >= 0) { // New posting
    bytesHash->byteStart(termID);
    // Init stream slices
    if (numPostingInt + intPool->intUpto > IntBlockPool::INT_BLOCK_SIZE) {
      intPool->nextBuffer();
    }

    if (ByteBlockPool::BYTE_BLOCK_SIZE - bytePool->byteUpto <
        numPostingInt * ByteBlockPool::FIRST_LEVEL_SIZE) {
      bytePool->nextBuffer();
    }

    intUptos = intPool->buffer;
    intUptoStart = intPool->intUpto;
    intPool->intUpto += streamCount;

    postingsArray->intStarts[termID] = intUptoStart + intPool->intOffset;

    for (int i = 0; i < streamCount; i++) {
      constexpr int upto = bytePool->newSlice(ByteBlockPool::FIRST_LEVEL_SIZE);
      intUptos[intUptoStart + i] = upto + bytePool->byteOffset;
    }
    postingsArray->byteStarts[termID] = intUptos[intUptoStart];

    newTerm(termID);

  } else {
    termID = (-termID) - 1;
    int intStart = postingsArray->intStarts[termID];
    intUptos = intPool->buffers[intStart >> IntBlockPool::INT_BLOCK_SHIFT];
    intUptoStart = intStart & IntBlockPool::INT_BLOCK_MASK;
    addTerm(termID);
  }

  if (doNextCall) {
    nextPerField->add(postingsArray->textStarts[termID]);
  }
}

void TermsHashPerField::writeByte(int stream, char b)
{
  int upto = intUptos[intUptoStart + stream];
  std::deque<char> bytes =
      bytePool->buffers[upto >> ByteBlockPool::BYTE_BLOCK_SHIFT];
  assert(bytes.size() > 0);
  int offset = upto & ByteBlockPool::BYTE_BLOCK_MASK;
  if (bytes[offset] != 0) {
    // End of slice; allocate a new one
    offset = bytePool->allocSlice(bytes, offset);
    bytes = bytePool->buffer;
    intUptos[intUptoStart + stream] = offset + bytePool->byteOffset;
  }
  bytes[offset] = b;
  (intUptos[intUptoStart + stream])++;
}

void TermsHashPerField::writeBytes(int stream, std::deque<char> &b, int offset,
                                   int len)
{
  // TODO: optimize
  constexpr int end = offset + len;
  for (int i = offset; i < end; i++) {
    writeByte(stream, b[i]);
  }
}

void TermsHashPerField::writeVInt(int stream, int i)
{
  assert(stream < streamCount);
  while ((i & ~0x7F) != 0) {
    writeByte(stream, static_cast<char>((i & 0x7f) | 0x80));
    i = static_cast<int>(static_cast<unsigned int>(i) >> 7);
  }
  writeByte(stream, static_cast<char>(i));
}

TermsHashPerField::PostingsBytesStartArray::PostingsBytesStartArray(
    shared_ptr<TermsHashPerField> perField, shared_ptr<Counter> bytesUsed)
    : perField(perField), bytesUsed(bytesUsed)
{
}

std::deque<int> TermsHashPerField::PostingsBytesStartArray::init()
{
  if (perField->postingsArray == nullptr) {
    perField->postingsArray = perField->createPostingsArray(2);
    perField->newPostingsArray();
    bytesUsed_->addAndGet(perField->postingsArray->size *
                          perField->postingsArray->bytesPerPosting());
  }
  return perField->postingsArray->textStarts;
}

std::deque<int> TermsHashPerField::PostingsBytesStartArray::grow()
{
  shared_ptr<ParallelPostingsArray> postingsArray = perField->postingsArray;
  constexpr int oldSize = perField->postingsArray->size;
  postingsArray = perField->postingsArray = postingsArray->grow();
  perField->newPostingsArray();
  bytesUsed_->addAndGet(
      (postingsArray->bytesPerPosting() * (postingsArray->size - oldSize)));
  return postingsArray->textStarts;
}

std::deque<int> TermsHashPerField::PostingsBytesStartArray::clear()
{
  if (perField->postingsArray != nullptr) {
    bytesUsed_->addAndGet(-(perField->postingsArray->size *
                            perField->postingsArray->bytesPerPosting()));
    perField->postingsArray.reset();
    perField->newPostingsArray();
  }
  return nullptr;
}

shared_ptr<Counter> TermsHashPerField::PostingsBytesStartArray::bytesUsed()
{
  return bytesUsed_;
}

int TermsHashPerField::compareTo(shared_ptr<TermsHashPerField> other)
{
  return fieldInfo->name.compare(other->fieldInfo->name);
}

void TermsHashPerField::finish() 
{
  if (nextPerField != nullptr) {
    nextPerField->finish();
  }
}

bool TermsHashPerField::start(shared_ptr<IndexableField> field, bool first)
{
  termAtt = fieldState->termAttribute;
  termFreqAtt = fieldState->termFreqAttribute;
  if (nextPerField != nullptr) {
    doNextCall = nextPerField->start(field, first);
  }

  return true;
}
} // namespace org::apache::lucene::index