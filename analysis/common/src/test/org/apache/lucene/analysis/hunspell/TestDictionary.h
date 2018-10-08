#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/LuceneTestCase.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

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
namespace org::apache::lucene::analysis::hunspell
{

using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestDictionary : public LuceneTestCase
{
  GET_CLASS_NAME(TestDictionary)

public:
  virtual void testSimpleDictionary() ;

  virtual void testCompressedDictionary() ;

  virtual void testCompressedBeforeSetDictionary() ;

  virtual void testCompressedEmptyAliasDictionary() ;

  // malformed rule causes ParseException
  virtual void testInvalidData() ;

  // malformed flags causes ParseException
  virtual void testInvalidFlags() ;

private:
  class CloseCheckInputStream : public FilterInputStream
  {
    GET_CLASS_NAME(CloseCheckInputStream)
  private:
    bool closed = false;

  public:
    CloseCheckInputStream(std::shared_ptr<InputStream> delegate_);

    virtual ~CloseCheckInputStream();

    virtual bool isClosed();

  protected:
    std::shared_ptr<CloseCheckInputStream> shared_from_this()
    {
      return std::static_pointer_cast<CloseCheckInputStream>(
          java.io.FilterInputStream::shared_from_this());
    }
  };

public:
  virtual void testResourceCleanup() ;

  virtual void testReplacements() ;

  virtual void testSetWithCrazyWhitespaceAndBOMs() ;

  virtual void testFlagWithCrazyWhitespace() ;

private:
  std::shared_ptr<Directory> getDirectory();

protected:
  std::shared_ptr<TestDictionary> shared_from_this()
  {
    return std::static_pointer_cast<TestDictionary>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/hunspell/
