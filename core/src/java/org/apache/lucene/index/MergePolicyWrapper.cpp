using namespace std;

#include "MergePolicyWrapper.h"

namespace org::apache::lucene::index
{

MergePolicyWrapper::MergePolicyWrapper(shared_ptr<MergePolicy> in_)
    : FilterMergePolicy(in_)
{
}
} // namespace org::apache::lucene::index