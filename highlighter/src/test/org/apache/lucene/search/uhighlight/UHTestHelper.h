#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class FieldType;
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
namespace org::apache::lucene::search::uhighlight
{

using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;

/**
 * Helper for {@link UnifiedHighlighter} tests.
 */
class UHTestHelper : public std::enable_shared_from_this<UHTestHelper>
{
  GET_CLASS_NAME(UHTestHelper)

public:
  static const std::shared_ptr<FieldType> postingsType;
  static const std::shared_ptr<FieldType> tvType;
  static const std::shared_ptr<FieldType> postingsWithTvType;
  static const std::shared_ptr<FieldType> reanalysisType;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static UHTestHelper::StaticConstructor staticConstructor;

public:
  static std::shared_ptr<FieldType>
  randomFieldType(std::shared_ptr<Random> random,
                  std::deque<FieldType> &typePossibilities);

  /**
   * for {@link
   * com.carrotsearch.randomizedtesting.annotations.ParametersFactory}
   */
  // https://github.com/carrotsearch/randomizedtesting/blob/master/examples/maven/src/main/java/com/carrotsearch/examples/randomizedrunner/Test007ParameterizedTests.java
  static std::deque<std::deque<std::any>> parametersFactoryList();
};

} // namespace org::apache::lucene::search::uhighlight
