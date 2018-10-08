#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

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
namespace org::apache::lucene::search::grouping
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Base class for grouping related tests.
 */
// TODO (MvG) : The grouping tests contain a lot of code duplication. Try to
// move the common code to this class..
class AbstractGroupingTestCase : public LuceneTestCase
{

protected:
  virtual std::wstring generateRandomNonEmptyString();

protected:
  std::shared_ptr<AbstractGroupingTestCase> shared_from_this()
  {
    return std::static_pointer_cast<AbstractGroupingTestCase>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::grouping
