using namespace std;

#include "ValueSource.h"

namespace org::apache::lucene::queries::function
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Explanation = org::apache::lucene::search::Explanation;
using FieldComparator = org::apache::lucene::search::FieldComparator;
using FieldComparatorSource =
    org::apache::lucene::search::FieldComparatorSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using LongValues = org::apache::lucene::search::LongValues;
using LongValuesSource = org::apache::lucene::search::LongValuesSource;
using Scorer = org::apache::lucene::search::Scorer;
using SimpleFieldComparator =
    org::apache::lucene::search::SimpleFieldComparator;
using SortField = org::apache::lucene::search::SortField;

wstring ValueSource::toString() { return description(); }

void ValueSource::createWeight(
    unordered_map context,
    shared_ptr<IndexSearcher> searcher) 
{
}

unordered_map ValueSource::newContext(shared_ptr<IndexSearcher> searcher)
{
  unordered_map context = make_shared<IdentityHashMap>();
  context.emplace(L"searcher", searcher);
  return context;
}

ValueSource::FakeScorer::FakeScorer()
    : org::apache::lucene::search::Scorer(nullptr)
{
}

int ValueSource::FakeScorer::docID() { return current; }

float ValueSource::FakeScorer::score()  { return score_; }

shared_ptr<DocIdSetIterator> ValueSource::FakeScorer::iterator()
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<LongValuesSource> ValueSource::asLongValuesSource()
{
  return make_shared<WrappedLongValuesSource>(shared_from_this());
}

ValueSource::WrappedLongValuesSource::WrappedLongValuesSource(
    shared_ptr<ValueSource> in_)
    : in_(in_)
{
}

shared_ptr<LongValues> ValueSource::WrappedLongValuesSource::getValues(
    shared_ptr<LeafReaderContext> ctx,
    shared_ptr<DoubleValues> scores) 
{
  unordered_map context = make_shared<IdentityHashMap<>>();
  shared_ptr<FakeScorer> scorer = make_shared<FakeScorer>();
  context.emplace(L"scorer", scorer);
  shared_ptr<FunctionValues> *const fv = in_->getValues(context, ctx);
  return make_shared<LongValuesAnonymousInnerClass>(shared_from_this(), scores,
                                                    scorer, fv);
}

ValueSource::WrappedLongValuesSource::LongValuesAnonymousInnerClass::
    LongValuesAnonymousInnerClass(
        shared_ptr<WrappedLongValuesSource> outerInstance,
        shared_ptr<DoubleValues> scores,
        shared_ptr<
            org::apache::lucene::queries::function::ValueSource::FakeScorer>
            scorer,
        shared_ptr<org::apache::lucene::queries::function::FunctionValues> fv)
{
  this->outerInstance = outerInstance;
  this->scores = scores;
  this->scorer = scorer;
  this->fv = fv;
}

int64_t ValueSource::WrappedLongValuesSource::LongValuesAnonymousInnerClass::
    longValue() 
{
  return fv->longVal(scorer->current);
}

bool ValueSource::WrappedLongValuesSource::LongValuesAnonymousInnerClass::
    advanceExact(int doc) 
{
  scorer->current = doc;
  if (scores != nullptr && scores->advanceExact(doc)) {
    scorer->score_ = static_cast<float>(scores->doubleValue());
  } else {
    scorer->score_ = 0;
  }
  return fv->exists(doc);
}

bool ValueSource::WrappedLongValuesSource::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return false;
}

bool ValueSource::WrappedLongValuesSource::needsScores() { return false; }

bool ValueSource::WrappedLongValuesSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<WrappedLongValuesSource> that =
      any_cast<std::shared_ptr<WrappedLongValuesSource>>(o);
  return Objects::equals(in_, that->in_);
}

int ValueSource::WrappedLongValuesSource::hashCode()
{
  return Objects::hash(in_);
}

wstring ValueSource::WrappedLongValuesSource::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return in_->toString();
}

shared_ptr<LongValuesSource> ValueSource::WrappedLongValuesSource::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  return shared_from_this();
}

shared_ptr<DoubleValuesSource> ValueSource::asDoubleValuesSource()
{
  return make_shared<WrappedDoubleValuesSource>(shared_from_this());
}

ValueSource::WrappedDoubleValuesSource::WrappedDoubleValuesSource(
    shared_ptr<ValueSource> in_)
    : in_(in_)
{
}

