using namespace std;

#include "AbstractPagedMutable.h"

namespace org::apache::lucene::util::packed
{
//    import static org.apache.lucene.util.packed.PackedInts.checkBlockSize;
//    import static org.apache.lucene.util.packed.PackedInts.numBlocks;
using Accountable = org::apache::lucene::util::Accountable;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using LongValues = org::apache::lucene::util::LongValues;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
} // namespace org::apache::lucene::util::packed