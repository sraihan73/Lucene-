using namespace std;

#include "TermAutomatonQuery.h"

namespace org::apache::lucene::search
{
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using TermState = org::apache::lucene::index::TermState;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using SpanNearQuery = org::apache::lucene::search::spans::SpanNearQuery;
using BytesRef = org::apache::lucene::util::BytesRef;
using IntsRef = org::apache::lucene::util::IntsRef;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using Operations = org::apache::lucene::util::automaton::Operations;
using Transition = org::apache::lucene::util::automaton::Transition;
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

TermAutomatonQuery::TermAutomatonQuery(const wstring &field)
    : field(field), builder(make_shared<Automaton::Builder>())
{
}

int TermAutomatonQuery::createState() { return builder->createState(); }

void TermAutomatonQuery::setAccept(int state, bool accept)
{
  builder->setAccept(state, accept);
}

void TermAutomatonQuery::addTransition(int source, int dest,
                                       const wstring &term)
{
  addTransition(source, dest, make_shared<BytesRef>(term));
}

void TermAutomatonQuery::addTransition(int source, int dest,
                                       shared_ptr<BytesRef> term)
{
  if (term == nullptr) {
    throw make_shared<NullPointerException>(L"term should not be null");
  }
  builder->addTransition(source, dest, getTermID(term));
}

void TermAutomatonQuery::addAnyTransition(int source, int dest)
{
  builder->addTransition(source, dest, getTermID(nullptr));
}

void TermAutomatonQuery::finish()
{
  finish(Operations::DEFAULT_MAX_DETERMINIZED_STATES);
}

void TermAutomatonQuery::finish(int maxDeterminizedStates)
{
  shared_ptr<Automaton> automaton = builder->finish();

  // System.out.println("before det:\n" + automaton.toDot());

  shared_ptr<Transition> t = make_shared<Transition>();

  // TODO: should we add "eps back to initial node" for all states,
  // and det that?  then we don't need to revisit initial node at
  // every position?  but automaton could blow up?  And, this makes it
  // harder to skip useless positions at search time?

  if (anyTermID != -1) {

    // Make sure there are no leading or trailing ANY:
    int count = automaton->initTransition(0, t);
    for (int i = 0; i < count; i++) {
      automaton->getNextTransition(t);
      if (anyTermID >= t->min && anyTermID <= t->max) {
        throw make_shared<IllegalStateException>(
            L"automaton cannot lead with an ANY transition");
      }
    }

    int numStates = automaton->getNumStates();
    for (int i = 0; i < numStates; i++) {
      count = automaton->initTransition(i, t);
      for (int j = 0; j < count; j++) {
        automaton->getNextTransition(t);
        if (automaton->isAccept(t->dest) && anyTermID >= t->min &&
            anyTermID <= t->max) {
          throw make_shared<IllegalStateException>(
              L"automaton cannot end with an ANY transition");
        }
      }
    }

    int termCount = termToID.size();

    // We have to carefully translate these transitions so automaton
    // realizes they also match all other terms:
    shared_ptr<Automaton> newAutomaton = make_shared<Automaton>();
    for (int i = 0; i < numStates; i++) {
      newAutomaton->createState();
      newAutomaton->setAccept(i, automaton->isAccept(i));
    }

    for (int i = 0; i < numStates; i++) {
      count = automaton->initTransition(i, t);
      for (int j = 0; j < count; j++) {
        automaton->getNextTransition(t);
        int min, max;
        if (t->min <= anyTermID && anyTermID <= t->max) {
          // Match any term
          min = 0;
          max = termCount - 1;
        } else {
          min = t->min;
          max = t->max;
        }
        newAutomaton->addTransition(t->source, t->dest, min, max);
      }
    }
    newAutomaton->finishState();
    automaton = newAutomaton;
  }

  det = Operations::removeDeadStates(
      Operations::determinize(automaton, maxDeterminizedStates));

  if (det->isAccept(0)) {
    throw make_shared<IllegalStateException>(L"cannot accept the empty string");
  }
}

shared_ptr<Weight>
TermAutomatonQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                 bool needsScores,
                                 float boost) 
{
  shared_ptr<IndexReaderContext> context = searcher->getTopReaderContext();
  unordered_map<int, std::shared_ptr<TermContext>> termStates =
      unordered_map<int, std::shared_ptr<TermContext>>();

  for (auto ent : termToID) {
    if (ent.first != nullptr) {
      termStates.emplace(
          ent.second,
          TermContext::build(context, make_shared<Term>(field, ent.first)));
    }
  }

  return make_shared<TermAutomatonWeight>(shared_from_this(), det, searcher,
                                          termStates, boost);
}

