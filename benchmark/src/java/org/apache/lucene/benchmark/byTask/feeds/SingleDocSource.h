#pragma once
#include "ContentSource.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/NoMoreDataException.h"

#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/DocData.h"

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
namespace org::apache::lucene::benchmark::byTask::feeds
{

/**
 * Creates the same document each time {@link #getNextDocData(DocData)} is
 * called.
 */
class SingleDocSource : public ContentSource
{
  GET_CLASS_NAME(SingleDocSource)

private:
  int docID = 0;

public:
  static const std::wstring DOC_TEXT;

  // return a new docid
private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  int newdocid() ;

public:
  virtual ~SingleDocSource();
  std::shared_ptr<DocData> getNextDocData(
      std::shared_ptr<DocData> docData)  override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  void resetInputs()  override;

protected:
  std::shared_ptr<SingleDocSource> shared_from_this()
  {
    return std::static_pointer_cast<SingleDocSource>(
        ContentSource::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/
