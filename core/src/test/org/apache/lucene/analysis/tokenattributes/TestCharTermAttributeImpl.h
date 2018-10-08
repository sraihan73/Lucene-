#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
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

using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestCharTermAttributeImpl : public LuceneTestCase
{
  GET_CLASS_NAME(TestCharTermAttributeImpl)

public:
  virtual void testResize();

  virtual void testSetLength();

  virtual void testGrow();

  virtual void testToString() ;

  virtual void testClone() ;

  virtual void testEquals() ;

  virtual void testCopyTo() ;

  virtual void testAttributeReflection() ;

private:
  class HashMapAnonymousInnerClass
      : public std::unordered_map<std::wstring, std::any>
  {
    GET_CLASS_NAME(HashMapAnonymousInnerClass)
  private:
    std::shared_ptr<TestCharTermAttributeImpl> outerInstance;

  public:
    HashMapAnonymousInnerClass(
        std::shared_ptr<TestCharTermAttributeImpl> outerInstance);

  protected:
    std::shared_ptr<HashMapAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<HashMapAnonymousInnerClass>(
          java.util.HashMap<std::wstring, Object>::shared_from_this());
    }
  };

public:
  virtual void testCharSequenceInterface();

  virtual void testAppendableInterface();

  virtual void testAppendableInterfaceWithLongSequences();

private:
  class CharSequenceAnonymousInnerClass : public std::wstring
  {
    GET_CLASS_NAME(CharSequenceAnonymousInnerClass)
  private:
    std::shared_ptr<TestCharTermAttributeImpl> outerInstance;

    std::wstring longTestString;

  public:
    CharSequenceAnonymousInnerClass(
        std::shared_ptr<TestCharTermAttributeImpl> outerInstance,
        const std::wstring &longTestString);

    wchar_t charAt(int i) override;
    int length() override;
    std::shared_ptr<std::wstring> subSequence(int start, int end) override;
    virtual std::wstring toString();

  protected:
    std::shared_ptr<CharSequenceAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<CharSequenceAnonymousInnerClass>(
          std::wstring::shared_from_this());
    }
  };

public:
  virtual void testNonCharSequenceAppend();

  virtual void testExceptions();

  template <typename T>
  static T assertCloneIsEqual(T att);

  template <typename T>
  static T assertCopyIsEqual(T att) ;

  /*

  // test speed of the dynamic instanceof checks in append(std::wstring),
  // to find the best max length for the generic while (start<end) loop:
  public void testAppendPerf() {
    CharTermAttributeImpl t = new CharTermAttributeImpl();
    final int count = 32;
    std::wstring[] csq = new std::wstring[count * 6];
    final StringBuilder sb = new StringBuilder();
    for (int i=0,j=0; i<count; i++) {
      sb.append(i%10);
      final std::wstring testString = sb.toString();
      CharTermAttribute cta = new CharTermAttributeImpl();
      cta.append(testString);
      csq[j++] = cta;
      csq[j++] = testString;
      csq[j++] = new StringBuilder(sb);
      csq[j++] = new StringBuffer(sb);
      csq[j++] = CharBuffer.wrap(testString.toCharArray());
      csq[j++] = new std::wstring() {
        public char charAt(int i) { return testString.charAt(i); }
        public int length() { return testString.length(); }
        public std::wstring subSequence(int start, int end) { return
  testString.subSequence(start, end); } public std::wstring toString() { return
  testString; }
      };
    }

    Random rnd = newRandom();
    long startTime = System.currentTimeMillis();
    for (int i=0; i<100000000; i++) {
      t.setEmpty().append(csq[rnd.nextInt(csq.length)]);
    }
    long endTime = System.currentTimeMillis();
    System.out.println("Time: " + (endTime-startTime)/1000.0 + " s");
  }

  */

protected:
  std::shared_ptr<TestCharTermAttributeImpl> shared_from_this()
  {
    return std::static_pointer_cast<TestCharTermAttributeImpl>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/tokenattributes/
