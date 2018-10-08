#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
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
namespace org::apache::lucene::queryparser::surround::query
{

using Directory = org::apache::lucene::store::Directory;

class SingleFieldTestDb : public std::enable_shared_from_this<SingleFieldTestDb>
{
  GET_CLASS_NAME(SingleFieldTestDb)
private:
  std::shared_ptr<Directory> db;
  std::deque<std::wstring> docs;
  std::wstring fieldName;

public:
  SingleFieldTestDb(std::shared_ptr<Random> random,
                    std::deque<std::wstring> &documents,
                    const std::wstring &fName);

  virtual std::shared_ptr<Directory> getDb();
  virtual std::deque<std::wstring> getDocs();
  virtual std::wstring getFieldname();
};

} // namespace org::apache::lucene::queryparser::surround::query
