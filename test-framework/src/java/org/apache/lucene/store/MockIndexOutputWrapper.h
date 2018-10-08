#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/MockDirectoryWrapper.h"

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
namespace org::apache::lucene::store
{

/**
 * Used by MockRAMDirectory to create an output stream that
 * will throw an IOException on fake disk full, track max
 * disk space actually used, and maybe throw random
 * IOExceptions.
 */

class MockIndexOutputWrapper : public IndexOutput
{
  GET_CLASS_NAME(MockIndexOutputWrapper)
private:
  std::shared_ptr<MockDirectoryWrapper> dir;
  const std::shared_ptr<IndexOutput> delegate_;
  bool first = true;

public:
  const std::wstring name;

  std::deque<char> singleByte = std::deque<char>(1);

  /** Construct an empty output buffer. */
  MockIndexOutputWrapper(std::shared_ptr<MockDirectoryWrapper> dir,
                         std::shared_ptr<IndexOutput> delegate_,
                         const std::wstring &name);

private:
  void checkCrashed() ;

  void checkDiskFull(std::deque<char> &b, int offset,
                     std::shared_ptr<DataInput> in_,
                     int64_t len) ;

  bool closed = false;

public:
  virtual ~MockIndexOutputWrapper();

private:
  void ensureOpen();

public:
  void writeByte(char b)  override;

  void writeBytes(std::deque<char> &b, int offset,
                  int len)  override;

  int64_t getFilePointer() override;

  void copyBytes(std::shared_ptr<DataInput> input,
                 int64_t numBytes)  override;

  int64_t getChecksum()  override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<MockIndexOutputWrapper> shared_from_this()
  {
    return std::static_pointer_cast<MockIndexOutputWrapper>(
        IndexOutput::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/store/
