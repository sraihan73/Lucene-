using namespace std;

#include "EmptyDocValuesProducer.h"

namespace org::apache::lucene::index
{
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;

EmptyDocValuesProducer::EmptyDocValuesProducer() {}

shared_ptr<NumericDocValues> EmptyDocValuesProducer::getNumeric(
    shared_ptr<FieldInfo> field) 
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<BinaryDocValues> EmptyDocValuesProducer::getBinary(
    shared_ptr<FieldInfo> field) 
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<SortedDocValues> EmptyDocValuesProducer::getSorted(
    shared_ptr<FieldInfo> field) 
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<SortedNumericDocValues> EmptyDocValuesProducer::getSortedNumeric(
    shared_ptr<FieldInfo> field) 
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<SortedSetDocValues> EmptyDocValuesProducer::getSortedSet(
    shared_ptr<FieldInfo> field) 
{
  throw make_shared<UnsupportedOperationException>();
}

void EmptyDocValuesProducer::checkIntegrity()
{
  throw make_shared<UnsupportedOperationException>();
}

EmptyDocValuesProducer::~EmptyDocValuesProducer()
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t EmptyDocValuesProducer::ramBytesUsed()
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::index