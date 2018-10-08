using namespace std;

#include "DocFreqValueSource.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using DoubleDocValues =
    org::apache::lucene::queries::function::docvalues::DoubleDocValues;
using IntDocValues =
    org::apache::lucene::queries::function::docvalues::IntDocValues;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using BytesRef = org::apache::lucene::util::BytesRef;

// C++ TODO: There is no native C++ equivalent to 'toString':
ConstIntDocValues::ConstIntDocValues(int val, shared_ptr<ValueSource> parent)
    : org::apache::lucene::queries::function::docvalues::IntDocValues(parent),
      ival(val), fval(val), dval(val), lval(val), sval(Integer::toString(val)),
      parent(parent)
{
}

float ConstIntDocValues::floatVal(int doc) { return fval; }

int ConstIntDocValues::intVal(int doc) { return ival; }

int64_t ConstIntDocValues::longVal(int doc) { return lval; }

double ConstIntDocValues::doubleVal(int doc) { return dval; }

wstring ConstIntDocValues::strVal(int doc) { return sval; }

wstring ConstIntDocValues::toString(int doc)
{
  return parent->description() + StringHelper::toString(L'=') + sval;
}

// C++ TODO: There is no native C++ equivalent to 'toString':
ConstDoubleDocValues::ConstDoubleDocValues(double val,
                                           shared_ptr<ValueSource> parent)
    : org::apache::lucene::queries::function::docvalues::DoubleDocValues(
          parent),
      ival(static_cast<int>(val)), fval(static_cast<float>(val)), dval(val),
      lval(static_cast<int64_t>(val)), sval(Double::toString(val)),
      parent(parent)
{
}

float ConstDoubleDocValues::floatVal(int doc) { return fval; }

int ConstDoubleDocValues::intVal(int doc) { return ival; }

int64_t ConstDoubleDocValues::longVal(int doc) { return lval; }

double ConstDoubleDocValues::doubleVal(int doc) { return dval; }

wstring ConstDoubleDocValues::strVal(int doc) { return sval; }

wstring ConstDoubleDocValues::toString(int doc)
{
  return parent->description() + StringHelper::toString(L'=') + sval;
}

DocFreqValueSource::DocFreqValueSource(const wstring &field, const wstring &val,
                                       const wstring &indexedField,
                                       shared_ptr<BytesRef> indexedBytes)
    : field(field), indexedField(indexedField), val(val),
      indexedBytes(indexedBytes)
{
}

wstring DocFreqValueSource::name() { return L"docfreq"; }

wstring DocFreqValueSource::description()
{
  return name() + StringHelper::toString(L'(') + field +
         StringHelper::toString(L',') + val + StringHelper::toString(L')');
}

shared_ptr<FunctionValues> DocFreqValueSource::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<IndexSearcher> searcher =
      std::static_pointer_cast<IndexSearcher>(context[L"searcher"]);
  int docfreq = searcher->getIndexReader()->docFreq(
      make_shared<Term>(indexedField, indexedBytes));
  return make_shared<ConstIntDocValues>(docfreq, shared_from_this());
}

void DocFreqValueSource::createWeight(
    unordered_map context,
    shared_ptr<IndexSearcher> searcher) 
{
  context.emplace(L"searcher", searcher);
}

int DocFreqValueSource::hashCode()
{
  return getClass().hashCode() + indexedField.hashCode() * 29 +
         indexedBytes->hashCode();
}

bool DocFreqValueSource::equals(any o)
{
  if (this->getClass() != o.type()) {
    return false;
  }
  shared_ptr<DocFreqValueSource> other =
      any_cast<std::shared_ptr<DocFreqValueSource>>(o);
  return this->indexedField == other->indexedField &&
         this->indexedBytes->equals(other->indexedBytes);
}
} // namespace org::apache::lucene::queries::function::valuesource