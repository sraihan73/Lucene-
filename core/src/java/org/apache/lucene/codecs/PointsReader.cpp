using namespace std;

#include "PointsReader.h"
#include "../index/PointValues.h"

namespace org::apache::lucene::codecs
{
using PointValues = org::apache::lucene::index::PointValues;
using Accountable = org::apache::lucene::util::Accountable;

PointsReader::PointsReader() {}

shared_ptr<PointsReader> PointsReader::getMergeInstance() 
{
  return shared_from_this();
}
} // namespace org::apache::lucene::codecs