using namespace std;

#include "PostingsWithTermVectorsOffsetStrategy.h"

namespace org::apache::lucene::search::uhighlight
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using Terms = org::apache::lucene::index::Terms;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;

PostingsWithTermVectorsOffsetStrategy::PostingsWithTermVectorsOffsetStrategy(
    const wstring &field, std::deque<std::shared_ptr<BytesRef>> &queryTerms,
    shared_ptr<PhraseHelper> phraseHelper,
    std::deque<std::shared_ptr<CharacterRunAutomaton>> &automata)
    : FieldOffsetStrategy(field, queryTerms, phraseHelper, automata)
{
}

shared_ptr<OffsetsEnum> PostingsWithTermVectorsOffsetStrategy::getOffsetsEnum(
    shared_ptr<IndexReader> reader, int docId,
    const wstring &content) 
{
  shared_ptr<LeafReader> leafReader;
  if (std::dynamic_pointer_cast<LeafReader>(reader) != nullptr) {
    leafReader = std::static_pointer_cast<LeafReader>(reader);
  } else {
    deque<std::shared_ptr<LeafReaderContext>> leaves = reader->leaves();
    LeafReaderContext LeafReaderContext =
        leaves[ReaderUtil::subIndex(docId, leaves)];
    leafReader = LeafReaderContext::reader();
    docId -= LeafReaderContext::docBase; // adjust 'doc' to be within this
                                         // atomic reader
  }

  shared_ptr<Terms> docTerms = leafReader->getTermVector(docId, field);
  if (docTerms == nullptr) {
    return OffsetsEnum::EMPTY;
  }
  leafReader = make_shared<TermVectorFilteredLeafReader>(leafReader, docTerms);

  return createOffsetsEnumFromReader(leafReader, docId);
}

UnifiedHighlighter::OffsetSource
PostingsWithTermVectorsOffsetStrategy::getOffsetSource()
{
  return UnifiedHighlighter::OffsetSource::POSTINGS_WITH_TERM_VECTORS;
}
} // namespace org::apache::lucene::search::uhighlight