shared_ptr<DoubleValues> ValueSource::WrappedDoubleValuesSource::getValues(
    shared_ptr<LeafReaderContext> ctx,
    shared_ptr<DoubleValues> scores) 
{
  unordered_map context = unordered_map<>();
  shared_ptr<FakeScorer> scorer = make_shared<FakeScorer>();
  context.emplace(L"scorer", scorer);
  context.emplace(L"searcher", searcher);
  shared_ptr<FunctionValues> fv = in_->getValues(context, ctx);
  return make_shared<DoubleValuesAnonymousInnerClass>(shared_from_this(),
                                                      scores, scorer, fv);
}

ValueSource::WrappedDoubleValuesSource::DoubleValuesAnonymousInnerClass::
    DoubleValuesAnonymousInnerClass(
        shared_ptr<WrappedDoubleValuesSource> outerInstance,
        shared_ptr<DoubleValues> scores,
        shared_ptr<
            org::apache::lucene::queries::function::ValueSource::FakeScorer>
            scorer,
        shared_ptr<org::apache::lucene::queries::function::FunctionValues> fv)
{
  this->outerInstance = outerInstance;
  this->scores = scores;
  this->scorer = scorer;
  this->fv = fv;
}

double ValueSource::WrappedDoubleValuesSource::DoubleValuesAnonymousInnerClass::
    doubleValue() 
{
  return fv->doubleVal(scorer->current);
}

bool ValueSource::WrappedDoubleValuesSource::DoubleValuesAnonymousInnerClass::
    advanceExact(int doc) 
{
  scorer->current = doc;
  if (scores != nullptr && scores->advanceExact(doc)) {
    scorer->score_ = static_cast<float>(scores->doubleValue());
  } else {
    scorer->score_ = 0;
  }
  return fv->exists(doc);
}

bool ValueSource::WrappedDoubleValuesSource::needsScores()
{
  return true; // be on the safe side
}

bool ValueSource::WrappedDoubleValuesSource::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return false;
}

shared_ptr<Explanation> ValueSource::WrappedDoubleValuesSource::explain(
    shared_ptr<LeafReaderContext> ctx, int docId,
    shared_ptr<Explanation> scoreExplanation) 
{
  unordered_map context = unordered_map<>();
  shared_ptr<FakeScorer> scorer = make_shared<FakeScorer>();
  scorer->score_ = scoreExplanation->getValue();
  context.emplace(L"scorer", scorer);
  context.emplace(L"searcher", searcher);
  shared_ptr<FunctionValues> fv = in_->getValues(context, ctx);
  return fv->explain(docId);
}

shared_ptr<DoubleValuesSource> ValueSource::WrappedDoubleValuesSource::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  this->searcher = searcher;
  return shared_from_this();
}

bool ValueSource::WrappedDoubleValuesSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<WrappedDoubleValuesSource> that =
      any_cast<std::shared_ptr<WrappedDoubleValuesSource>>(o);
  return Objects::equals(in_, that->in_);
}

int ValueSource::WrappedDoubleValuesSource::hashCode()
{
  return Objects::hash(in_);
}

wstring ValueSource::WrappedDoubleValuesSource::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return in_->toString();
}

shared_ptr<ValueSource>
ValueSource::fromDoubleValuesSource(shared_ptr<DoubleValuesSource> in_)
{
  return make_shared<FromDoubleValuesSource>(in_);
}

ValueSource::FromDoubleValuesSource::FromDoubleValuesSource(
    shared_ptr<DoubleValuesSource> in_)
    : in_(in_)
{
}

shared_ptr<FunctionValues> ValueSource::FromDoubleValuesSource::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<Scorer> scorer =
      std::static_pointer_cast<Scorer>(context[L"scorer"]);
  shared_ptr<DoubleValues> scores =
      scorer == nullptr ? nullptr : DoubleValuesSource::fromScorer(scorer);

  shared_ptr<IndexSearcher> searcher =
      std::static_pointer_cast<IndexSearcher>(context[L"searcher"]);
  shared_ptr<DoubleValues> inner;
  if (searcher != nullptr) {
    inner = in_->rewrite(searcher)->getValues(readerContext, scores);
  } else {
    inner = in_->getValues(readerContext, scores);
  }

  return make_shared<FunctionValuesAnonymousInnerClass>(shared_from_this(),
                                                        inner);
}

