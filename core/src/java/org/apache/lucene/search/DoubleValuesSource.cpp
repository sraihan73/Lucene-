using namespace std;

#include "DoubleValuesSource.h"

namespace org::apache::lucene::search
{
using DocValues = org::apache::lucene::index::DocValues;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;

shared_ptr<Explanation> DoubleValuesSource::explain(
    shared_ptr<LeafReaderContext> ctx, int docId,
    shared_ptr<Explanation> scoreExplanation) 
{
  shared_ptr<DoubleValues> dv =
      getValues(ctx, DoubleValuesSource::constant(scoreExplanation->getValue())
                         ->getValues(ctx, nullptr));
  if (dv->advanceExact(docId)) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return Explanation::match(static_cast<float>(dv->doubleValue()),
                              this->toString());
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Explanation::noMatch(this->toString());
}

shared_ptr<SortField> DoubleValuesSource::getSortField(bool reverse)
{
  return make_shared<DoubleValuesSortField>(shared_from_this(), reverse);
}

shared_ptr<LongValuesSource> DoubleValuesSource::toLongValuesSource()
{
  return make_shared<LongDoubleValuesSource>(shared_from_this());
}

DoubleValuesSource::LongDoubleValuesSource::LongDoubleValuesSource(
    shared_ptr<DoubleValuesSource> inner)
    : inner(inner)
{
}

shared_ptr<LongValues> DoubleValuesSource::LongDoubleValuesSource::getValues(
    shared_ptr<LeafReaderContext> ctx,
    shared_ptr<DoubleValues> scores) 
{
  shared_ptr<DoubleValues> in_ = inner->getValues(ctx, scores);
  return make_shared<LongValuesAnonymousInnerClass>(shared_from_this(), in_);
}

DoubleValuesSource::LongDoubleValuesSource::LongValuesAnonymousInnerClass::
    LongValuesAnonymousInnerClass(
        shared_ptr<LongDoubleValuesSource> outerInstance,
        shared_ptr<org::apache::lucene::search::DoubleValues> in_)
{
  this->outerInstance = outerInstance;
  this->in_ = in_;
}

int64_t DoubleValuesSource::LongDoubleValuesSource::
    LongValuesAnonymousInnerClass::longValue() 
{
  return static_cast<int64_t>(in_->doubleValue());
}

bool DoubleValuesSource::LongDoubleValuesSource::LongValuesAnonymousInnerClass::
    advanceExact(int doc) 
{
  return in_->advanceExact(doc);
}

bool DoubleValuesSource::LongDoubleValuesSource::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return inner->isCacheable(ctx);
}

bool DoubleValuesSource::LongDoubleValuesSource::needsScores()
{
  return inner->needsScores();
}

bool DoubleValuesSource::LongDoubleValuesSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<LongDoubleValuesSource> that =
      any_cast<std::shared_ptr<LongDoubleValuesSource>>(o);
  return Objects::equals(inner, that->inner);
}

int DoubleValuesSource::LongDoubleValuesSource::hashCode()
{
  return Objects::hash(inner);
}

wstring DoubleValuesSource::LongDoubleValuesSource::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"long(" + inner->toString() + L")";
}

shared_ptr<LongValuesSource>
DoubleValuesSource::LongDoubleValuesSource::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  return inner->rewrite(searcher)->toLongValuesSource();
}

shared_ptr<DoubleValuesSource>
DoubleValuesSource::fromField(const wstring &field,
                              function<double(int64_t)> &decoder)
{
  return make_shared<FieldValuesSource>(field, decoder);
}

shared_ptr<DoubleValuesSource>
DoubleValuesSource::fromDoubleField(const wstring &field)
{
  return fromField(field, optional<double>::longBitsToDouble);
}

shared_ptr<DoubleValuesSource>
DoubleValuesSource::fromFloatField(const wstring &field)
{
  return fromField(field, [&](v) {
    static_cast<double>(Float::intBitsToFloat(static_cast<int>(v)));
  });
}

