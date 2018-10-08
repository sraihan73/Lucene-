#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Matches;
}

namespace org::apache::lucene::search
{
class MatchesIterator;
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

namespace org::apache::lucene::search
{

class AssertingMatches : public std::enable_shared_from_this<AssertingMatches>,
                         public Matches
{
  GET_CLASS_NAME(AssertingMatches)

private:
  const std::shared_ptr<Matches> in_;

public:
  AssertingMatches(std::shared_ptr<Matches> matches);

  std::shared_ptr<MatchesIterator>
  getMatches(const std::wstring &field)  override;

  std::shared_ptr<Iterator<std::wstring>> iterator() override;
};

} // namespace org::apache::lucene::search
