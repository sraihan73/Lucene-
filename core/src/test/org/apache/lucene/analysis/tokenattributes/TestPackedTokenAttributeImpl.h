#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
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
namespace org::apache::lucene::analysis::tokenattributes
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestPackedTokenAttributeImpl : public LuceneTestCase
{
  GET_CLASS_NAME(TestPackedTokenAttributeImpl)

  /* the CharTermAttributeStuff is tested by TestCharTermAttributeImpl */

public:
  virtual void testClone() ;

  virtual void testCopyTo() ;

  virtual void testPackedTokenAttributeFactory() ;

  virtual void testAttributeReflection() ;

private:
  class HashMapAnonymousInnerClass
      : public std::unordered_map<std::wstring, std::any>
  {
    GET_CLASS_NAME(HashMapAnonymousInnerClass)
  private:
    std::shared_ptr<TestPackedTokenAttributeImpl> outerInstance;

  public:
    HashMapAnonymousInnerClass(
        std::shared_ptr<TestPackedTokenAttributeImpl> outerInstance);

  protected:
    std::shared_ptr<HashMapAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<HashMapAnonymousInnerClass>(
          java.util.HashMap<std::wstring, Object>::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestPackedTokenAttributeImpl> shared_from_this()
  {
    return std::static_pointer_cast<TestPackedTokenAttributeImpl>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/tokenattributes/
