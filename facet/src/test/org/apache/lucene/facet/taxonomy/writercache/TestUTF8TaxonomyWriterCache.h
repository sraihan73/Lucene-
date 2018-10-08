#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/taxonomy/FacetLabel.h"

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

namespace org::apache::lucene::facet::taxonomy::writercache
{

using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using FacetLabel = org::apache::lucene::facet::taxonomy::FacetLabel;

class TestUTF8TaxonomyWriterCache : public FacetTestCase
{
  GET_CLASS_NAME(TestUTF8TaxonomyWriterCache)

public:
  virtual void testRandom() ;

private:
  class LabelToOrdinalMap : public LabelToOrdinal
  {
    GET_CLASS_NAME(LabelToOrdinalMap)
  private:
    std::unordered_map<std::shared_ptr<FacetLabel>, int> map_obj =
        std::unordered_map<std::shared_ptr<FacetLabel>, int>();

  public:
    LabelToOrdinalMap();

    void addLabel(std::shared_ptr<FacetLabel> label, int ordinal) override;

    int getOrdinal(std::shared_ptr<FacetLabel> label) override;

  protected:
    std::shared_ptr<LabelToOrdinalMap> shared_from_this()
    {
      return std::static_pointer_cast<LabelToOrdinalMap>(
          LabelToOrdinal::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestUTF8TaxonomyWriterCache> shared_from_this()
  {
    return std::static_pointer_cast<TestUTF8TaxonomyWriterCache>(
        org.apache.lucene.facet.FacetTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/writercache/