shared_ptr<DoubleValuesSource>
DoubleValuesSource::fromLongField(const wstring &field)
{
  return fromField(field, [&](v) { static_cast<double>(v); });
}

shared_ptr<DoubleValuesSource>
DoubleValuesSource::fromIntField(const wstring &field)
{
  return fromLongField(field);
}

const shared_ptr<DoubleValuesSource> DoubleValuesSource::SCORES =
    make_shared<DoubleValuesSourceAnonymousInnerClass>();

DoubleValuesSource::DoubleValuesSourceAnonymousInnerClass::
    DoubleValuesSourceAnonymousInnerClass()
{
}

shared_ptr<DoubleValues>
DoubleValuesSource::DoubleValuesSourceAnonymousInnerClass::getValues(
    shared_ptr<LeafReaderContext> ctx,
    shared_ptr<DoubleValues> scores) 
{
  assert(scores != nullptr);
  return scores;
}

bool DoubleValuesSource::DoubleValuesSourceAnonymousInnerClass::needsScores()
{
  return true;
}

bool DoubleValuesSource::DoubleValuesSourceAnonymousInnerClass::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return false;
}

shared_ptr<Explanation>
DoubleValuesSource::DoubleValuesSourceAnonymousInnerClass::explain(
    shared_ptr<LeafReaderContext> ctx, int docId,
    shared_ptr<Explanation> scoreExplanation)
{
  return scoreExplanation;
}

int DoubleValuesSource::DoubleValuesSourceAnonymousInnerClass::hashCode()
{
  return 0;
}

bool DoubleValuesSource::DoubleValuesSourceAnonymousInnerClass::equals(any obj)
{
  return obj == shared_from_this();
}

wstring DoubleValuesSource::DoubleValuesSourceAnonymousInnerClass::toString()
{
  return L"scores";
}

shared_ptr<DoubleValuesSource>
DoubleValuesSource::DoubleValuesSourceAnonymousInnerClass::rewrite(
    shared_ptr<IndexSearcher> searcher)
{
  return shared_from_this();
}

shared_ptr<DoubleValuesSource> DoubleValuesSource::constant(double value)
{
  return make_shared<ConstantValuesSource>(value);
}

DoubleValuesSource::ConstantValuesSource::ConstantValuesSource(double value)
    : value(value)
{
}

shared_ptr<DoubleValuesSource>
DoubleValuesSource::ConstantValuesSource::rewrite(
    shared_ptr<IndexSearcher> searcher)
{
  return shared_from_this();
}

shared_ptr<DoubleValues> DoubleValuesSource::ConstantValuesSource::getValues(
    shared_ptr<LeafReaderContext> ctx,
    shared_ptr<DoubleValues> scores) 
{
  return make_shared<DoubleValuesAnonymousInnerClass>(shared_from_this());
}

DoubleValuesSource::ConstantValuesSource::DoubleValuesAnonymousInnerClass::
    DoubleValuesAnonymousInnerClass(
        shared_ptr<ConstantValuesSource> outerInstance)
{
  this->outerInstance = outerInstance;
}

double DoubleValuesSource::ConstantValuesSource::
    DoubleValuesAnonymousInnerClass::doubleValue() 
{
  return outerInstance->value;
}

bool DoubleValuesSource::ConstantValuesSource::DoubleValuesAnonymousInnerClass::
    advanceExact(int doc) 
{
  return true;
}

bool DoubleValuesSource::ConstantValuesSource::needsScores() { return false; }

bool DoubleValuesSource::ConstantValuesSource::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return true;
}

shared_ptr<Explanation> DoubleValuesSource::ConstantValuesSource::explain(
    shared_ptr<LeafReaderContext> ctx, int docId,
    shared_ptr<Explanation> scoreExplanation)
{
  return Explanation::match(static_cast<float>(value),
                            L"constant(" + to_wstring(value) + L")");
}

int DoubleValuesSource::ConstantValuesSource::hashCode()
{
  return Objects::hash(value);
}

