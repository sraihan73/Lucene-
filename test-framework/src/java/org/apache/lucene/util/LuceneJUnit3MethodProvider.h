#pragma once
#include "stringhelper.h"
#include <memory>
#include <typeinfo>
#include <unordered_map>
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
namespace org::apache::lucene::util
{

using com::carrotsearch::randomizedtesting::ClassModel;
using com::carrotsearch::randomizedtesting::TestMethodProvider;

/**
 * Backwards compatible test* method provider (public, non-static).
 */
class LuceneJUnit3MethodProvider final
    : public std::enable_shared_from_this<LuceneJUnit3MethodProvider>,
      public TestMethodProvider
{
  GET_CLASS_NAME(LuceneJUnit3MethodProvider)
public:
  std::shared_ptr<std::deque<std::shared_ptr<Method>>>
  getTestMethods(std::type_info suiteClass,
                 std::shared_ptr<ClassModel> classModel) override;
};

} // namespace org::apache::lucene::util
