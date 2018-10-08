#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/similarities/BasicStats.h"

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
namespace org::apache::lucene::search::similarities
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.similarities.SimilarityBase.log2;

/**
 * An approximation of the <em>I(n<sub>e</sub>)</em> model.
 * @lucene.experimental
 */
class BasicModelIF : public BasicModel
{
  GET_CLASS_NAME(BasicModelIF)

  /** Sole constructor: parameter-free */
public:
  BasicModelIF();

  float score(std::shared_ptr<BasicStats> stats, float tfn) override final;

  virtual std::wstring toString();

protected:
  std::shared_ptr<BasicModelIF> shared_from_this()
  {
    return std::static_pointer_cast<BasicModelIF>(
        BasicModel::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/similarities/