wstring TermAutomatonQuery::toString(const wstring &field)
{
  // TODO: what really am I supposed to do with the incoming field...
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"TermAutomatonQuery(field=");
  sb->append(this->field);
  if (det != nullptr) {
    sb->append(L" numStates=");
    sb->append(det->getNumStates());
  }
  sb->append(L')');
  return sb->toString();
}

int TermAutomatonQuery::getTermID(shared_ptr<BytesRef> term)
{
  optional<int> id = termToID[term];
  if (!id) {
    id = termToID.size();
    if (term != nullptr) {
      term = BytesRef::deepCopyOf(term);
    }
    termToID.emplace(term, id);
    idToTerm.emplace(id, term);
    if (term == nullptr) {
      anyTermID = id;
    }
  }

  return id;
}

bool TermAutomatonQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool TermAutomatonQuery::checkFinished(shared_ptr<TermAutomatonQuery> q)
{
  if (q->det == nullptr) {
    throw make_shared<IllegalStateException>(L"Call finish first on: " + q);
  }
  return true;
}

bool TermAutomatonQuery::equalsTo(shared_ptr<TermAutomatonQuery> other)
{
  return checkFinished(shared_from_this()) && checkFinished(other) &&
         other == shared_from_this();
}

int TermAutomatonQuery::hashCode()
{
  checkFinished(shared_from_this());
  // LUCENE-7295: this used to be very awkward toDot() call; it is safer to
  // assume that no two instances are equivalent instead (until somebody finds a
  // better way to check on automaton equivalence quickly).
  return System::identityHashCode(shared_from_this());
}

wstring TermAutomatonQuery::toDot()
{

  // TODO: refactor & share with Automaton.toDot!

  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"digraph Automaton {\n");
  b->append(L"  rankdir = LR\n");
  constexpr int numStates = det->getNumStates();
  if (numStates > 0) {
    b->append(L"  initial [shape=plaintext,label=\"0\"]\n");
    b->append(L"  initial -> 0\n");
  }

  shared_ptr<Transition> t = make_shared<Transition>();
  for (int state = 0; state < numStates; state++) {
    b->append(L"  ");
    b->append(state);
    if (det->isAccept(state)) {
      b->append(L" [shape=doublecircle,label=\"" + to_wstring(state) +
                L"\"]\n");
    } else {
      b->append(L" [shape=circle,label=\"" + to_wstring(state) + L"\"]\n");
    }
    int numTransitions = det->initTransition(state, t);
    for (int i = 0; i < numTransitions; i++) {
      det->getNextTransition(t);
      assert(t->max >= t->min);
      for (int j = t->min; j <= t->max; j++) {
        b->append(L"  ");
        b->append(state);
        b->append(L" -> ");
        b->append(t->dest);
        b->append(L" [label=\"");
        if (j == anyTermID) {
          b->append(L'*');
        } else {
          b->append(idToTerm[j]->utf8ToString());
        }
        b->append(L"\"]\n");
      }
    }
  }
  b->append(L'}');
  return b->toString();
}

TermAutomatonQuery::EnumAndScorer::EnumAndScorer(
    int termID, shared_ptr<PostingsEnum> posEnum)
    : termID(termID), posEnum(posEnum)
{
}

TermAutomatonQuery::TermAutomatonWeight::TermAutomatonWeight(
    shared_ptr<TermAutomatonQuery> outerInstance,
    shared_ptr<Automaton> automaton, shared_ptr<IndexSearcher> searcher,
    unordered_map<int, std::shared_ptr<TermContext>> &termStates,
    float boost) 
    : Weight(TermAutomatonQuery::this), automaton(automaton),
      termStates(termStates),
      stats(similarity->computeWeight(
          boost, searcher->collectionStatistics(outerInstance->field),
          {allTermStats::toArray(std::deque<std::shared_ptr<TermStatistics>>(
              allTermStats->size()))})),
      similarity(searcher->getSimilarity(true)), outerInstance(outerInstance)
{
  deque<std::shared_ptr<TermStatistics>> allTermStats =
      deque<std::shared_ptr<TermStatistics>>();
  for (auto ent : outerInstance->idToTerm) {
    optional<int> termID = ent.first;
    if (ent.second != nullptr) {
      allTermStats.push_back(searcher->termStatistics(
          make_shared<Term>(outerInstance->field, ent.second),
          termStates[termID]));
    }
  }
}

