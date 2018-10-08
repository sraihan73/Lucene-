using namespace std;

#include "TermVectorsReader.h"
#include "../index/Fields.h"

namespace org::apache::lucene::codecs
{
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using Fields = org::apache::lucene::index::Fields;
using Accountable = org::apache::lucene::util::Accountable;

TermVectorsReader::TermVectorsReader() {}

shared_ptr<TermVectorsReader>
TermVectorsReader::getMergeInstance() 
{
  return shared_from_this();
}
} // namespace org::apache::lucene::codecs