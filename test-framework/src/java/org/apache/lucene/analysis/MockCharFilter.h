#pragma once
#include "stringhelper.h"
#include <map_obj>
#include <memory>
#include <stdexcept>
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
namespace org::apache::lucene::analysis
{

/** the purpose of this charfilter is to send offsets out of bounds
  if the analyzer doesn't use correctOffset or does incorrect offset math. */
class MockCharFilter : public CharFilter
{
  GET_CLASS_NAME(MockCharFilter)
public:
  const int remainder;

  // for testing only
  MockCharFilter(std::shared_ptr<Reader> in_, int remainder);

  // for testing only, uses a remainder of 0
  MockCharFilter(std::shared_ptr<Reader> in_);

  int currentOffset = -1;
  int delta = 0;
  int bufferedCh = -1;

  int read()  override;

  int read(std::deque<wchar_t> &cbuf, int off,
           int len)  override;

  int correct(int currentOff) override;

protected:
  virtual void addOffCorrectMap(int off, int cumulativeDiff);

public:
  std::map_obj<int, int> corrections = std::map_obj<int, int>();

protected:
  std::shared_ptr<MockCharFilter> shared_from_this()
  {
    return std::static_pointer_cast<MockCharFilter>(
        CharFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis
