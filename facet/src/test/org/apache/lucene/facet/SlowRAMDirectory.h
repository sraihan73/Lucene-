#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"

#include  "core/src/java/org/apache/lucene/store/IOContext.h"
#include  "core/src/java/org/apache/lucene/store/IndexInput.h"

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
namespace org::apache::lucene::facet
{

using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;

/**
 * Test utility - slow directory
 */
// TODO: move to test-framework and sometimes use in tests?
class SlowRAMDirectory : public RAMDirectory
{
  GET_CLASS_NAME(SlowRAMDirectory)

private:
  static constexpr int IO_SLEEP_THRESHOLD = 50;

public:
  std::shared_ptr<Random> random;

private:
  int sleepMillis = 0;

public:
  virtual void setSleepMillis(int sleepMillis);

  SlowRAMDirectory(int sleepMillis, std::shared_ptr<Random> random);

  std::shared_ptr<IndexOutput>
  createOutput(const std::wstring &name,
               std::shared_ptr<IOContext> context)  override;

  std::shared_ptr<IndexInput>
  openInput(const std::wstring &name,
            std::shared_ptr<IOContext> context)  override;

  virtual void doSleep(std::shared_ptr<Random> random, int length);

  /** Make a private random. */
  virtual std::shared_ptr<Random> forkRandom();

  /**
   * Delegate class to wrap an IndexInput and delay reading bytes by some
   * specified time.
   */
private:
  class SlowIndexInput : public IndexInput
  {
    GET_CLASS_NAME(SlowIndexInput)
  private:
    std::shared_ptr<SlowRAMDirectory> outerInstance;

    std::shared_ptr<IndexInput> ii;
    int numRead = 0;
    std::shared_ptr<Random> rand;

  public:
    SlowIndexInput(std::shared_ptr<SlowRAMDirectory> outerInstance,
                   std::shared_ptr<IndexInput> ii);

    char readByte()  override;

    void readBytes(std::deque<char> &b, int offset,
                   int len)  override;

    // TODO: is it intentional that clone doesnt wrap?
    std::shared_ptr<IndexInput> clone() override;
    std::shared_ptr<IndexInput>
    slice(const std::wstring &sliceDescription, int64_t offset,
          int64_t length)  override;
    virtual ~SlowIndexInput();
    virtual bool equals(std::any o);
    int64_t getFilePointer() override;
    virtual int hashCode();
    int64_t length() override;
    void seek(int64_t pos)  override;

  protected:
    std::shared_ptr<SlowIndexInput> shared_from_this()
    {
      return std::static_pointer_cast<SlowIndexInput>(
          org.apache.lucene.store.IndexInput::shared_from_this());
    }
  };

  /**
   * Delegate class to wrap an IndexOutput and delay writing bytes by some
   * specified time.
   */
private:
  class SlowIndexOutput : public IndexOutput
  {
    GET_CLASS_NAME(SlowIndexOutput)
  private:
    std::shared_ptr<SlowRAMDirectory> outerInstance;

    std::shared_ptr<IndexOutput> io;
    int numWrote = 0;
    const std::shared_ptr<Random> rand;

  public:
    SlowIndexOutput(std::shared_ptr<SlowRAMDirectory> outerInstance,
                    std::shared_ptr<IndexOutput> io);

    void writeByte(char b)  override;

    void writeBytes(std::deque<char> &b, int offset,
                    int length)  override;

    virtual ~SlowIndexOutput();
    int64_t getFilePointer() override;
    int64_t getChecksum()  override;

  protected:
    std::shared_ptr<SlowIndexOutput> shared_from_this()
    {
      return std::static_pointer_cast<SlowIndexOutput>(
          org.apache.lucene.store.IndexOutput::shared_from_this());
    }
  };

protected:
  std::shared_ptr<SlowRAMDirectory> shared_from_this()
  {
    return std::static_pointer_cast<SlowRAMDirectory>(
        org.apache.lucene.store.RAMDirectory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/
