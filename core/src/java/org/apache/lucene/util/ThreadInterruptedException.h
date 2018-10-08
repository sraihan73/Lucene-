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
namespace org::apache::lucene::util
{

/**
 * Thrown by lucene on detecting that Thread.interrupt() had
 * been called.  Unlike Java's InterruptedException, this
 * exception is not checked..
 */

class ThreadInterruptedException final : public std::runtime_error
{
  GET_CLASS_NAME(ThreadInterruptedException)
public:
  ThreadInterruptedException(std::shared_ptr<InterruptedException> ie);

protected:
  std::shared_ptr<ThreadInterruptedException> shared_from_this()
  {
    return std::static_pointer_cast<ThreadInterruptedException>(
        RuntimeException::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
