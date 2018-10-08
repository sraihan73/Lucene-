#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class RAMFile;
}

namespace org::apache::lucene::store
{
class IndexInput;
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

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.store.RAMOutputStream.BUFFER_SIZE;

/** A memory-resident {@link IndexInput} implementation.
 *
 *  @lucene.internal */
class RAMInputStream : public IndexInput, public Cloneable
{
  GET_CLASS_NAME(RAMInputStream)

private:
  const std::shared_ptr<RAMFile> file;
  // C++ NOTE: Fields cannot have the same name as methods:
  const int64_t length_;

  std::deque<char> currentBuffer;
  int currentBufferIndex = 0;

  int bufferPosition = 0;
  int bufferLength = 0;

public:
  RAMInputStream(const std::wstring &name,
                 std::shared_ptr<RAMFile> f) ;

  RAMInputStream(const std::wstring &name, std::shared_ptr<RAMFile> f,
                 int64_t length) ;

  virtual ~RAMInputStream();

  int64_t length() override;

  char readByte()  override;

  void readBytes(std::deque<char> &b, int offset,
                 int len)  override;

  int64_t getFilePointer() override;

  void seek(int64_t pos)  override;

private:
  void nextBuffer() ;

  void setCurrentBuffer() ;

public:
  std::shared_ptr<IndexInput>
  slice(const std::wstring &sliceDescription, int64_t const offset,
        int64_t const sliceLength)  override;

private:
  class RAMInputStreamAnonymousInnerClass;

protected:
  std::shared_ptr<RAMInputStream> shared_from_this()
  {
    return std::static_pointer_cast<RAMInputStream>(
        IndexInput::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
class RAMInputStream::RAMInputStreamAnonymousInnerClass : public RAMInputStream
{
  GET_CLASS_NAME(RAMInputStream::RAMInputStreamAnonymousInnerClass)
private:
  std::shared_ptr<RAMInputStream> outerInstance;

  std::wstring sliceDescription;
  int64_t offset = 0;
  int64_t sliceLength = 0;

public:
  RAMInputStreamAnonymousInnerClass(
      std::shared_ptr<RAMInputStream> outerInstance,
      const std::wstring &getFullSliceDescription,
      std::shared_ptr<org::apache::lucene::store::RAMFile> file,
      int64_t offset, const std::wstring &sliceDescription, int64_t offset,
      int64_t sliceLength);

  void seek(int64_t pos)  override;

  int64_t getFilePointer() override;

  int64_t length() override;

  std::shared_ptr<IndexInput> slice(const std::wstring &sliceDescription,
                                    int64_t ofs,
                                    int64_t len)  override;

protected:
  std::shared_ptr<RAMInputStreamAnonymousInnerClass> shared_from_this()
  {
    return std::static_pointer_cast<RAMInputStreamAnonymousInnerClass>(
        RAMInputStream::shared_from_this());
  }
};
