#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
}
namespace org::apache::lucene::store
{
class Directory;
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
namespace org::apache::lucene::index
{

using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestIndexWriterForceMerge : public LuceneTestCase
{
  GET_CLASS_NAME(TestIndexWriterForceMerge)
public:
  virtual void testPartialMerge() ;

  virtual void testMaxNumSegments2() ;

  /**
   * Make sure forceMerge doesn't use any more than 1X
   * starting index size as its temporary free space
   * required.
   */
  virtual void testForceMergeTempSpaceUsage() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterForceMerge> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterForceMerge> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

  // print out listing of files and sizes, but recurse into CFS to debug nested
  // files there.
private:
  std::wstring listFiles(std::shared_ptr<Directory> dir) ;

  // Test calling forceMerge(1, false) whereby forceMerge is kicked
  // off but we don't wait for it to finish (but
  // writer.close()) does wait
public:
  virtual void testBackgroundForceMerge() ;

protected:
  std::shared_ptr<TestIndexWriterForceMerge> shared_from_this()
  {
    return std::static_pointer_cast<TestIndexWriterForceMerge>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
