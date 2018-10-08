#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class FSLockFactory;
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
namespace org::apache::lucene::store
{

/**
 * Simple standalone tool that forever acquires and releases a
 * lock using a specific LockFactory.  Run without any args
 * to see usage.
 *
 * @see VerifyingLockFactory
 * @see LockVerifyServer
 */

class LockStressTest : public std::enable_shared_from_this<LockStressTest>
{
  GET_CLASS_NAME(LockStressTest)

public:
  static const std::wstring LOCK_FILE_NAME;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressForbidden(reason = "System.out required: command
  // line tool") @SuppressWarnings("try") public static void main(std::wstring[] args)
  // throws Exception
  static void main(std::deque<std::wstring> &args) ;

private:
  static std::shared_ptr<FSLockFactory> getNewLockFactory(
      const std::wstring &lockFactoryClassName) ;
};

} // namespace org::apache::lucene::store
