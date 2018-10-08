#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

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

/**
 * Exception indicating there is no more data.
 * Thrown by Docs Makers if content.source.forever is false and docs sources of
 * that maker where exhausted. This is useful for iterating all document of a
 * source, in case we don't know in advance how many docs there are.
 */
class NoMoreDataException : public std::runtime_error
{
  GET_CLASS_NAME(NoMoreDataException)

protected:
  std::shared_ptr<NoMoreDataException> shared_from_this()
  {
    return std::static_pointer_cast<NoMoreDataException>(
        Exception::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::feeds
