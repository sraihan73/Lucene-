using namespace std;

#include "PostingsOffsetStrategy.h"

namespace org::apache::lucene::search::uhighlight
{
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;

PostingsOffsetStrategy::PostingsOffsetStrategy(
    const wstring &field, std::deque<std::shared_ptr<BytesRef>> &queryTerms,
    shared_ptr<PhraseHelper> phraseHelper,
    std::deque<std::shared_ptr<CharacterRunAutomaton>> &automata)
    : FieldOffsetStrategy(field, queryTerms, phraseHelper, automata)
{
}

shared_ptr<OffsetsEnum> PostingsOffsetStrategy::getOffsetsEnum(
    shared_ptr<IndexReader> reader, int docId,
    const wstring &content) 
{
  shared_ptr<LeafReader> *const leafReader;
  if (std::dynamic_pointer_cast<LeafReader>(reader) != nullptr) {
    leafReader = std::static_pointer_cast<LeafReader>(reader);
  } else {
    deque<std::shared_ptr<LeafReaderContext>> leaves = reader->leaves();
    shared_ptr<LeafReaderContext> leafReaderContext =
        leaves[ReaderUtil::subIndex(docId, leaves)];
    leafReader = leafReaderContext->reader();
    docId -= leafReaderContext
                 ->docBase; // adjust 'doc' to be within this leaf reader
  }

  return createOffsetsEnumFromReader(leafReader, docId);
}

UnifiedHighlighter::OffsetSource PostingsOffsetStrategy::getOffsetSource()
{
  return UnifiedHighlighter::OffsetSource::POSTINGS;
}
} // namespace org::apache::lucene::search::uhighlight