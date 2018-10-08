using namespace std;

#include "NoDeletionPolicy.h"

namespace org::apache::lucene::index
{

const shared_ptr<IndexDeletionPolicy> NoDeletionPolicy::INSTANCE =
    make_shared<NoDeletionPolicy>();

NoDeletionPolicy::NoDeletionPolicy()
{
  // keep private to avoid instantiation
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public void onCommit(java.util.List<?
// extends IndexCommit> commits)
void NoDeletionPolicy::onCommit(deque<T1> commits)
{
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public void onInit(java.util.List<?
// extends IndexCommit> commits)
void NoDeletionPolicy::onInit(deque<T1> commits)
{
}
} // namespace org::apache::lucene::index