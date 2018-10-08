using namespace std;

#include "TermVectorOffsetStrategy.h"

namespace org::apache::lucene::search::uhighlight
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using Terms = org::apache::lucene::index::Terms;
using TermVectorLeafReader =
    org::apache::lucene::search::highlight::TermVectorLeafReader;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;

TermVectorOffsetStrategy::TermVectorOffsetStrategy(
    const wstring &field, std::deque<std::shared_ptr<BytesRef>> &queryTerms,
    shared_ptr<PhraseHelper> phraseHelper,
    std::deque<std::shared_ptr<CharacterRunAutomaton>> &automata)
    : FieldOffsetStrategy(field, queryTerms, phraseHelper, automata)
{
}

UnifiedHighlighter::OffsetSource TermVectorOffsetStrategy::getOffsetSource()
{
  return UnifiedHighlighter::OffsetSource::TERM_VECTORS;
}

shared_ptr<OffsetsEnum> TermVectorOffsetStrategy::getOffsetsEnum(
    shared_ptr<IndexReader> reader, int docId,
    const wstring &content) 
{
  shared_ptr<Terms> tvTerms = reader->getTermVector(docId, field);
  if (tvTerms == nullptr) {
    return OffsetsEnum::EMPTY;
  }

  shared_ptr<LeafReader> leafReader =
      make_shared<TermVectorLeafReader>(field, tvTerms);
  docId = 0;

  return createOffsetsEnumFromReader(leafReader, docId);
}
} // namespace org::apache::lucene::search::uhighlight