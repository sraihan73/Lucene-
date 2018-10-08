using namespace std;

#include "StoredFieldsReader.h"
#include "../index/StoredFieldVisitor.h"

namespace org::apache::lucene::codecs
{
using StoredFieldVisitor = org::apache::lucene::index::StoredFieldVisitor;
using Accountable = org::apache::lucene::util::Accountable;

StoredFieldsReader::StoredFieldsReader() {}

shared_ptr<StoredFieldsReader>
StoredFieldsReader::getMergeInstance() 
{
  return shared_from_this();
}
} // namespace org::apache::lucene::codecs