ValueSource::FromDoubleValuesSource::FunctionValuesAnonymousInnerClass::
    FunctionValuesAnonymousInnerClass(
        shared_ptr<FromDoubleValuesSource> outerInstance,
        shared_ptr<DoubleValues> inner)
{
  this->outerInstance = outerInstance;
  this->inner = inner;
}

wstring ValueSource::FromDoubleValuesSource::FunctionValuesAnonymousInnerClass::
    toString(int doc) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return outerInstance->in_->toString();
}

float ValueSource::FromDoubleValuesSource::FunctionValuesAnonymousInnerClass::
    floatVal(int doc) 
{
  if (inner->advanceExact(doc) == false) {
    return 0;
  }
  return static_cast<float>(inner->doubleValue());
}

double ValueSource::FromDoubleValuesSource::FunctionValuesAnonymousInnerClass::
    doubleVal(int doc) 
{
  if (inner->advanceExact(doc) == false) {
    return 0;
  }
  return inner->doubleValue();
}

bool ValueSource::FromDoubleValuesSource::FunctionValuesAnonymousInnerClass::
    exists(int doc) 
{
  return inner->advanceExact(doc);
}

bool ValueSource::FromDoubleValuesSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<FromDoubleValuesSource> that =
      any_cast<std::shared_ptr<FromDoubleValuesSource>>(o);
  return Objects::equals(in_, that->in_);
}

int ValueSource::FromDoubleValuesSource::hashCode()
{
  return Objects::hash(in_);
}

wstring ValueSource::FromDoubleValuesSource::description()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return in_->toString();
}

shared_ptr<SortField> ValueSource::getSortField(bool reverse)
{
  return make_shared<ValueSourceSortField>(shared_from_this(), reverse);
}

ValueSource::ValueSourceSortField::ValueSourceSortField(
    shared_ptr<ValueSource> outerInstance, bool reverse)
    : org::apache::lucene::search::SortField(
          description(), SortField::Type::REWRITEABLE, reverse),
      outerInstance(outerInstance)
{
}

shared_ptr<SortField> ValueSource::ValueSourceSortField::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  unordered_map context = newContext(searcher);
  outerInstance->createWeight(context, searcher);
  return make_shared<SortField>(
      getField(),
      make_shared<ValueSourceComparatorSource>(outerInstance, context),
      getReverse());
}

ValueSource::ValueSourceComparatorSource::ValueSourceComparatorSource(
    shared_ptr<ValueSource> outerInstance, unordered_map context)
    : context(context), outerInstance(outerInstance)
{
}

shared_ptr<FieldComparator<double>>
ValueSource::ValueSourceComparatorSource::newComparator(
    const wstring &fieldname, int numHits, int sortPos, bool reversed)
{
  return make_shared<ValueSourceComparator>(outerInstance, context, numHits);
}

ValueSource::ValueSourceComparator::ValueSourceComparator(
    shared_ptr<ValueSource> outerInstance, unordered_map fcontext, int numHits)
    : values(std::deque<double>(numHits)), fcontext(fcontext),
      outerInstance(outerInstance)
{
}

int ValueSource::ValueSourceComparator::compare(int slot1, int slot2)
{
  return Double::compare(values[slot1], values[slot2]);
}

int ValueSource::ValueSourceComparator::compareBottom(int doc) throw(
    IOException)
{
  return Double::compare(bottom, docVals->doubleVal(doc));
}

void ValueSource::ValueSourceComparator::copy(int slot,
                                              int doc) 
{
  values[slot] = docVals->doubleVal(doc);
}

void ValueSource::ValueSourceComparator::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  docVals = outerInstance->getValues(fcontext, context);
}

void ValueSource::ValueSourceComparator::setBottom(int const bottom)
{
  this->bottom = values[bottom];
}

void ValueSource::ValueSourceComparator::setTopValue(optional<double> &value)
{
  this->topValue = value.value();
}

optional<double> ValueSource::ValueSourceComparator::value(int slot)
{
  return values[slot];
}

int ValueSource::ValueSourceComparator::compareTop(int doc) 
{
  constexpr double docValue = docVals->doubleVal(doc);
  return Double::compare(topValue, docValue);
}
} // namespace org::apache::lucene::queries::function