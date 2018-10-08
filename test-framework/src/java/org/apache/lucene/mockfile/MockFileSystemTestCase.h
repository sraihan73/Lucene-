#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>

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
namespace org::apache::lucene::mockfile
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Base class for testing mockfilesystems. This tests things
 * that really need to work: Path equals()/hashcode(), directory listing
 * glob and filtering, URI conversion, etc.
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressFileSystems("*") public abstract class
// MockFileSystemTestCase extends org.apache.lucene.util.LuceneTestCase
class MockFileSystemTestCase : public LuceneTestCase
{

  /** wraps Path with custom behavior */
protected:
  virtual std::shared_ptr<Path> wrap(std::shared_ptr<Path> path) = 0;

  /** Test that Path.hashcode/equals are sane */
public:
  virtual void testHashCodeEquals() ;

  /** Test that URIs are not corrumpted */
  virtual void testURI() ;

  virtual void testURIumlaute() ;

  virtual void testURIchinese() ;

private:
  void implTestURI(const std::wstring &fileName) ;

  /** Tests that newDirectoryStream with a filter works correctly */
public:
  virtual void testDirectoryStreamFiltered() ;

  /** Tests that newDirectoryStream with globbing works correctly */
  virtual void testDirectoryStreamGlobFiltered() ;

protected:
  std::shared_ptr<MockFileSystemTestCase> shared_from_this()
  {
    return std::static_pointer_cast<MockFileSystemTestCase>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/mockfile/
