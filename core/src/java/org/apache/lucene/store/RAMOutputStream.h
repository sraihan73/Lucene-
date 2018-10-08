#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/RAMFile.h"

#include  "core/src/java/org/apache/lucene/store/DataOutput.h"
#include  "core/src/java/org/apache/lucene/util/Accountable.h"

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

using Accountable = org::apache::lucene::util::Accountable;

/**
 * A memory-resident {@link IndexOutput} implementation.
 *
 * @lucene.internal
 */
class RAMOutputStream : public IndexOutput, public Accountable
{
  GET_CLASS_NAME(RAMOutputStream)
public:
  static constexpr int BUFFER_SIZE = 1024;

private:
  const std::shared_ptr<RAMFile> file;

  std::deque<char> currentBuffer;
  int currentBufferIndex = 0;

  int bufferPosition = 0;
  int64_t bufferStart = 0;
  int bufferLength = 0;

  const std::shared_ptr<Checksum> crc;

  /** Construct an empty output buffer. */
public:
  RAMOutputStream();

  /** Creates this, with no name. */
  RAMOutputStream(std::shared_ptr<RAMFile> f, bool checksum);

  /** Creates this, with specified name. */
  RAMOutputStream(const std::wstring &name, std::shared_ptr<RAMFile> f,
                  bool checksum);

  /** Copy the current contents of this buffer to the provided {@link
   * DataOutput}. */
  virtual void writeTo(std::shared_ptr<DataOutput> out) ;

  /** Copy the current contents of this buffer to output
   *  byte array */
  virtual void writeTo(std::deque<char> &bytes, int offset) ;

  /** Resets this to an empty file. */
  virtual void reset();

  virtual ~RAMOutputStream();

  void writeByte(char b)  override;

  void writeBytes(std::deque<char> &b, int offset,
                  int len)  override;

private:
  void switchCurrentBuffer();

  void setFileLength();

  /** Forces any buffered output to be written. */
protected:
  virtual void flush() ;

public:
  int64_t getFilePointer() override;

  /** Returns byte usage of all buffers. */
  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  int64_t getChecksum()  override;

protected:
  std::shared_ptr<RAMOutputStream> shared_from_this()
  {
    return std::static_pointer_cast<RAMOutputStream>(
        IndexOutput::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/store/
