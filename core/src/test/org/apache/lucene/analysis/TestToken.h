#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/AttributeImpl.h"

#include  "core/src/java/org/apache/lucene/util/AttributeReflector.h"

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
namespace org::apache::lucene::analysis
{

using namespace org::apache::lucene::analysis::tokenattributes;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using Attribute = org::apache::lucene::util::Attribute;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public class TestToken extends
// org.apache.lucene.util.LuceneTestCase
class TestToken : public LuceneTestCase
{

public:
  virtual void testCtor() ;

  /* the CharTermAttributeStuff is tested by TestCharTermAttributeImpl */

  virtual void testClone() ;

  virtual void testCopyTo() ;

public:
  class SenselessAttribute : public Attribute
  {
    GET_CLASS_NAME(SenselessAttribute)
  };

public:
  class SenselessAttributeImpl final : public AttributeImpl,
                                       public SenselessAttribute
  {
    GET_CLASS_NAME(SenselessAttributeImpl)
  public:
    void copyTo(std::shared_ptr<AttributeImpl> target) override;
    void clear() override;
    virtual bool equals(std::any o);
    virtual int hashCode();
    void reflectWith(AttributeReflector reflector) override;

  protected:
    std::shared_ptr<SenselessAttributeImpl> shared_from_this()
    {
      return std::static_pointer_cast<SenselessAttributeImpl>(
          org.apache.lucene.util.AttributeImpl::shared_from_this());
    }
  };

public:
  virtual void testTokenAttributeFactory() ;

  virtual void testAttributeReflection() ;

private:
  class HashMapAnonymousInnerClass
      : public std::unordered_map<std::wstring, std::any>
  {
    GET_CLASS_NAME(HashMapAnonymousInnerClass)
  private:
    std::shared_ptr<TestToken> outerInstance;

  public:
    HashMapAnonymousInnerClass(std::shared_ptr<TestToken> outerInstance);

  protected:
    std::shared_ptr<HashMapAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<HashMapAnonymousInnerClass>(
          java.util.HashMap<std::wstring, Object>::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestToken> shared_from_this()
  {
    return std::static_pointer_cast<TestToken>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/
