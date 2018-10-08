#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/LuceneTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/util/ResourceLoader.h"

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
namespace org::apache::lucene::analysis::core
{

using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using StringMockResourceLoader =
    org::apache::lucene::analysis::util::StringMockResourceLoader;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Tests that any newly added Tokenizers/TokenFilters/CharFilters have a
 * corresponding factory (and that the SPI configuration is correct)
 */
class TestAllAnalyzersHaveFactories : public LuceneTestCase
{
  GET_CLASS_NAME(TestAllAnalyzersHaveFactories)

  // these are test-only components (e.g. test-framework)
private:
  static const std::shared_ptr<Set<std::type_info>> testComponents;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static TestAllAnalyzersHaveFactories::StaticConstructor staticConstructor;

  // these are 'crazy' components like cachingtokenfilter. does it make sense to
  // add factories for these?
  static const std::shared_ptr<Set<std::type_info>> crazyComponents;

  // these are oddly-named (either the actual analyzer, or its factory)
  // they do actually have factories.
  // TODO: clean this up!
  static const std::shared_ptr<Set<std::type_info>> oddlyNamedComponents;

  // The following token filters are excused from having their factory.
  static const std::shared_ptr<Set<std::type_info>> tokenFiltersWithoutFactory;

  static const std::shared_ptr<ResourceLoader> loader;

public:
  virtual void test() ;

protected:
  std::shared_ptr<TestAllAnalyzersHaveFactories> shared_from_this()
  {
    return std::static_pointer_cast<TestAllAnalyzersHaveFactories>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/core/