bool DoubleValuesSource::ConstantValuesSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<ConstantValuesSource> that =
      any_cast<std::shared_ptr<ConstantValuesSource>>(o);
  return Double::compare(that->value, value) == 0;
}

wstring DoubleValuesSource::ConstantValuesSource::toString()
{
  return L"constant(" + to_wstring(value) + L")";
}

shared_ptr<DoubleValues>
DoubleValuesSource::fromScorer(shared_ptr<Scorer> scorer)
{
  return make_shared<DoubleValuesAnonymousInnerClass>(scorer);
}

DoubleValuesSource::DoubleValuesAnonymousInnerClass::
    DoubleValuesAnonymousInnerClass(
        shared_ptr<org::apache::lucene::search::Scorer> scorer)
{
  this->scorer = scorer;
}

double DoubleValuesSource::DoubleValuesAnonymousInnerClass::doubleValue() throw(
    IOException)
{
  return scorer->score();
}

bool DoubleValuesSource::DoubleValuesAnonymousInnerClass::advanceExact(
    int doc) 
{
  assert(scorer->docID() == doc);
  return true;
}

DoubleValuesSource::FieldValuesSource::FieldValuesSource(
    const wstring &field, function<double(int64_t)> &decoder)
    : field(field), decoder(decoder)
{
}

bool DoubleValuesSource::FieldValuesSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<FieldValuesSource> that =
      any_cast<std::shared_ptr<FieldValuesSource>>(o);
  return Objects::equals(field, that->field) &&
         Objects::equals(decoder, that->decoder);
}

wstring DoubleValuesSource::FieldValuesSource::toString()
{
  return L"double(" + field + L")";
}

int DoubleValuesSource::FieldValuesSource::hashCode()
{
  return Objects::hash(field, decoder);
}

shared_ptr<DoubleValues> DoubleValuesSource::FieldValuesSource::getValues(
    shared_ptr<LeafReaderContext> ctx,
    shared_ptr<DoubleValues> scores) 
{
  shared_ptr<NumericDocValues> *const values =
      DocValues::getNumeric(ctx->reader(), field);
  return make_shared<DoubleValuesAnonymousInnerClass2>(shared_from_this(),
                                                       values);
}

DoubleValuesSource::FieldValuesSource::DoubleValuesAnonymousInnerClass2::
    DoubleValuesAnonymousInnerClass2(
        shared_ptr<FieldValuesSource> outerInstance,
        shared_ptr<NumericDocValues> values)
{
  this->outerInstance = outerInstance;
  this->values = values;
}

double DoubleValuesSource::FieldValuesSource::DoubleValuesAnonymousInnerClass2::
    doubleValue() 
{
  return outerInstance->decoder->applyAsDouble(values->longValue());
}

bool DoubleValuesSource::FieldValuesSource::DoubleValuesAnonymousInnerClass2::
    advanceExact(int target) 
{
  return values->advanceExact(target);
}

bool DoubleValuesSource::FieldValuesSource::needsScores() { return false; }

bool DoubleValuesSource::FieldValuesSource::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return DocValues::isCacheable(ctx, {field});
}

shared_ptr<Explanation> DoubleValuesSource::FieldValuesSource::explain(
    shared_ptr<LeafReaderContext> ctx, int docId,
    shared_ptr<Explanation> scoreExplanation) 
{
  shared_ptr<DoubleValues> values = getValues(ctx, nullptr);
  if (values->advanceExact(docId)) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return Explanation::match(static_cast<float>(values->doubleValue()),
                              this->toString());
  } else {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return Explanation::noMatch(this->toString());
  }
}

shared_ptr<DoubleValuesSource> DoubleValuesSource::FieldValuesSource::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  return shared_from_this();
}

DoubleValuesSource::DoubleValuesSortField::DoubleValuesSortField(
    shared_ptr<DoubleValuesSource> producer, bool reverse)
    : SortField(producer->toString(),
                new DoubleValuesComparatorSource(producer), reverse),
      producer(producer)
{
}

