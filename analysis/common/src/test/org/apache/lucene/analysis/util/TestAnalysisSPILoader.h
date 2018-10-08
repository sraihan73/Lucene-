#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/LuceneTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

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
namespace org::apache::lucene::analysis::util
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestAnalysisSPILoader : public LuceneTestCase
{
  GET_CLASS_NAME(TestAnalysisSPILoader)

private:
  std::unordered_map<std::wstring, std::wstring> versionArgOnly();

private:
  class HashMapAnonymousInnerClass
      : public std::unordered_map<std::wstring, std::wstring>
  {
    GET_CLASS_NAME(HashMapAnonymousInnerClass)
  private:
    std::shared_ptr<TestAnalysisSPILoader> outerInstance;

  public:
    HashMapAnonymousInnerClass(
        std::shared_ptr<TestAnalysisSPILoader> outerInstance);

  protected:
    std::shared_ptr<HashMapAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<HashMapAnonymousInnerClass>(
          java.util.HashMap<std::wstring, std::wstring>::shared_from_this());
    }
  };

public:
  virtual void testLookupTokenizer();

  virtual void testBogusLookupTokenizer();

  virtual void testLookupTokenizerClass();

  virtual void testBogusLookupTokenizerClass();

  virtual void testAvailableTokenizers();

  virtual void testLookupTokenFilter();

  virtual void testBogusLookupTokenFilter();

  virtual void testLookupTokenFilterClass();

  virtual void testBogusLookupTokenFilterClass();

  virtual void testAvailableTokenFilters();

  virtual void testLookupCharFilter();

  virtual void testBogusLookupCharFilter();

  virtual void testLookupCharFilterClass();

  virtual void testBogusLookupCharFilterClass();

  virtual void testAvailableCharFilters();

protected:
  std::shared_ptr<TestAnalysisSPILoader> shared_from_this()
  {
    return std::static_pointer_cast<TestAnalysisSPILoader>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/util/
