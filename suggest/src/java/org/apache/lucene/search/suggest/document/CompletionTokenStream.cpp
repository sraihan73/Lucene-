using namespace std;

#include "CompletionTokenStream.h"

namespace org::apache::lucene::search::suggest::document
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ConcatenateGraphFilter =
    org::apache::lucene::analysis::miscellaneous::ConcatenateGraphFilter;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;
using Automaton = org::apache::lucene::util::automaton::Automaton;

CompletionTokenStream::CompletionTokenStream(
    shared_ptr<TokenStream> inputTokenStream)
    : CompletionTokenStream(
          inputTokenStream, ConcatenateGraphFilter::DEFAULT_PRESERVE_SEP,
          ConcatenateGraphFilter::DEFAULT_PRESERVE_POSITION_INCREMENTS,
          ConcatenateGraphFilter::DEFAULT_MAX_GRAPH_EXPANSIONS)
{
}

CompletionTokenStream::CompletionTokenStream(
    shared_ptr<TokenStream> inputTokenStream, bool preserveSep,
    bool preservePositionIncrements, int maxGraphExpansions)
    : org::apache::lucene::analysis::TokenFilter(new ConcatenateGraphFilter(
          inputTokenStream, preserveSep, preservePositionIncrements,
          maxGraphExpansions)),
      inputTokenStream(inputTokenStream), preserveSep(preserveSep),
      preservePositionIncrements(preservePositionIncrements),
      maxGraphExpansions(maxGraphExpansions)
{
}

void CompletionTokenStream::setPayload(shared_ptr<BytesRef> payload)
{
  this->payload = payload;
}

bool CompletionTokenStream::incrementToken() 
{
  if (input->incrementToken()) {
    payloadAttr->setPayload(payload);
    return true;
  } else {
    return false;
  }
}

shared_ptr<Automaton> CompletionTokenStream::toAutomaton() 
{
  return (std::static_pointer_cast<ConcatenateGraphFilter>(input))
      ->toAutomaton();
}

shared_ptr<Automaton>
CompletionTokenStream::toAutomaton(bool unicodeAware) 
{
  return (std::static_pointer_cast<ConcatenateGraphFilter>(input))
      ->toAutomaton(unicodeAware);
}
} // namespace org::apache::lucene::search::suggest::document