void TermAutomatonQuery::TermAutomatonWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  for (auto text : outerInstance->termToID) {
    if (text->first != nullptr) {
      terms->add(make_shared<Term>(outerInstance->field, text->first));
    }
  }
}

wstring TermAutomatonQuery::TermAutomatonWeight::toString()
{
  return L"weight(" + outerInstance + L")";
}

shared_ptr<Scorer> TermAutomatonQuery::TermAutomatonWeight::scorer(
    shared_ptr<LeafReaderContext> context) 
{

  // Initialize the enums; null for a given slot means that term didn't appear
  // in this reader
  std::deque<std::shared_ptr<EnumAndScorer>> enums(
      outerInstance->idToTerm.size());

  bool any = false;
  for (auto ent : termStates) {
    shared_ptr<TermContext> termContext = ent.second;
    assert((termContext->wasBuiltFor(ReaderUtil::getTopLevelContext(context)),
            L"The top-reader used to create Weight is not the same as the "
            L"current reader's top-reader (" +
                ReaderUtil::getTopLevelContext(context)));
    shared_ptr<BytesRef> term = outerInstance->idToTerm[ent.first];
    shared_ptr<TermState> state = termContext->get(context->ord);
    if (state != nullptr) {
      shared_ptr<TermsEnum> termsEnum =
          context->reader()->terms(outerInstance->field)->begin();
      termsEnum->seekExact(term, state);
      enums[ent.first] = make_shared<EnumAndScorer>(
          ent.first, termsEnum->postings(nullptr, PostingsEnum::POSITIONS));
      any = true;
    }
  }

  if (any) {
    return make_shared<TermAutomatonScorer>(
        shared_from_this(), enums, outerInstance->anyTermID,
        outerInstance->idToTerm, similarity->simScorer(stats, context));
  } else {
    return nullptr;
  }
}

bool TermAutomatonQuery::TermAutomatonWeight::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return true;
}

shared_ptr<Explanation> TermAutomatonQuery::TermAutomatonWeight::explain(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  // TODO
  return nullptr;
}

shared_ptr<Query>
TermAutomatonQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  if (Operations::isEmpty(det)) {
    return make_shared<MatchNoDocsQuery>();
  }

  shared_ptr<IntsRef> single = Operations::getSingleton(det);
  if (single != nullptr && single->length == 1) {
    return make_shared<TermQuery>(
        make_shared<Term>(field, idToTerm[single->ints[single->offset]]));
  }

  // TODO: can PhraseQuery really handle multiple terms at the same position? If
  // so, why do we even have MultiPhraseQuery?

  // Try for either PhraseQuery or MultiPhraseQuery, which only works when the
  // automaton is a sausage:
  shared_ptr<MultiPhraseQuery::Builder> mpq =
      make_shared<MultiPhraseQuery::Builder>();
  shared_ptr<PhraseQuery::Builder> pq = make_shared<PhraseQuery::Builder>();

  shared_ptr<Transition> t = make_shared<Transition>();
  int state = 0;
  int pos = 0;
  while (true) {
    int count = det->initTransition(state, t);
    if (count == 0) {
      if (det->isAccept(state) == false) {
        mpq.reset();
        pq.reset();
      }
      break;
    } else if (det->isAccept(state)) {
      mpq.reset();
      pq.reset();
      break;
    }
    int dest = -1;
    deque<std::shared_ptr<Term>> terms = deque<std::shared_ptr<Term>>();
    bool matchesAny = false;
    for (int i = 0; i < count; i++) {
      det->getNextTransition(t);
      if (i == 0) {
        dest = t->dest;
      } else if (dest != t->dest) {
        mpq.reset();
        pq.reset();
        goto queryBreak;
      }

      matchesAny |= anyTermID >= t->min && anyTermID <= t->max;

      if (matchesAny == false) {
        for (int termID = t->min; termID <= t->max; termID++) {
          terms.push_back(make_shared<Term>(field, idToTerm[termID]));
        }
      }
    }
    if (matchesAny == false) {
      mpq->add(terms.toArray(std::deque<std::shared_ptr<Term>>(terms.size())),
               pos);
      if (pq != nullptr) {
        if (terms.size() == 1) {
          pq->add(terms[0], pos);
        } else {
          pq.reset();
        }
      }
    }
    state = dest;
    pos++;
  queryContinue:;
  }
queryBreak:

  if (pq != nullptr) {
    return pq->build();
  } else if (mpq != nullptr) {
    return mpq->build();
  }

  // TODO: we could maybe also rewrite to union of PhraseQuery (pull all finite
  // strings) if it's "worth it"?
  return shared_from_this();
}
} // namespace org::apache::lucene::search