using namespace std;

#include "KeepOnlyLastCommitDeletionPolicy.h"

namespace org::apache::lucene::index
{

KeepOnlyLastCommitDeletionPolicy::KeepOnlyLastCommitDeletionPolicy() {}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public void onInit(java.util.List<?
// extends IndexCommit> commits)
void KeepOnlyLastCommitDeletionPolicy::onInit(deque<T1> commits)
{
  // Note that commits.size() should normally be 1:
  onCommit(commits);
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public void onCommit(java.util.List<?
// extends IndexCommit> commits)
void KeepOnlyLastCommitDeletionPolicy::onCommit(deque<T1> commits)
{
  // Note that commits.size() should normally be 2 (if not
  // called by onInit above):
  int size = commits.size();
  for (int i = 0; i < size - 1; i++) {
    commits[i]->delete ();
  }
}
} // namespace org::apache::lucene::index