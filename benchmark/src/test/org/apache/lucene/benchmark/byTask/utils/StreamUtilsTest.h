#pragma once
#include "../../BenchmarkTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

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
namespace org::apache::lucene::benchmark::byTask::utils
{

using BenchmarkTestCase = org::apache::lucene::benchmark::BenchmarkTestCase;

class StreamUtilsTest : public BenchmarkTestCase
{
  GET_CLASS_NAME(StreamUtilsTest)
private:
  static const std::wstring TEXT;
  std::shared_ptr<Path> testDir;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGetInputStreamPlainText() throws
  // Exception
  virtual void testGetInputStreamPlainText() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGetInputStreamGzip() throws Exception
  virtual void testGetInputStreamGzip() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGetInputStreamBzip2() throws Exception
  virtual void testGetInputStreamBzip2() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGetOutputStreamBzip2() throws
  // Exception
  virtual void testGetOutputStreamBzip2() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGetOutputStreamGzip() throws Exception
  virtual void testGetOutputStreamGzip() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGetOutputStreamPlain() throws
  // Exception
  virtual void testGetOutputStreamPlain() ;

private:
  std::shared_ptr<Path>
  rawTextFile(const std::wstring &ext) ;

  std::shared_ptr<Path>
  rawGzipFile(const std::wstring &ext) ;

  std::shared_ptr<Path>
  rawBzip2File(const std::wstring &ext) ;

  std::shared_ptr<Path>
  autoOutFile(const std::wstring &ext) ;

  void writeText(std::shared_ptr<OutputStream> os) ;

  void assertReadText(std::shared_ptr<Path> f) ;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @Before public void setUp() throws Exception
  void setUp()  override;

protected:
  std::shared_ptr<StreamUtilsTest> shared_from_this()
  {
    return std::static_pointer_cast<StreamUtilsTest>(
        org.apache.lucene.benchmark.BenchmarkTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::utils
