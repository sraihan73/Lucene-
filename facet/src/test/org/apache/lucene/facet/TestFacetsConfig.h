#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
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
namespace org::apache::lucene::facet
{

class TestFacetsConfig : public FacetTestCase
{
  GET_CLASS_NAME(TestFacetsConfig)

public:
  virtual void testPathToStringAndBack() ;

  virtual void testAddSameDocTwice() ;

  /** LUCENE-5479 */
  virtual void testCustomDefault();

private:
  class FacetsConfigAnonymousInnerClass : public FacetsConfig
  {
    GET_CLASS_NAME(FacetsConfigAnonymousInnerClass)
  private:
    std::shared_ptr<TestFacetsConfig> outerInstance;

  public:
    FacetsConfigAnonymousInnerClass(
        std::shared_ptr<TestFacetsConfig> outerInstance);

  protected:
    std::shared_ptr<DimConfig> getDefaultDimConfig() override;

  protected:
    std::shared_ptr<FacetsConfigAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FacetsConfigAnonymousInnerClass>(
          FacetsConfig::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestFacetsConfig> shared_from_this()
  {
    return std::static_pointer_cast<TestFacetsConfig>(
        FacetTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/
