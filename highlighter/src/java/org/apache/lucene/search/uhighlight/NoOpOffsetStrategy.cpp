using namespace std;

#include "NoOpOffsetStrategy.h"

namespace org::apache::lucene::search::uhighlight
{
using IndexReader = org::apache::lucene::index::IndexReader;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
const shared_ptr<NoOpOffsetStrategy> NoOpOffsetStrategy::INSTANCE =
    make_shared<NoOpOffsetStrategy>();

NoOpOffsetStrategy::NoOpOffsetStrategy()
    : FieldOffsetStrategy(L"_ignored_", new BytesRef[0], PhraseHelper::NONE,
                          new CharacterRunAutomaton[0])
{
}

UnifiedHighlighter::OffsetSource NoOpOffsetStrategy::getOffsetSource()
{
  return UnifiedHighlighter::OffsetSource::NONE_NEEDED;
}

shared_ptr<OffsetsEnum>
NoOpOffsetStrategy::getOffsetsEnum(shared_ptr<IndexReader> reader, int docId,
                                   const wstring &content) 
{
  return OffsetsEnum::EMPTY;
}
} // namespace org::apache::lucene::search::uhighlight