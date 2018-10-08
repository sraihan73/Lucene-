using namespace std;

#include "Builder.h"

namespace org::apache::lucene::util::fst
{
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using INPUT_TYPE = org::apache::lucene::util::fst::FST::INPUT_TYPE;

bool Builder<T>::CompiledNode::isCompiled() { return true; }
} // namespace org::apache::lucene::util::fst