using namespace std;

#include "Jobs.h"

namespace org::apache::lucene::replicator::nrt
{
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;

Jobs::Jobs(shared_ptr<Node> node) : node(node) {}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<SimpleCopyJob> Jobs::getNextJob()
{
  while (true) {
    if (finish) {
      return nullptr;
    } else if (queue->isEmpty()) {
      try {
        wait();
      } catch (const InterruptedException &ie) {
        throw runtime_error(ie);
      }
    } else {
      return std::static_pointer_cast<SimpleCopyJob>(queue->poll());
    }
  }
}

void Jobs::run()
{
  while (true) {
    shared_ptr<SimpleCopyJob> topJob = getNextJob();
    if (topJob == nullptr) {
      assert(finish);
      break;
    }

    this->setName(L"jobs o" + to_wstring(topJob->ord));

    assert(topJob != nullptr);

    bool result;
    try {
      result = topJob->visit();
    } catch (const runtime_error &t) {
      if ((std::dynamic_pointer_cast<AlreadyClosedException>(t) != nullptr) ==
          false) {
        node->message(L"exception during job.visit job=" + topJob +
                      L"; now cancel");
        t.printStackTrace(System::out);
      } else {
        node->message(L"AlreadyClosedException during job.visit job=" + topJob +
                      L"; now cancel");
      }
      try {
        topJob->cancel(L"unexpected exception in visit", t);
      } catch (const runtime_error &t2) {
        node->message(L"ignore exception calling cancel: " + t2);
        t2.printStackTrace(System::out);
      }
      try {
        topJob->onceDone->run(topJob);
      } catch (const runtime_error &t2) {
        node->message(L"ignore exception calling OnceDone: " + t2);
        t2.printStackTrace(System::out);
      }
      continue;
    }

    if (result == false) {
      // Job isn't done yet; put it back:
      // C++ TODO: Multithread locking on 'this' is not converted to native C++:
      synchronized(shared_from_this()) { queue->offer(topJob); }
    } else {
      // Job finished, now notify caller:
      try {
        topJob->onceDone->run(topJob);
      } catch (const runtime_error &t) {
        node->message(L"ignore exception calling OnceDone: " + t);
        t.printStackTrace(System::out);
      }
    }
  }

  node->message(L"top: jobs now exit run thread");

  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    // Gracefully cancel any jobs we didn't finish:
    while (queue->isEmpty() == false) {
      shared_ptr<SimpleCopyJob> job =
          std::static_pointer_cast<SimpleCopyJob>(queue->poll());
      node->message(L"top: Jobs: now cancel job=" + job);
      try {
        job->cancel(L"jobs closing", nullptr);
      } catch (const runtime_error &t) {
        node->message(L"ignore exception calling cancel");
        t.printStackTrace(System::out);
      }
      try {
        job->onceDone->run(job);
      } catch (const runtime_error &t) {
        node->message(L"ignore exception calling OnceDone");
        t.printStackTrace(System::out);
      }
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void Jobs::launch(shared_ptr<CopyJob> job)
{
  if (finish == false) {
    queue->offer(job);
    notify();
  } else {
    throw make_shared<AlreadyClosedException>(L"closed");
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void Jobs::cancelConflictingJobs(shared_ptr<CopyJob> newJob) 
{
  for (auto job : queue) {
    if (job->conflicts(newJob)) {
      node->message(L"top: now cancel existing conflicting job=" + job +
                    L" due to newJob=" + newJob);
      job->cancel(L"conflicts with new job", nullptr);
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
Jobs::~Jobs()
{
  finish = true;
  notify();
  try {
    join();
  } catch (const InterruptedException &ie) {
    throw runtime_error(ie);
  }
}
} // namespace org::apache::lucene::replicator::nrt