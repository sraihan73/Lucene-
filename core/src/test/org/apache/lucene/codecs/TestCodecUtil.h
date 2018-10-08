#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"

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

namespace org::apache::lucene::codecs
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/** tests for codecutil methods */
class TestCodecUtil : public LuceneTestCase
{
  GET_CLASS_NAME(TestCodecUtil)

public:
  virtual void testHeaderLength() ;

  virtual void testWriteTooLongHeader() ;

  virtual void testWriteNonAsciiHeader() ;

  virtual void testReadHeaderWrongMagic() ;

  virtual void testChecksumEntireFile() ;

  virtual void testCheckFooterValid() ;

  virtual void testCheckFooterValidAtFooter() ;

  virtual void testCheckFooterValidPastFooter() ;

  virtual void testCheckFooterInvalid() ;

  virtual void testSegmentHeaderLength() ;

  virtual void testWriteTooLongSuffix() ;

  virtual void testWriteVeryLongSuffix() ;

  virtual void testWriteNonAsciiSuffix() ;

  virtual void testReadBogusCRC() ;

  virtual void testWriteBogusCRC() ;

private:
  class IndexOutputAnonymousInnerClass : public IndexOutput
  {
    GET_CLASS_NAME(IndexOutputAnonymousInnerClass)
  private:
    std::shared_ptr<TestCodecUtil> outerInstance;

    std::shared_ptr<IndexOutput> output;
    std::shared_ptr<AtomicLong> fakeChecksum;

  public:
    IndexOutputAnonymousInnerClass(std::shared_ptr<TestCodecUtil> outerInstance,
                                   std::shared_ptr<IndexOutput> output,
                                   std::shared_ptr<AtomicLong> fakeChecksum);

    virtual ~IndexOutputAnonymousInnerClass();

    int64_t getFilePointer() override;

    int64_t getChecksum()  override;

    void writeByte(char b)  override;

    void writeBytes(std::deque<char> &b, int offset,
                    int length)  override;

  protected:
    std::shared_ptr<IndexOutputAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IndexOutputAnonymousInnerClass>(
          org.apache.lucene.store.IndexOutput::shared_from_this());
    }
  };

public:
  virtual void
  testTruncatedFileThrowsCorruptIndexException() ;

protected:
  std::shared_ptr<TestCodecUtil> shared_from_this()
  {
    return std::static_pointer_cast<TestCodecUtil>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/
