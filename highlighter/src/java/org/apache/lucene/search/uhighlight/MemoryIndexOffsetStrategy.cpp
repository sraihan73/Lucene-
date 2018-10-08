using namespace std;

#include "MemoryIndexOffsetStrategy.h"

namespace org::apache::lucene::search::uhighlight
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using FilteringTokenFilter =
    org::apache::lucene::analysis::FilteringTokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using MemoryIndex = org::apache::lucene::index::memory::MemoryIndex;
using Query = org::apache::lucene::search::Query;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using BytesRef = org::apache::lucene::util::BytesRef;
using Automata = org::apache::lucene::util::automaton::Automata;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;

MemoryIndexOffsetStrategy::MemoryIndexOffsetStrategy(
    const wstring &field, function<bool(const wstring &)> &fieldMatcher,
    std::deque<std::shared_ptr<BytesRef>> &extractedTerms,
    shared_ptr<PhraseHelper> phraseHelper,
    std::deque<std::shared_ptr<CharacterRunAutomaton>> &automata,
    shared_ptr<Analyzer> analyzer,
    function<deque<std::shared_ptr<Query>>> &multiTermQueryRewrite)
    : AnalysisOffsetStrategy(field, extractedTerms, phraseHelper, automata,
                             analyzer),
      memoryIndex(make_shared<MemoryIndex>(true, storePayloads)) / *true ==
          store offsets * /
      , leafReader(std::static_pointer_cast<LeafReader>(
            memoryIndex->createSearcher()->getIndexReader())) /
                *appears to be re
            - usable * /
      , preMemIndexFilterAutomaton(
            buildCombinedAutomaton(fieldMatcher, terms, this->automata,
                                   phraseHelper, multiTermQueryRewrite))
{
  bool storePayloads =
      phraseHelper->hasPositionSensitivity(); // might be needed
  // preFilter for MemoryIndex
}

shared_ptr<CharacterRunAutomaton>
MemoryIndexOffsetStrategy::buildCombinedAutomaton(
    function<bool(const wstring &)> &fieldMatcher,
    std::deque<std::shared_ptr<BytesRef>> &terms,
    std::deque<std::shared_ptr<CharacterRunAutomaton>> &automata,
    shared_ptr<PhraseHelper> strictPhrases,
    function<deque<std::shared_ptr<Query>>> &multiTermQueryRewrite)
{
  deque<std::shared_ptr<CharacterRunAutomaton>> allAutomata =
      deque<std::shared_ptr<CharacterRunAutomaton>>();
  if (terms.size() > 0) {
    allAutomata.push_back(make_shared<CharacterRunAutomaton>(
        Automata::makeStringUnion(Arrays::asList(terms))));
  }
  Collections::addAll(allAutomata, automata);
  for (auto spanQuery : strictPhrases->getSpanQueries()) {
    Collections::addAll(allAutomata,
                        MultiTermHighlighting::extractAutomata(
                            spanQuery, fieldMatcher, true,
                            multiTermQueryRewrite)); // true==lookInSpan
  }

  if (allAutomata.size() == 1) {
    return allAutomata[0];
  }
  // TODO it'd be nice if we could get at the underlying Automaton in
  // CharacterRunAutomaton so that we
  //  could union them all. But it's not exposed, and sometimes the automaton is
  //  byte (not char) oriented

  // Return an aggregate CharacterRunAutomaton of others
  return make_shared<CharacterRunAutomatonAnonymousInnerClass>(
      Automata::makeEmpty(), allAutomata);
}

MemoryIndexOffsetStrategy::CharacterRunAutomatonAnonymousInnerClass::
    CharacterRunAutomatonAnonymousInnerClass(
        shared_ptr<org::apache::lucene::util::automaton::Automaton> makeEmpty,
        deque<std::shared_ptr<CharacterRunAutomaton>> &allAutomata)
    : org::apache::lucene::util::automaton::CharacterRunAutomaton(makeEmpty)
{
  this->allAutomata = allAutomata;
}

bool MemoryIndexOffsetStrategy::CharacterRunAutomatonAnonymousInnerClass::run(
    std::deque<wchar_t> &chars, int offset, int length)
{
  for (int i = 0; i < allAutomata.size();
       i++) { // don't use foreach to avoid Iterator allocation
    if (allAutomata[i]->run(chars, offset, length)) {
      return true;
    }
  }
  return false;
}

shared_ptr<OffsetsEnum> MemoryIndexOffsetStrategy::getOffsetsEnum(
    shared_ptr<IndexReader> reader, int docId,
    const wstring &content) 
{
  // note: don't need LimitTokenOffsetFilter since content is already truncated
  // to maxLength
  shared_ptr<TokenStream> tokenStream = this->tokenStream(content);

  // Filter the tokenStream to applicable terms
  tokenStream = newKeepWordFilter(tokenStream, preMemIndexFilterAutomaton);
  memoryIndex->reset();
  memoryIndex->addField(
      field, tokenStream); // note: calls tokenStream.reset() & close()
  docId = 0;

  return createOffsetsEnumFromReader(leafReader, docId);
}

shared_ptr<FilteringTokenFilter> MemoryIndexOffsetStrategy::newKeepWordFilter(
    shared_ptr<TokenStream> tokenStream,
    shared_ptr<CharacterRunAutomaton> charRunAutomaton)
{
  // it'd be nice to use KeepWordFilter but it demands a CharArraySet. TODO File
  // JIRA? Need a new interface?
  return make_shared<FilteringTokenFilterAnonymousInnerClass>(tokenStream,
                                                              charRunAutomaton);
}

MemoryIndexOffsetStrategy::FilteringTokenFilterAnonymousInnerClass::
    FilteringTokenFilterAnonymousInnerClass(
        shared_ptr<TokenStream> tokenStream,
        shared_ptr<CharacterRunAutomaton> charRunAutomaton)
    : org::apache::lucene::analysis::FilteringTokenFilter(tokenStream)
{
  this->charRunAutomaton = charRunAutomaton;
}

bool MemoryIndexOffsetStrategy::FilteringTokenFilterAnonymousInnerClass::
    accept() 
{
  return charRunAutomaton->run(charAtt::buffer(), 0, charAtt->length());
}
} // namespace org::apache::lucene::search::uhighlight