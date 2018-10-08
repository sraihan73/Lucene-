#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/IndexInput.h"

#include  "core/src/java/org/apache/lucene/store/MockDirectoryWrapper.h"

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
 * Takes a while to open files: gives testThreadInterruptDeadlock
 * a chance to find file leaks if opening an input throws exception
 */
class SlowOpeningMockIndexInputWrapper : public MockIndexInputWrapper
{
  GET_CLASS_NAME(SlowOpeningMockIndexInputWrapper)

public:
  SlowOpeningMockIndexInputWrapper(
      std::shared_ptr<MockDirectoryWrapper> dir, const std::wstring &name,
      std::shared_ptr<IndexInput> delegate_) ;

protected:
  std::shared_ptr<SlowOpeningMockIndexInputWrapper> shared_from_this()
  {
    return std::static_pointer_cast<SlowOpeningMockIndexInputWrapper>(
        MockIndexInputWrapper::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/store/
