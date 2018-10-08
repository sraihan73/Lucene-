#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
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
namespace org::apache::lucene::search::uhighlight
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestSplittingBreakIterator : public LuceneTestCase
{
  GET_CLASS_NAME(TestSplittingBreakIterator)

private:
  static const std::shared_ptr<BreakIterator> LINE_BI;
  static const std::shared_ptr<BreakIterator> SPLIT_BI;

public:
  virtual void testLineBreakIterator();

private:
  void testWithoutSplits(std::shared_ptr<BreakIterator> bi);

public:
  virtual void testWithoutSplits();

  virtual void testOnlySingleSplitChar();

  virtual void testSplitThenValue();

  virtual void testValueThenSplit();

  virtual void testValueThenSplitThenValue();

  virtual void testValueThenDoubleSplitThenValue();

  virtual void testValueThenSplitThenDoubleValueThenSplitThenValue();

private:
  void testBreakIterator(std::shared_ptr<BreakIterator> bi,
                         const std::wstring &text,
                         const std::wstring &boundaries);

  void testFirstAndLast(std::shared_ptr<BreakIterator> bi,
                        const std::wstring &text,
                        const std::wstring &boundaries);

  void testFollowing(std::shared_ptr<BreakIterator> bi,
                     const std::wstring &text, const std::wstring &boundaries,
                     std::deque<int> &indexes);

  void testPreceding(std::shared_ptr<BreakIterator> bi,
                     const std::wstring &text, const std::wstring &boundaries,
                     std::deque<int> &indexes);

  std::deque<int> randomIntsBetweenInclusive(int end);

  void testPrevious(std::shared_ptr<BreakIterator> bi, const std::wstring &text,
                    const std::wstring &boundaries);

  /**
   * Returns a string comprised of spaces and '^' only at the boundaries.
   */
  std::wstring readBoundariesToString(std::shared_ptr<BreakIterator> bi,
                                      const std::wstring &text);

protected:
  std::shared_ptr<TestSplittingBreakIterator> shared_from_this()
  {
    return std::static_pointer_cast<TestSplittingBreakIterator>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::uhighlight
