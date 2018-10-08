#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <unordered_set>
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
namespace org::apache::lucene::search::vectorhighlight
{

/**
 * Simple boundary scanner implementation that divides fragments
 * based on a set of separator characters.
 */
class SimpleBoundaryScanner
    : public std::enable_shared_from_this<SimpleBoundaryScanner>,
      public BoundaryScanner
{
  GET_CLASS_NAME(SimpleBoundaryScanner)

public:
  static constexpr int DEFAULT_MAX_SCAN = 20;
  static std::deque<std::optional<wchar_t>> const DEFAULT_BOUNDARY_CHARS;

protected:
  int maxScan = 0;
  std::shared_ptr<Set<wchar_t>> boundaryChars;

public:
  SimpleBoundaryScanner();

  SimpleBoundaryScanner(int maxScan);

  SimpleBoundaryScanner(std::deque<std::optional<wchar_t> &> &boundaryChars);

  SimpleBoundaryScanner(int maxScan, std::deque<Character> &boundaryChars);

  SimpleBoundaryScanner(int maxScan,
                        std::shared_ptr<Set<wchar_t>> boundaryChars);

  int findStartOffset(std::shared_ptr<StringBuilder> buffer,
                      int start) override;

  int findEndOffset(std::shared_ptr<StringBuilder> buffer, int start) override;
};

} // namespace org::apache::lucene::search::vectorhighlight
