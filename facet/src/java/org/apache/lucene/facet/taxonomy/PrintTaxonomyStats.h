#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::facet::taxonomy
{
class TaxonomyReader;
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
namespace org::apache::lucene::facet::taxonomy
{

/** Prints how many ords are under each dimension. */

// java -cp ../build/core/classes/java:../build/facet/classes/java
// org.apache.lucene.facet.util.PrintTaxonomyStats -printTree
// /s2/scratch/indices/wikibig.trunk.noparents.facets.Lucene41.nd1M/facets
class PrintTaxonomyStats
    : public std::enable_shared_from_this<PrintTaxonomyStats>
{

  /** Sole constructor. */
public:
  PrintTaxonomyStats();

  /** Command-line tool. */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressForbidden(reason = "System.out required: command
  // line tool") public static void main(std::wstring[] args) throws
  // java.io.IOException
  static void main(std::deque<std::wstring> &args) ;

  /** Recursively prints stats for all ordinals. */
  static void printStats(std::shared_ptr<TaxonomyReader> r,
                         std::shared_ptr<PrintStream> out,
                         bool printTree) ;

private:
  static int countAllChildren(std::shared_ptr<TaxonomyReader> r,
                              int ord) ;

  static void printAllChildren(std::shared_ptr<PrintStream> out,
                               std::shared_ptr<TaxonomyReader> r, int ord,
                               const std::wstring &indent,
                               int depth) ;
};

} // namespace org::apache::lucene::facet::taxonomy
