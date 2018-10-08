#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
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

/**
 * A {@link Closeable} that attempts to remove a given file/folder.
 */
class RemoveUponClose final
    : public std::enable_shared_from_this<RemoveUponClose>
{
  GET_CLASS_NAME(RemoveUponClose)
private:
  const std::shared_ptr<Path> path;
  const std::shared_ptr<TestRuleMarkFailure> failureMarker;
  const std::wstring creationStack;

public:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  RemoveUponClose(std::shared_ptr<Path> path,
                  std::shared_ptr<TestRuleMarkFailure> failureMarker);

  virtual ~RemoveUponClose();
};
} // #include  "core/src/java/org/apache/lucene/util/
