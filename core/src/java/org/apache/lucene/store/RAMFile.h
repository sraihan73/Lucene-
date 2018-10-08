#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/RAMDirectory.h"

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
 * Represents a file in RAM as a deque of byte[] buffers.
 * @lucene.internal */
class RAMFile : public std::enable_shared_from_this<RAMFile>, public Accountable
{
  GET_CLASS_NAME(RAMFile)
protected:
  const std::deque<std::deque<char>> buffers =
      std::deque<std::deque<char>>();

public:
  int64_t length = 0;
  std::shared_ptr<RAMDirectory> directory;

protected:
  int64_t sizeInBytes = 0;

  // File used as buffer, in no RAMDirectory
public:
  RAMFile();

  RAMFile(std::shared_ptr<RAMDirectory> directory);

  // For non-stream access from thread that might be concurrent with writing
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual int64_t getLength();

protected:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void setLength(int64_t length);

  std::deque<char> addBuffer(int size);

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::deque<char> getBuffer(int index);

  // C++ WARNING: The following method was originally marked 'synchronized':
  int numBuffers();

  /**
   * Expert: allocate a new buffer.
   * Subclasses can allocate differently.
   * @param size size of allocated buffer.
   * @return allocated buffer.
   */
  virtual std::deque<char> newBuffer(int size);

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t ramBytesUsed() override;

  virtual std::wstring toString();

  virtual int hashCode();

  bool equals(std::any obj) override;
};

} // #include  "core/src/java/org/apache/lucene/store/
