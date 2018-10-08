#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/FieldType.h"

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

using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/** Test that creates way, way, way too many fields */
class TestManyFields : public LuceneTestCase
{
  GET_CLASS_NAME(TestManyFields)
private:
  static const std::shared_ptr<FieldType> storedTextType;

public:
  virtual void testManyFields() ;

  virtual void testDiverseDocs() ;

  // LUCENE-4398
  virtual void testRotatingFieldNames() ;

protected:
  std::shared_ptr<TestManyFields> shared_from_this()
  {
    return std::static_pointer_cast<TestManyFields>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
