#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/StoredFieldVisitor.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class IndexSearcher;
}

namespace org::apache::lucene::index
{
class FieldInfo;
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
namespace org::apache::lucene::benchmark::quality::utils
{

using IndexSearcher = org::apache::lucene::search::IndexSearcher;

/**
 * Utility: extract doc names from an index
 */
class DocNameExtractor : public std::enable_shared_from_this<DocNameExtractor>
{
  GET_CLASS_NAME(DocNameExtractor)

private:
  const std::wstring docNameField;

  /**
   * Constructor for DocNameExtractor.
   * @param docNameField name of the stored field containing the doc name.
   */
public:
  DocNameExtractor(const std::wstring &docNameField);

  /**
   * Extract the name of the input doc from the index.
   * @param searcher access to the index.
   * @param docid ID of doc whose name is needed.
   * @return the name of the input doc as extracted from the index.
   * @throws IOException if cannot extract the doc name from the index.
   */
  virtual std::wstring docName(std::shared_ptr<IndexSearcher> searcher,
                               int docid) ;

private:
  class StoredFieldVisitorAnonymousInnerClass : public StoredFieldVisitor
  {
    GET_CLASS_NAME(StoredFieldVisitorAnonymousInnerClass)
  private:
    std::shared_ptr<DocNameExtractor> outerInstance;

    std::deque<std::wstring> name;

  public:
    StoredFieldVisitorAnonymousInnerClass(
        std::shared_ptr<DocNameExtractor> outerInstance,
        std::deque<std::wstring> &name);

    void stringField(std::shared_ptr<FieldInfo> fieldInfo,
                     std::deque<char> &bytes) override;

    Status needsField(std::shared_ptr<FieldInfo> fieldInfo) override;

  protected:
    std::shared_ptr<StoredFieldVisitorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<StoredFieldVisitorAnonymousInnerClass>(
          org.apache.lucene.index.StoredFieldVisitor::shared_from_this());
    }
  };
};

} // namespace org::apache::lucene::benchmark::quality::utils
