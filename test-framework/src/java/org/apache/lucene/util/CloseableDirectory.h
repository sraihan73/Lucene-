#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/BaseDirectoryWrapper.h"

#include  "core/src/java/org/apache/lucene/util/TestRuleMarkFailure.h"

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
namespace org::apache::lucene::util
{

using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;

/**
 * Attempts to close a {@link BaseDirectoryWrapper}.
 *
 * @see LuceneTestCase#newDirectory(java.util.Random)
 */
class CloseableDirectory final
    : public std::enable_shared_from_this<CloseableDirectory>
{
  GET_CLASS_NAME(CloseableDirectory)
private:
  const std::shared_ptr<BaseDirectoryWrapper> dir;
  const std::shared_ptr<TestRuleMarkFailure> failureMarker;

public:
  CloseableDirectory(std::shared_ptr<BaseDirectoryWrapper> dir,
                     std::shared_ptr<TestRuleMarkFailure> failureMarker);

  virtual ~CloseableDirectory();
};

} // #include  "core/src/java/org/apache/lucene/util/
