#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <queue>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::replicator::nrt
{
class CopyJob;
}

namespace org::apache::lucene::replicator::nrt
{
class Node;
}
namespace org::apache::lucene::replicator::nrt
{
class SimpleCopyJob;
}

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

namespace org::apache::lucene::replicator::nrt
{

/** Runs CopyJob(s) in background thread; each ReplicaNode has an instance of
 * this running.  At a given there could be one NRT copy job running, and
 * multiple pre-warm merged segments jobs. */
class Jobs : public Thread
{
  GET_CLASS_NAME(Jobs)

private:
  const std::shared_ptr<PriorityQueue<std::shared_ptr<CopyJob>>> queue =
      std::make_shared<PriorityQueue<std::shared_ptr<CopyJob>>>();

  const std::shared_ptr<Node> node;

public:
  Jobs(std::shared_ptr<Node> node);

private:
  bool finish = false;

  /** Returns null if we are closing, else, returns the top job or waits for one
   * to arrive if the queue is empty. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<SimpleCopyJob> getNextJob();

public:
  void run() override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void launch(std::shared_ptr<CopyJob> job);

  /** Cancels any existing jobs that are copying the same file names as this one
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void
  cancelConflictingJobs(std::shared_ptr<CopyJob> newJob) ;

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual ~Jobs();

protected:
  std::shared_ptr<Jobs> shared_from_this()
  {
    return std::static_pointer_cast<Jobs>(Thread::shared_from_this());
  }
};

} // namespace org::apache::lucene::replicator::nrt