bool DoubleValuesSource::DoubleValuesSortField::needsScores()
{
  return producer->needsScores();
}

wstring DoubleValuesSource::DoubleValuesSortField::toString()
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>(L"<");
  buffer->append(getField())->append(L">");
  if (reverse) {
    buffer->append(L"!");
  }
  return buffer->toString();
}

shared_ptr<SortField> DoubleValuesSource::DoubleValuesSortField::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  return make_shared<DoubleValuesSortField>(producer->rewrite(searcher),
                                            reverse);
}

DoubleValuesSource::DoubleValuesComparatorSource::DoubleValuesComparatorSource(
    shared_ptr<DoubleValuesSource> producer)
    : producer(producer)
{
}

shared_ptr<FieldComparator<double>>
DoubleValuesSource::DoubleValuesComparatorSource::newComparator(
    const wstring &fieldname, int numHits, int sortPos, bool reversed)
{
  return make_shared<DoubleComparatorAnonymousInnerClass>(shared_from_this(),
                                                          numHits, fieldname);
}

DoubleValuesSource::DoubleValuesComparatorSource::
    DoubleComparatorAnonymousInnerClass::DoubleComparatorAnonymousInnerClass(
        shared_ptr<DoubleValuesComparatorSource> outerInstance, int numHits,
        const wstring &fieldname)
    : FieldComparator::DoubleComparator(numHits, fieldname, 0.0)
{
  this->outerInstance = outerInstance;
  holder = make_shared<DoubleValuesHolder>();
}

shared_ptr<NumericDocValues> DoubleValuesSource::DoubleValuesComparatorSource::
    DoubleComparatorAnonymousInnerClass::getNumericDocValues(
        shared_ptr<LeafReaderContext> context,
        const wstring &field) 
{
  ctx = context;
  return asNumericDocValues(holder, optional<double>::doubleToLongBits);
}

void DoubleValuesSource::DoubleValuesComparatorSource::
    DoubleComparatorAnonymousInnerClass::setScorer(
        shared_ptr<Scorer> scorer) 
{
  holder->values = outerInstance->producer->getValues(ctx, fromScorer(scorer));
}

shared_ptr<NumericDocValues>
DoubleValuesSource::asNumericDocValues(shared_ptr<DoubleValuesHolder> in_,
                                       function<int64_t(double)> &converter)
{
  return make_shared<NumericDocValuesAnonymousInnerClass>(in_, converter);
}

DoubleValuesSource::NumericDocValuesAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass(
        shared_ptr<
            org::apache::lucene::search::DoubleValuesSource::DoubleValuesHolder>
            in_,
        function<int64_t(double)> &converter)
{
  this->in_ = in_;
  this->converter = converter;
}

int64_t
DoubleValuesSource::NumericDocValuesAnonymousInnerClass::longValue() throw(
    IOException)
{
  return converter(in_->values->doubleValue());
}

bool DoubleValuesSource::NumericDocValuesAnonymousInnerClass::advanceExact(
    int target) 
{
  return in_->values->advanceExact(target);
}

int DoubleValuesSource::NumericDocValuesAnonymousInnerClass::docID()
{
  throw make_shared<UnsupportedOperationException>();
}

int DoubleValuesSource::NumericDocValuesAnonymousInnerClass::nextDoc() throw(
    IOException)
{
  throw make_shared<UnsupportedOperationException>();
}

int DoubleValuesSource::NumericDocValuesAnonymousInnerClass::advance(
    int target) 
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t DoubleValuesSource::NumericDocValuesAnonymousInnerClass::cost()
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<DoubleValuesSource>
DoubleValuesSource::fromQuery(shared_ptr<Query> query)
{
  return make_shared<QueryDoubleValuesSource>(query);
}

DoubleValuesSource::QueryDoubleValuesSource::QueryDoubleValuesSource(
    shared_ptr<Query> query)
    : query(query)
{
}

