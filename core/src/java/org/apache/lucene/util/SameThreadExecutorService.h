#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

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
namespace org::apache::lucene::util
{

/** An {@code ExecutorService} that executes tasks immediately in the calling
 * thread during submit.
 *
 *  @lucene.internal */
class SameThreadExecutorService final : public AbstractExecutorService
{
  GET_CLASS_NAME(SameThreadExecutorService)
private:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile bool shutdown;
  // C++ NOTE: Fields cannot have the same name as methods:
  bool shutdown_ = false;

public:
  void execute(Runnable command) override;

  std::deque<Runnable> shutdownNow() override;

  void shutdown() override;

  bool isTerminated() override;

  bool isShutdown() override;

  bool awaitTermination(
      int64_t timeout,
      std::shared_ptr<TimeUnit> unit)  override;

private:
  void checkShutdown();

protected:
  std::shared_ptr<SameThreadExecutorService> shared_from_this()
  {
    return std::static_pointer_cast<SameThreadExecutorService>(
        java.util.concurrent.AbstractExecutorService::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
