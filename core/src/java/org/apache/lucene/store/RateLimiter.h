#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>

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
namespace org::apache::lucene::store
{

/** Abstract base class to rate limit IO.  Typically implementations are
 *  shared across multiple IndexInputs or IndexOutputs (for example
 *  those involved all merging).  Those IndexInputs and
 *  IndexOutputs would call {@link #pause} whenever the have read
 *  or written more than {@link #getMinPauseCheckBytes} bytes. */
class RateLimiter : public std::enable_shared_from_this<RateLimiter>
{
  GET_CLASS_NAME(RateLimiter)

  /**
   * Sets an updated MB per second rate limit.
   */
public:
  virtual void setMBPerSec(double mbPerSec) = 0;

  /**
   * The current MB per second rate limit.
   */
  virtual double getMBPerSec() = 0;

  /** Pauses, if necessary, to keep the instantaneous IO
   *  rate at or below the target.
   *  <p>
   *  Note: the implementation is thread-safe
   *  </p>
   *  @return the pause time in nano seconds
   * */
  virtual int64_t pause(int64_t bytes) = 0;

  /** How many bytes caller should add up itself before invoking {@link #pause}.
   */
  virtual int64_t getMinPauseCheckBytes() = 0;

  /**
   * Simple class to rate limit IO.
   */
public:
  class SimpleRateLimiter;
};

} // namespace org::apache::lucene::store
class RateLimiter::SimpleRateLimiter : public RateLimiter
{
  GET_CLASS_NAME(RateLimiter::SimpleRateLimiter)

private:
  static constexpr int MIN_PAUSE_CHECK_MSEC = 5;

  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile double mbPerSec;
  double mbPerSec = 0;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile long minPauseCheckBytes;
  int64_t minPauseCheckBytes = 0;
  int64_t lastNS = 0;

  // TODO: we could also allow eg a sub class to dynamically
  // determine the allowed rate, eg if an app wants to
  // change the allowed rate over time or something

  /** mbPerSec is the MB/sec max IO rate */
public:
  SimpleRateLimiter(double mbPerSec);

  /**
   * Sets an updated mb per second rate limit.
   */
  void setMBPerSec(double mbPerSec) override;

  int64_t getMinPauseCheckBytes() override;

  /**
   * The current mb per second rate limit.
   */
  double getMBPerSec() override;

  /** Pauses, if necessary, to keep the instantaneous IO
   *  rate at or below the target.  Be sure to only call
   *  this method when bytes &gt; {@link #getMinPauseCheckBytes},
   *  otherwise it will pause way too long!
   *
   *  @return the pause time in nano seconds */
  int64_t pause(int64_t bytes) override;

protected:
  std::shared_ptr<SimpleRateLimiter> shared_from_this()
  {
    return std::static_pointer_cast<SimpleRateLimiter>(
        RateLimiter::shared_from_this());
  }
};
