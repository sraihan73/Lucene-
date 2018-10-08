#pragma once
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
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

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.search.uhighlight.TestWholeBreakIterator.assertSameBreaks;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.hamcrest.CoreMatchers.equalTo;

class TestCustomSeparatorBreakIterator : public LuceneTestCase
{
  GET_CLASS_NAME(TestCustomSeparatorBreakIterator)

private:
  static std::deque<std::optional<wchar_t>> const SEPARATORS;

public:
  virtual void testBreakOnCustomSeparator() ;

  virtual void testSingleSentences() ;

  virtual void testSliceEnd() ;

  virtual void testSliceStart() ;

  virtual void testSliceMiddle() ;

  /** the current position must be ignored, initial position is always first()
   */
  virtual void testFirstPosition() ;

private:
  static wchar_t randomSeparator();

protected:
  std::shared_ptr<TestCustomSeparatorBreakIterator> shared_from_this()
  {
    return std::static_pointer_cast<TestCustomSeparatorBreakIterator>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::uhighlight
