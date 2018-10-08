using namespace std;

#include "TwoPhaseCommitTool.h"

namespace org::apache::lucene::index
{

TwoPhaseCommitTool::TwoPhaseCommitTool() {}

TwoPhaseCommitTool::PrepareCommitFailException::PrepareCommitFailException(
    runtime_error cause, shared_ptr<TwoPhaseCommit> obj)
    : java::io::IOException(L"prepareCommit() failed on " + obj, cause)
{
}

TwoPhaseCommitTool::CommitFailException::CommitFailException(
    runtime_error cause, shared_ptr<TwoPhaseCommit> obj)
    : java::io::IOException(L"commit() failed on " + obj, cause)
{
}

void TwoPhaseCommitTool::rollback(deque<TwoPhaseCommit> &objects)
{
  for (shared_ptr<TwoPhaseCommit> tpc : objects) {
    // ignore any exception that occurs during rollback - we want to ensure
    // all objects are rolled-back.
    if (tpc != nullptr) {
      try {
        tpc->rollback();
      } catch (const runtime_error &t) {
      }
    }
  }
}

void TwoPhaseCommitTool::execute(deque<TwoPhaseCommit> &objects) throw(
    PrepareCommitFailException, CommitFailException)
{
  shared_ptr<TwoPhaseCommit> tpc = nullptr;
  try {
    // first, all should successfully prepareCommit()
    for (int i = 0; i < objects->length; i++) {
      tpc = objects[i];
      if (tpc != nullptr) {
        tpc->prepareCommit();
      }
    }
  } catch (const runtime_error &t) {
    // first object that fails results in rollback all of them and
    // throwing an exception.
    rollback({objects});
    throw make_shared<PrepareCommitFailException>(t, tpc);
  }

  // If all successfully prepareCommit(), attempt the actual commit()
  try {
    for (int i = 0; i < objects->length; i++) {
      tpc = objects[i];
      if (tpc != nullptr) {
        tpc->commit();
      }
    }
  } catch (const runtime_error &t) {
    // first object that fails results in rollback all of them and
    // throwing an exception.
    rollback({objects});
    throw make_shared<CommitFailException>(t, tpc);
  }
}
} // namespace org::apache::lucene::index