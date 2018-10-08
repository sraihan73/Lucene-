using namespace std;

#include "ContextQuery.h"

namespace org::apache::lucene::search::suggest::document
{
using ConcatenateGraphFilter =
    org::apache::lucene::analysis::miscellaneous::ConcatenateGraphFilter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Weight = org::apache::lucene::search::Weight;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using Automata = org::apache::lucene::util::automaton::Automata;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using Operations = org::apache::lucene::util::automaton::Operations;
using Util = org::apache::lucene::util::fst::Util;

ContextQuery::ContextQuery(shared_ptr<CompletionQuery> query)
    : CompletionQuery(query->getTerm(), query->getFilter())
{
  if (std::dynamic_pointer_cast<ContextQuery>(query) != nullptr) {
    throw invalid_argument(L"'query' parameter must not be of type " +
                           this->getClass().getSimpleName());
  }
  this->innerQuery = query;
  contexts = unordered_map<>();
}

void ContextQuery::addContext(shared_ptr<std::wstring> context)
{
  addContext(context, 1.0f, true);
}

void ContextQuery::addContext(shared_ptr<std::wstring> context, float boost)
{
  addContext(context, boost, true);
}

void ContextQuery::addContext(shared_ptr<std::wstring> context, float boost,
                              bool exact)
{
  if (boost < 0.0f) {
    throw invalid_argument(L"'boost' must be >= 0");
  }
  for (int i = 0; i < context->length(); i++) {
    if (ContextSuggestField::CONTEXT_SEPARATOR == context->charAt(i)) {
      throw invalid_argument(
          L"Illegal value [" + context + L"] UTF-16 codepoint [0x" +
          Integer::toHexString(static_cast<int>(context->charAt(i))) +
          L"] at position " + to_wstring(i) + L" is a reserved character");
    }
  }
  contexts.emplace(IntsRef::deepCopyOf(Util::toIntsRef(
                       make_shared<BytesRef>(context), scratch)),
                   make_shared<ContextMetaData>(boost, exact));
}

void ContextQuery::addAllContexts() { matchAllContexts = true; }

wstring ContextQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  shared_ptr<BytesRefBuilder> scratch = make_shared<BytesRefBuilder>();
  for (auto context : contexts) {
    if (buffer->length() != 0) {
      buffer->append(L",");
    } else {
      buffer->append(L"contexts");
      buffer->append(L":[");
    }
    buffer->append(Util::toBytesRef(context->first, scratch)->utf8ToString());
    shared_ptr<ContextMetaData> metaData = contexts[context->first];
    if (metaData->exact == false) {
      buffer->append(L"*");
    }
    if (metaData->boost != 0) {
      buffer->append(L"^");
      // C++ TODO: There is no native C++ equivalent to 'toString':
      buffer->append(Float::toString(metaData->boost));
    }
  }
  if (buffer->length() != 0) {
    buffer->append(L"]");
    buffer->append(L",");
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return buffer->toString() + innerQuery->toString(field);
}

shared_ptr<Weight>
ContextQuery::createWeight(shared_ptr<IndexSearcher> searcher, bool needsScores,
                           float boost) 
{
  shared_ptr<CompletionWeight> *const innerWeight =
      (std::static_pointer_cast<CompletionWeight>(
          innerQuery->createWeight(searcher, needsScores, boost)));
  shared_ptr<Automaton> *const innerAutomaton = innerWeight->getAutomaton();

  // If the inner automaton matches nothing, then we return an empty weight to
  // avoid traversing all contexts during scoring.
  if (innerAutomaton->getNumStates() == 0) {
    return make_shared<CompletionWeight>(shared_from_this(), innerAutomaton);
  }

  // if separators are preserved the fst contains a SEP_LABEL
  // behind each gap. To have a matching automaton, we need to
  // include the SEP_LABEL in the query as well
  shared_ptr<Automaton> optionalSepLabel = Operations::optional(
      Automata::makeChar(ConcatenateGraphFilter::SEP_LABEL));
  shared_ptr<Automaton> prefixAutomaton =
      Operations::concatenate(optionalSepLabel, innerAutomaton);
  shared_ptr<Automaton> contextsAutomaton = Operations::concatenate(
      toContextAutomaton(contexts, matchAllContexts), prefixAutomaton);
  contextsAutomaton = Operations::determinize(
      contextsAutomaton, Operations::DEFAULT_MAX_DETERMINIZED_STATES);

  const unordered_map<std::shared_ptr<IntsRef>, float> contextMap =
      unordered_map<std::shared_ptr<IntsRef>, float>(contexts.size());
  const set<int> contextLengths = set<int>();
  for (auto entry : contexts) {
    shared_ptr<ContextMetaData> contextMetaData = entry.second;
    contextMap.emplace(entry.first, contextMetaData->boost);
    contextLengths.insert(entry.first->length);
  }
  std::deque<int> contextLengthArray(contextLengths.size());
  const Iterator<int> iterator = contextLengths.descendingIterator();
  for (int i = 0; iterator->hasNext(); i++) {
    contextLengthArray[i] = iterator->next();
  }
  return make_shared<ContextCompletionWeight>(shared_from_this(),
                                              contextsAutomaton, innerWeight,
                                              contextMap, contextLengthArray);
}

shared_ptr<Automaton> ContextQuery::toContextAutomaton(
    unordered_map<std::shared_ptr<IntsRef>, std::shared_ptr<ContextMetaData>>
        &contexts,
    bool const matchAllContexts)
{
  shared_ptr<Automaton> *const matchAllAutomaton =
      Operations::repeat(Automata::makeAnyString());
  shared_ptr<Automaton> *const sep =
      Automata::makeChar(ContextSuggestField::CONTEXT_SEPARATOR);
  if (matchAllContexts || contexts.empty()) {
    return Operations::concatenate(matchAllAutomaton, sep);
  } else {
    shared_ptr<Automaton> contextsAutomaton = nullptr;
    for (auto entry : contexts) {
      shared_ptr<ContextMetaData> *const contextMetaData = entry.second;
      shared_ptr<IntsRef> *const ref = entry.first;
      shared_ptr<Automaton> contextAutomaton =
          Automata::makeString(ref->ints, ref->offset, ref->length);
      if (contextMetaData->exact == false) {
        contextAutomaton =
            Operations::concatenate(contextAutomaton, matchAllAutomaton);
      }
      contextAutomaton = Operations::concatenate(contextAutomaton, sep);
      if (contextsAutomaton == nullptr) {
        contextsAutomaton = contextAutomaton;
      } else {
        contextsAutomaton =
            Operations::union_(contextsAutomaton, contextAutomaton);
      }
    }
    return contextsAutomaton;
  }
}

ContextQuery::ContextMetaData::ContextMetaData(float boost, bool exact)
    : boost(boost), exact(exact)
{
}

ContextQuery::ContextCompletionWeight::ContextCompletionWeight(
    shared_ptr<CompletionQuery> query, shared_ptr<Automaton> automaton,
    shared_ptr<CompletionWeight> innerWeight,
    unordered_map<std::shared_ptr<IntsRef>, float> &contextMap,
    std::deque<int> &contextLengths) 
    : CompletionWeight(query, automaton), contextMap(contextMap),
      contextLengths(contextLengths), innerWeight(innerWeight)
{
}

void ContextQuery::ContextCompletionWeight::setNextMatch(
    shared_ptr<IntsRef> pathPrefix)
{
  shared_ptr<IntsRef> ref = pathPrefix->clone();

  // check if the pathPrefix matches any
  // defined context, longer context first
  for (auto contextLength : contextLengths) {
    if (contextLength > pathPrefix->length) {
      continue;
    }
    ref->length = contextLength;
    if (contextMap.find(ref) != contextMap.end()) {
      currentBoost = contextMap[ref];
      ref->length = pathPrefix->length;
      setInnerWeight(ref, contextLength);
      return;
    }
  }
  // unknown context
  ref->length = pathPrefix->length;
  currentBoost = 0.0f;
  setInnerWeight(ref, 0);
}

void ContextQuery::ContextCompletionWeight::setInnerWeight(
    shared_ptr<IntsRef> ref, int offset)
{
  shared_ptr<IntsRefBuilder> refBuilder = make_shared<IntsRefBuilder>();
  for (int i = offset; i < ref->length; i++) {
    if (ref->ints[ref->offset + i] == ContextSuggestField::CONTEXT_SEPARATOR) {
      if (i > 0) {
        refBuilder->copyInts(ref->ints, ref->offset, i);
        currentContext =
            Util::toBytesRef(refBuilder->get(), scratch)->utf8ToString();
      } else {
        currentContext.reset();
      }
      ref->offset = ++i;
      assert((ref->offset < ref->length,
              L"input should not end with the context separator"));
      if (ref->ints[i] == ConcatenateGraphFilter::SEP_LABEL) {
        ref->offset++;
        assert(
            (ref->offset < ref->length, L"input should not end with a context "
                                        L"separator followed by SEP_LABEL"));
      }
      ref->length = ref->length - ref->offset;
      refBuilder->copyInts(ref->ints, ref->offset, ref->length);
      innerWeight->setNextMatch(refBuilder->get());
      return;
    }
  }
}

shared_ptr<std::wstring> ContextQuery::ContextCompletionWeight::context()
{
  return currentContext;
}

float ContextQuery::ContextCompletionWeight::boost()
{
  return currentBoost + innerWeight->boost();
}

bool ContextQuery::equals(any o)
{
  throw make_shared<UnsupportedOperationException>();
}

int ContextQuery::hashCode()
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::search::suggest::document