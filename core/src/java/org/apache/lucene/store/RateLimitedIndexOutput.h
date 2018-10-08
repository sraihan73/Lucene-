#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IndexOutput;
}

namespace org::apache::lucene::store
{
class RateLimiter;
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
namespace org::apache::lucene::store
{

/**
 * A {@link RateLimiter rate limiting} {@link IndexOutput}
 *
 * @lucene.internal
 */

class RateLimitedIndexOutput final : public IndexOutput
{
  GET_CLASS_NAME(RateLimitedIndexOutput)

private:
  const std::shared_ptr<IndexOutput> delegate_;
  const std::shared_ptr<RateLimiter> rateLimiter;

  /** How many bytes we've written since we last called rateLimiter.pause. */
  int64_t bytesSinceLastPause = 0;

  /** Cached here not not always have to call
   * RateLimiter#getMinPauseCheckBytes() which does volatile read. */
  int64_t currentMinPauseCheckBytes = 0;

public:
  RateLimitedIndexOutput(std::shared_ptr<RateLimiter> rateLimiter,
                         std::shared_ptr<IndexOutput> delegate_);

  virtual ~RateLimitedIndexOutput();

  int64_t getFilePointer() override;

  int64_t getChecksum()  override;

  void writeByte(char b)  override;

  void writeBytes(std::deque<char> &b, int offset,
                  int length)  override;

private:
  void checkRate() ;

protected:
  std::shared_ptr<RateLimitedIndexOutput> shared_from_this()
  {
    return std::static_pointer_cast<RateLimitedIndexOutput>(
        IndexOutput::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
