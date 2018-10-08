using namespace std;

#include "NormsProducer.h"
#include "../index/FieldInfo.h"
#include "../index/NumericDocValues.h"

namespace org::apache::lucene::codecs
{
using FieldInfo = org::apache::lucene::index::FieldInfo;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using Accountable = org::apache::lucene::util::Accountable;

NormsProducer::NormsProducer() {}

shared_ptr<NormsProducer> NormsProducer::getMergeInstance() 
{
  return shared_from_this();
}
} // namespace org::apache::lucene::codecs