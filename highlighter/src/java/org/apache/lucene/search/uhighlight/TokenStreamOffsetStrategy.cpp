using namespace std;

#include "TokenStreamOffsetStrategy.h"

namespace org::apache::lucene::search::uhighlight
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using IndexReader = org::apache::lucene::index::IndexReader;
using BytesRef = org::apache::lucene::util::BytesRef;
using Automata = org::apache::lucene::util::automaton::Automata;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
std::deque<std::shared_ptr<org::apache::lucene::util::BytesRef>> const
    TokenStreamOffsetStrategy::ZERO_LEN_BYTES_REF_ARRAY =
        std::deque<std::shared_ptr<org::apache::lucene::util::BytesRef>>(0);

TokenStreamOffsetStrategy::TokenStreamOffsetStrategy(
    const wstring &field, std::deque<std::shared_ptr<BytesRef>> &terms,
    shared_ptr<PhraseHelper> phraseHelper,
    std::deque<std::shared_ptr<CharacterRunAutomaton>> &automata,
    shared_ptr<Analyzer> indexAnalyzer)
    : AnalysisOffsetStrategy(field, ZERO_LEN_BYTES_REF_ARRAY, phraseHelper,
                             convertTermsToAutomata(terms, automata),
                             indexAnalyzer)
{
  assert(phraseHelper->hasPositionSensitivity() == false);
}

std::deque<std::shared_ptr<CharacterRunAutomaton>>
TokenStreamOffsetStrategy::convertTermsToAutomata(
    std::deque<std::shared_ptr<BytesRef>> &terms,
    std::deque<std::shared_ptr<CharacterRunAutomaton>> &automata)
{
  std::deque<std::shared_ptr<CharacterRunAutomaton>> newAutomata(
      terms.size() + automata.size());
  for (int i = 0; i < terms.size(); i++) {
    wstring termString = terms[i]->utf8ToString();
    newAutomata[i] = make_shared<CharacterRunAutomatonAnonymousInnerClass>(
        Automata::makeString(termString), termString);
  }
  // Append existing automata (that which is used for MTQs)
  System::arraycopy(automata, 0, newAutomata, terms.size(), automata.size());
  return newAutomata;
}

TokenStreamOffsetStrategy::CharacterRunAutomatonAnonymousInnerClass::
    CharacterRunAutomatonAnonymousInnerClass(
        shared_ptr<org::apache::lucene::util::automaton::Automaton> makeString,
        const wstring &termString)
    : org::apache::lucene::util::automaton::CharacterRunAutomaton(makeString)
{
  this->termString = termString;
}

wstring
TokenStreamOffsetStrategy::CharacterRunAutomatonAnonymousInnerClass::toString()
{
  return termString;
}

shared_ptr<OffsetsEnum> TokenStreamOffsetStrategy::getOffsetsEnum(
    shared_ptr<IndexReader> reader, int docId,
    const wstring &content) 
{
  return make_shared<TokenStreamOffsetsEnum>(tokenStream(content), automata);
}

TokenStreamOffsetStrategy::TokenStreamOffsetsEnum::TokenStreamOffsetsEnum(
    shared_ptr<TokenStream> ts,
    std::deque<std::shared_ptr<CharacterRunAutomaton>>
        &matchers) 
    : matchers(matchers),
      charTermAtt(ts->addAttribute(CharTermAttribute::typeid)),
      offsetAtt(ts->addAttribute(OffsetAttribute::typeid)),
      matchDescriptions(std::deque<std::shared_ptr<BytesRef>>(matchers.size()))
{
  this->stream = ts;
  ts->reset();
}

bool TokenStreamOffsetStrategy::TokenStreamOffsetsEnum::nextPosition() throw(
    IOException)
{
  if (stream != nullptr) {
    while (stream->incrementToken()) {
      for (int i = 0; i < matchers.size(); i++) {
        if (matchers[i]->run(charTermAtt->buffer(), 0, charTermAtt->length())) {
          currentMatch = i;
          return true;
        }
      }
    }
    stream->end();
    close();
  }
  // exhausted
  return false;
}

int TokenStreamOffsetStrategy::TokenStreamOffsetsEnum::freq() 
{
  return numeric_limits<int>::max(); // lie
}

int TokenStreamOffsetStrategy::TokenStreamOffsetsEnum::startOffset() throw(
    IOException)
{
  return offsetAtt->startOffset();
}

int TokenStreamOffsetStrategy::TokenStreamOffsetsEnum::endOffset() throw(
    IOException)
{
  return offsetAtt->endOffset();
}

shared_ptr<BytesRef>
TokenStreamOffsetStrategy::TokenStreamOffsetsEnum::getTerm() 
{
  if (matchDescriptions[currentMatch] == nullptr) {
    // these CharRunAutomata are subclassed so that toString() returns the query
    // C++ TODO: There is no native C++ equivalent to 'toString':
    matchDescriptions[currentMatch] =
        make_shared<BytesRef>(matchers[currentMatch]->toString());
  }
  return matchDescriptions[currentMatch];
}

TokenStreamOffsetStrategy::TokenStreamOffsetsEnum::~TokenStreamOffsetsEnum()
{
  if (stream != nullptr) {
    delete stream;
    stream.reset();
  }
}
} // namespace org::apache::lucene::search::uhighlight