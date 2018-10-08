#pragma once
#include "../../BenchmarkTestCase.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <deque>

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
namespace org::apache::lucene::benchmark::byTask::feeds
{

using org::apache::commons::compress::compressors::CompressorStreamFactory;
using BenchmarkTestCase = org::apache::lucene::benchmark::BenchmarkTestCase;

/** Tests the functionality of {@link LineDocSource}. */
class LineDocSourceTest : public BenchmarkTestCase
{
  GET_CLASS_NAME(LineDocSourceTest)

private:
  static const std::shared_ptr<CompressorStreamFactory> csFactory;

  void createBZ2LineFile(std::shared_ptr<Path> file,
                         bool addHeader) ;

  void
  writeDocsToFile(std::shared_ptr<BufferedWriter> writer, bool addHeader,
                  std::shared_ptr<Properties> otherFields) ;

  void createRegularLineFile(std::shared_ptr<Path> file,
                             bool addHeader) ;

  void createRegularLineFileWithMoreFields(
      std::shared_ptr<Path> file,
      std::deque<std::wstring> &extraFields) ;

  void doIndexAndSearchTest(
      std::shared_ptr<Path> file, std::type_info lineParserClass,
      const std::wstring &storedField) ;

  void doIndexAndSearchTestWithRepeats(
      std::shared_ptr<Path> file, std::type_info lineParserClass, int numAdds,
      const std::wstring &storedField) ;

  /* Tests LineDocSource with a bzip2 input stream. */
public:
  virtual void testBZip2() ;

  virtual void testBZip2NoHeaderLine() ;

  virtual void testRegularFile() ;

  virtual void testRegularFileSpecialHeader() ;

  virtual void testRegularFileNoHeaderLine() ;

  virtual void testInvalidFormat() ;

  /** Doc Name is not part of the default header */
  virtual void testWithDocsName() ;

  /** Use fields names that are not defined in Docmaker and so will go to
   * Properties */
  virtual void testWithProperties() ;

protected:
  std::shared_ptr<LineDocSourceTest> shared_from_this()
  {
    return std::static_pointer_cast<LineDocSourceTest>(
        org.apache.lucene.benchmark.BenchmarkTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/
