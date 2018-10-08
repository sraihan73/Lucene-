using namespace std;

#include "BinaryDocValuesField.h"

namespace org::apache::lucene::document
{
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using BytesRef = org::apache::lucene::util::BytesRef;
const shared_ptr<FieldType> BinaryDocValuesField::TYPE =
    make_shared<FieldType>();

BinaryDocValuesField::StaticConstructor::StaticConstructor()
{
  TYPE->setDocValuesType(DocValuesType::BINARY);
  TYPE->freeze();
}

BinaryDocValuesField::StaticConstructor BinaryDocValuesField::staticConstructor;

BinaryDocValuesField::BinaryDocValuesField(const wstring &name,
                                           shared_ptr<BytesRef> value)
    : Field(name, TYPE)
{
  fieldsData = value;
}
} // namespace org::apache::lucene::document