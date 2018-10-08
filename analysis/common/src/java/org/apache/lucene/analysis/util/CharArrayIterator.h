#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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
namespace org::apache::lucene::analysis::util
{

/**
 * A CharacterIterator used internally for use with {@link BreakIterator}
 * @lucene.internal
 */
class CharArrayIterator
    : public std::enable_shared_from_this<CharArrayIterator>,
      public CharacterIterator
{
  GET_CLASS_NAME(CharArrayIterator)
private:
  std::deque<wchar_t> array_;
  int start = 0;
  int index = 0;
  int length = 0;
  int limit = 0;

public:
  virtual std::deque<wchar_t> getText();

  virtual int getStart();

  virtual int getLength();

  /**
   * Set a new region of text to be examined by this iterator
   *
   * @param array text buffer to examine
   * @param start offset into buffer
   * @param length maximum length to examine
   */
  virtual void setText(std::deque<wchar_t> &array_, int start, int length);

  wchar_t current() override;

protected:
  virtual wchar_t jreBugWorkaround(wchar_t ch) = 0;

public:
  wchar_t first() override;

  int getBeginIndex() override;

  int getEndIndex() override;

  int getIndex() override;

  wchar_t last() override;

  wchar_t next() override;

  wchar_t previous() override;

  wchar_t setIndex(int position) override;

  std::shared_ptr<CharArrayIterator> clone() override;

  /**
   * Create a new CharArrayIterator that works around JRE bugs
   * in a manner suitable for {@link BreakIterator#getSentenceInstance()}
   */
  static std::shared_ptr<CharArrayIterator> newSentenceInstance();

private:
  class CharArrayIteratorAnonymousInnerClass;

private:
  class CharArrayIteratorAnonymousInnerClass2;

  /**
   * Create a new CharArrayIterator that works around JRE bugs
   * in a manner suitable for {@link BreakIterator#getWordInstance()}
   */
public:
  static std::shared_ptr<CharArrayIterator> newWordInstance();

private:
  class CharArrayIteratorAnonymousInnerClass3;

private:
  class CharArrayIteratorAnonymousInnerClass4;

  /**
   * True if this JRE has a buggy BreakIterator implementation
   */
public:
  static const bool HAS_BUGGY_BREAKITERATORS = false;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static CharArrayIterator::StaticConstructor staticConstructor;
};

} // namespace org::apache::lucene::analysis::util
class CharArrayIterator::CharArrayIteratorAnonymousInnerClass
    : public CharArrayIterator
{
  GET_CLASS_NAME(CharArrayIterator::CharArrayIteratorAnonymousInnerClass)
public:
  CharArrayIteratorAnonymousInnerClass();

  // work around this for now by lying about all surrogates to
  // the sentence tokenizer, instead we treat them all as
  // SContinue so we won't break around them.
protected:
  wchar_t jreBugWorkaround(wchar_t ch) override;

protected:
  std::shared_ptr<CharArrayIteratorAnonymousInnerClass> shared_from_this()
  {
    return std::static_pointer_cast<CharArrayIteratorAnonymousInnerClass>(
        CharArrayIterator::shared_from_this());
  }
};
class CharArrayIterator::CharArrayIteratorAnonymousInnerClass2
    : public CharArrayIterator
{
  GET_CLASS_NAME(CharArrayIterator::CharArrayIteratorAnonymousInnerClass2)
public:
  CharArrayIteratorAnonymousInnerClass2();

  // no bugs
protected:
  wchar_t jreBugWorkaround(wchar_t ch) override;

protected:
  std::shared_ptr<CharArrayIteratorAnonymousInnerClass2> shared_from_this()
  {
    return std::static_pointer_cast<CharArrayIteratorAnonymousInnerClass2>(
        CharArrayIterator::shared_from_this());
  }
};
class CharArrayIterator::CharArrayIteratorAnonymousInnerClass3
    : public CharArrayIterator
{
  GET_CLASS_NAME(CharArrayIterator::CharArrayIteratorAnonymousInnerClass3)
public:
  CharArrayIteratorAnonymousInnerClass3();

  // work around this for now by lying about all surrogates to the word,
  // instead we treat them all as ALetter so we won't break around them.
protected:
  wchar_t jreBugWorkaround(wchar_t ch) override;

protected:
  std::shared_ptr<CharArrayIteratorAnonymousInnerClass3> shared_from_this()
  {
    return std::static_pointer_cast<CharArrayIteratorAnonymousInnerClass3>(
        CharArrayIterator::shared_from_this());
  }
};
class CharArrayIterator::CharArrayIteratorAnonymousInnerClass4
    : public CharArrayIterator
{
  GET_CLASS_NAME(CharArrayIterator::CharArrayIteratorAnonymousInnerClass4)
public:
  CharArrayIteratorAnonymousInnerClass4();

  // no bugs
protected:
  wchar_t jreBugWorkaround(wchar_t ch) override;

protected:
  std::shared_ptr<CharArrayIteratorAnonymousInnerClass4> shared_from_this()
  {
    return std::static_pointer_cast<CharArrayIteratorAnonymousInnerClass4>(
        CharArrayIterator::shared_from_this());
  }
};
