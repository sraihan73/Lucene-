using namespace std;

#include "Fields.h"

namespace org::apache::lucene::index
{
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;

Fields::Fields() {}

std::deque<std::shared_ptr<Fields>> const Fields::EMPTY_ARRAY =
    std::deque<std::shared_ptr<Fields>>(0);
} // namespace org::apache::lucene::index