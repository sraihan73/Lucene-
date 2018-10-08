#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/document/Field.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"

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
namespace org::apache::lucene::index
{

using Field = org::apache::lucene::document::Field;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

class TestOmitNorms : public LuceneTestCase
{
  GET_CLASS_NAME(TestOmitNorms)
  // Tests whether the DocumentWriter correctly enable the
  // omitNorms bit in the FieldInfo
public:
  virtual void testOmitNorms() ;

  // Tests whether merging of docs that have different
  // omitNorms for the same field works
  virtual void testMixedMerge() ;

  // Make sure first adding docs that do not omitNorms for
  // field X, then adding docs that do omitNorms for that same
  // field,
  virtual void testMixedRAM() ;

private:
  void assertNoNrm(std::shared_ptr<Directory> dir) ;

  // Verifies no *.nrm exists when all fields omit norms:
public:
  virtual void testNoNrmFile() ;

  /**
   * Tests various combinations of omitNorms=true/false, the field not existing
   * at all, ensuring that only omitNorms is 'viral'. Internally checks that
   * MultiNorms.norms() is consistent (returns the same bytes) as the fully
   * merged equivalent.
   */
  virtual void testOmitNormsCombos() ;

  /**
   * Indexes at least 1 document with f1, and at least 1 document with f2.
   * returns the norms for "field".
   */
  virtual std::shared_ptr<NumericDocValues>
  getNorms(const std::wstring &field, std::shared_ptr<Field> f1,
           std::shared_ptr<Field> f2) ;

protected:
  std::shared_ptr<TestOmitNorms> shared_from_this()
  {
    return std::static_pointer_cast<TestOmitNorms>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
