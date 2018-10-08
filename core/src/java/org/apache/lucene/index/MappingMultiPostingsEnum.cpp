using namespace std;

#include "MappingMultiPostingsEnum.h"

namespace org::apache::lucene::index
{
using BytesRef = org::apache::lucene::util::BytesRef;

MappingMultiPostingsEnum::MappingPostingsSub::MappingPostingsSub(
    shared_ptr<MergeState::DocMap> docMap)
    : DocIDMerger::Sub(docMap)
{
}

int MappingMultiPostingsEnum::MappingPostingsSub::nextDoc()
{
  try {
    return postings->nextDoc();
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }
}

MappingMultiPostingsEnum::MappingMultiPostingsEnum(
    const wstring &field, shared_ptr<MergeState> mergeState) 
    : field(field), docIDMerger(DocIDMerger::of(subs, allSubs.size(),
                                                mergeState->needsIndexSort)),
      allSubs(std::deque<std::shared_ptr<MappingPostingsSub>>(
          mergeState->fieldsProducers.size()))
{
  for (int i = 0; i < allSubs.size(); i++) {
    allSubs[i] = make_shared<MappingPostingsSub>(mergeState->docMaps[i]);
  }
}

shared_ptr<MappingMultiPostingsEnum> MappingMultiPostingsEnum::reset(
    shared_ptr<MultiPostingsEnum> postingsEnum) 
{
  this->multiDocsAndPositionsEnum = postingsEnum;
  std::deque<std::shared_ptr<MultiPostingsEnum::EnumWithSlice>> subsArray =
      postingsEnum->getSubs();
  int count = postingsEnum->getNumSubs();
  subs.clear();
  for (int i = 0; i < count; i++) {
    shared_ptr<MappingPostingsSub> sub =
        allSubs[subsArray[i]->slice.readerIndex];
    sub->postings = subsArray[i]->postingsEnum;
    subs.push_back(sub);
  }
  docIDMerger->reset();
  return shared_from_this();
}

int MappingMultiPostingsEnum::freq() 
{
  return current->postings->freq();
}

int MappingMultiPostingsEnum::docID()
{
  if (current == nullptr) {
    return -1;
  } else {
    return current->mappedDocID;
  }
}

int MappingMultiPostingsEnum::advance(int target)
{
  throw make_shared<UnsupportedOperationException>();
}

int MappingMultiPostingsEnum::nextDoc() 
{
  current = docIDMerger->next();
  if (current == nullptr) {
    return NO_MORE_DOCS;
  } else {
    return current->mappedDocID;
  }
}

int MappingMultiPostingsEnum::nextPosition() 
{
  int pos = current->postings->nextPosition();
  if (pos < 0) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    throw make_shared<CorruptIndexException>(
        L"position=" + to_wstring(pos) + L" is negative, field=\"" + field +
            L" doc=" + to_wstring(current->mappedDocID),
        current->postings->toString());
  } else if (pos > IndexWriter::MAX_POSITION) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    throw make_shared<CorruptIndexException>(
        L"position=" + to_wstring(pos) +
            L" is too large (> IndexWriter.MAX_POSITION=" +
            to_wstring(IndexWriter::MAX_POSITION) + L"), field=\"" + field +
            L"\" doc=" + to_wstring(current->mappedDocID),
        current->postings->toString());
  }
  return pos;
}

int MappingMultiPostingsEnum::startOffset() 
{
  return current->postings->startOffset();
}

int MappingMultiPostingsEnum::endOffset() 
{
  return current->postings->endOffset();
}

shared_ptr<BytesRef> MappingMultiPostingsEnum::getPayload() 
{
  return current->postings->getPayload();
}

int64_t MappingMultiPostingsEnum::cost()
{
  int64_t cost = 0;
  for (auto sub : subs) {
    cost += sub->postings->cost();
  }
  return cost;
}
} // namespace org::apache::lucene::index