bool DoubleValuesSource::QueryDoubleValuesSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<QueryDoubleValuesSource> that =
      any_cast<std::shared_ptr<QueryDoubleValuesSource>>(o);
  return Objects::equals(query, that->query);
}

int DoubleValuesSource::QueryDoubleValuesSource::hashCode()
{
  return Objects::hash(query);
}

shared_ptr<DoubleValues> DoubleValuesSource::QueryDoubleValuesSource::getValues(
    shared_ptr<LeafReaderContext> ctx,
    shared_ptr<DoubleValues> scores) 
{
  throw make_shared<UnsupportedOperationException>(
      L"This DoubleValuesSource must be rewritten");
}

bool DoubleValuesSource::QueryDoubleValuesSource::needsScores()
{
  return false;
}

shared_ptr<DoubleValuesSource>
DoubleValuesSource::QueryDoubleValuesSource::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  return make_shared<WeightDoubleValuesSource>(
      searcher->rewrite(query)->createWeight(searcher, true, 1.0f));
}

wstring DoubleValuesSource::QueryDoubleValuesSource::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"score(" + query->toString() + L")";
}

bool DoubleValuesSource::QueryDoubleValuesSource::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return false;
}

DoubleValuesSource::WeightDoubleValuesSource::WeightDoubleValuesSource(
    shared_ptr<Weight> weight)
    : weight(weight)
{
}

shared_ptr<DoubleValues>
DoubleValuesSource::WeightDoubleValuesSource::getValues(
    shared_ptr<LeafReaderContext> ctx,
    shared_ptr<DoubleValues> scores) 
{
  shared_ptr<Scorer> scorer = weight->scorer(ctx);
  if (scorer == nullptr) {
    return DoubleValues::EMPTY;
  }
  shared_ptr<DocIdSetIterator> it = scorer->begin();
  return make_shared<DoubleValuesAnonymousInnerClass>(shared_from_this(),
                                                      scorer, it);
}

DoubleValuesSource::WeightDoubleValuesSource::DoubleValuesAnonymousInnerClass::
    DoubleValuesAnonymousInnerClass(
        shared_ptr<WeightDoubleValuesSource> outerInstance,
        shared_ptr<org::apache::lucene::search::Scorer> scorer,
        shared_ptr<org::apache::lucene::search::DocIdSetIterator> it)
{
  this->outerInstance = outerInstance;
  this->scorer = scorer;
  this->it = it;
}

double DoubleValuesSource::WeightDoubleValuesSource::
    DoubleValuesAnonymousInnerClass::doubleValue() 
{
  return scorer->score();
}

bool DoubleValuesSource::WeightDoubleValuesSource::
    DoubleValuesAnonymousInnerClass::advanceExact(int doc) 
{
  if (it->docID() > doc) {
    return false;
  }
  return it->docID() == doc || it->advance(doc) == doc;
}

shared_ptr<Explanation> DoubleValuesSource::WeightDoubleValuesSource::explain(
    shared_ptr<LeafReaderContext> ctx, int docId,
    shared_ptr<Explanation> scoreExplanation) 
{
  return weight->explain(ctx, docId);
}

bool DoubleValuesSource::WeightDoubleValuesSource::needsScores()
{
  return false;
}

shared_ptr<DoubleValuesSource>
DoubleValuesSource::WeightDoubleValuesSource::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  return shared_from_this();
}

bool DoubleValuesSource::WeightDoubleValuesSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<WeightDoubleValuesSource> that =
      any_cast<std::shared_ptr<WeightDoubleValuesSource>>(o);
  return Objects::equals(weight, that->weight);
}

int DoubleValuesSource::WeightDoubleValuesSource::hashCode()
{
  return Objects::hash(weight);
}

wstring DoubleValuesSource::WeightDoubleValuesSource::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"score(" + weight->parentQuery->toString() + L")";
}

bool DoubleValuesSource::WeightDoubleValuesSource::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return false;
}
} // namespace org::apache::lucene::search