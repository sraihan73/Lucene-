#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/ext/Extensions.h"

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
namespace org::apache::lucene::queryparser::ext
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Testcase for the {@link Extensions} class
 */
class TestExtensions : public LuceneTestCase
{
  GET_CLASS_NAME(TestExtensions)

private:
  std::shared_ptr<Extensions> ext;

public:
  void setUp()  override;

  virtual void testBuildExtensionField();

  virtual void testSplitExtensionField();

  virtual void testAddGetExtension();

  virtual void testGetExtDelimiter();

  virtual void testEscapeExtension();

protected:
  std::shared_ptr<TestExtensions> shared_from_this()
  {
    return std::static_pointer_cast<TestExtensions>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/ext/
