using namespace std;

#include "FieldsProducer.h"

namespace org::apache::lucene::codecs
{
using Fields = org::apache::lucene::index::Fields;
using Accountable = org::apache::lucene::util::Accountable;

FieldsProducer::FieldsProducer() {}

shared_ptr<FieldsProducer> FieldsProducer::getMergeInstance() 
{
  return shared_from_this();
}
} // namespace org::apache::lucene::codecs