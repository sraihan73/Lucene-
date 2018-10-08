using namespace std;

#include "DocValuesProducer.h"
#include "../index/BinaryDocValues.h"
#include "../index/FieldInfo.h"
#include "../index/NumericDocValues.h"
#include "../index/SortedDocValues.h"
#include "../index/SortedNumericDocValues.h"
#include "../index/SortedSetDocValues.h"

namespace org::apache::lucene::codecs
{
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using Accountable = org::apache::lucene::util::Accountable;

DocValuesProducer::DocValuesProducer() {}

shared_ptr<DocValuesProducer>
DocValuesProducer::getMergeInstance() 
{
  return shared_from_this();
}
} // namespace org::apache::lucene::codecs