#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"

#include  "core/src/java/org/apache/lucene/store/DataInput.h"

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

using DataInput = org::apache::lucene::store::DataInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;

/**
 * Intentionally slow IndexOutput for testing.
 */
class ThrottledIndexOutput : public IndexOutput
{
  GET_CLASS_NAME(ThrottledIndexOutput)
public:
  static constexpr int DEFAULT_MIN_WRITTEN_BYTES = 1024;

private:
  const int bytesPerSecond;
  std::shared_ptr<IndexOutput> delegate_;
  int64_t flushDelayMillis = 0;
  int64_t closeDelayMillis = 0;
  int64_t seekDelayMillis = 0;
  int64_t pendingBytes = 0;
  int64_t minBytesWritten = 0;
  int64_t timeElapsed = 0;
  std::deque<char> const bytes = std::deque<char>(1);

public:
  virtual std::shared_ptr<ThrottledIndexOutput>
  newFromDelegate(std::shared_ptr<IndexOutput> output);

  ThrottledIndexOutput(int bytesPerSecond, int64_t delayInMillis,
                       std::shared_ptr<IndexOutput> delegate_);

  ThrottledIndexOutput(int bytesPerSecond, int64_t delays,
                       int minBytesWritten,
                       std::shared_ptr<IndexOutput> delegate_);

  static int mBitsToBytes(int mbits);

  ThrottledIndexOutput(int bytesPerSecond, int64_t flushDelayMillis,
                       int64_t closeDelayMillis, int64_t seekDelayMillis,
                       int64_t minBytesWritten,
                       std::shared_ptr<IndexOutput> delegate_);

  virtual ~ThrottledIndexOutput();

  int64_t getFilePointer() override;

  void writeByte(char b)  override;

  void writeBytes(std::deque<char> &b, int offset,
                  int length)  override;

protected:
  virtual int64_t getDelay(bool closing);

private:
  static void sleep(int64_t ms);

public:
  void copyBytes(std::shared_ptr<DataInput> input,
                 int64_t numBytes)  override;

  int64_t getChecksum()  override;

protected:
  std::shared_ptr<ThrottledIndexOutput> shared_from_this()
  {
    return std::static_pointer_cast<ThrottledIndexOutput>(
        org.apache.lucene.store.IndexOutput::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
