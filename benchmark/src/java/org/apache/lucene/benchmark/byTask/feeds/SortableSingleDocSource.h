#pragma once
#include "SingleDocSource.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::benchmark::byTask::feeds
{
class DocData;
}

namespace org::apache::lucene::benchmark::byTask::feeds
{
class NoMoreDataException;
}
namespace org::apache::lucene::benchmark::byTask::utils
{
class Config;
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
namespace org::apache::lucene::benchmark::byTask::feeds
{

using Config = org::apache::lucene::benchmark::byTask::utils::Config;

/**
 * Adds fields appropriate for sorting: country, random_string and sort_field
 * (int). Supports the following parameters:
 * <ul>
 * <li><b>sort.rng</b> - defines the range for sort-by-int field (default
 * <b>20000</b>).
 * <li><b>rand.seed</b> - defines the seed to initialize Random with (default
 * <b>13</b>).
 * </ul>
 */
class SortableSingleDocSource : public SingleDocSource
{
  GET_CLASS_NAME(SortableSingleDocSource)

private:
  static std::deque<std::wstring> COUNTRIES;

  int sortRange = 0;
  std::shared_ptr<Random> r;

public:
  std::shared_ptr<DocData> getNextDocData(
      std::shared_ptr<DocData> docData)  override;

private:
  int nextInt(int start, int end);

public:
  void setConfig(std::shared_ptr<Config> config) override;

protected:
  std::shared_ptr<SortableSingleDocSource> shared_from_this()
  {
    return std::static_pointer_cast<SortableSingleDocSource>(
        SingleDocSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::feeds
