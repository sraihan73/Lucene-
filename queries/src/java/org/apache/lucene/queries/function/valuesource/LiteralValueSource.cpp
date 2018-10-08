using namespace std;

#include "LiteralValueSource.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using StrDocValues =
    org::apache::lucene::queries::function::docvalues::StrDocValues;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

LiteralValueSource::LiteralValueSource(const wstring &string)
    : string(string), bytesRef(make_shared<BytesRef>(string))
{
}

wstring LiteralValueSource::getValue() { return string; }

shared_ptr<FunctionValues> LiteralValueSource::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{

  return make_shared<StrDocValuesAnonymousInnerClass>(shared_from_this());
}

LiteralValueSource::StrDocValuesAnonymousInnerClass::
    StrDocValuesAnonymousInnerClass(
        shared_ptr<LiteralValueSource> outerInstance)
    : org::apache::lucene::queries::function::docvalues::StrDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
}

wstring LiteralValueSource::StrDocValuesAnonymousInnerClass::strVal(int doc)
{
  return outerInstance->string;
}

bool LiteralValueSource::StrDocValuesAnonymousInnerClass::bytesVal(
    int doc, shared_ptr<BytesRefBuilder> target)
{
  target->copyBytes(outerInstance->bytesRef);
  return true;
}

wstring LiteralValueSource::StrDocValuesAnonymousInnerClass::toString(int doc)
{
  return outerInstance->string;
}

wstring LiteralValueSource::description()
{
  return L"literal(" + string + L")";
}

bool LiteralValueSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (!(std::dynamic_pointer_cast<LiteralValueSource>(o) != nullptr)) {
    return false;
  }

  shared_ptr<LiteralValueSource> that =
      any_cast<std::shared_ptr<LiteralValueSource>>(o);

  return string == that->string;
}

int LiteralValueSource::hashCode() { return hash + string.hashCode(); }
} // namespace org::apache::lucene::queries::function::valuesource