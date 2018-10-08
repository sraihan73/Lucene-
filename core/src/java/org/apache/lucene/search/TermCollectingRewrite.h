#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class TermCollector;
}

namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::index
{
class IndexReaderContext;
}
namespace org::apache::lucene::util
{
class AttributeSource;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::index
{
class TermsEnum;
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
namespace org::apache::lucene::search
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using BytesRef = org::apache::lucene::util::BytesRef;

template <typename B>
class TermCollectingRewrite : public MultiTermQuery::RewriteMethod
{
  GET_CLASS_NAME(TermCollectingRewrite)

  /** Return a suitable builder for the top-level Query for holding all expanded
   * terms. */
protected:
  virtual B getTopLevelBuilder() = 0;

  /** Finalize the creation of the query from the builder. */
  virtual std::shared_ptr<Query> build(B builder) = 0;

  /** Add a MultiTermQuery term to the top-level query builder. */
  void addClause(B topLevel, std::shared_ptr<Term> term, int docCount,
                 float boost) 
  {
    addClause(topLevel, term, docCount, boost, nullptr);
  }

  virtual void addClause(B topLevel, std::shared_ptr<Term> term, int docCount,
                         float boost, std::shared_ptr<TermContext> states) = 0;

public:
  void collectTerms(std::shared_ptr<IndexReader> reader,
                    std::shared_ptr<MultiTermQuery> query,
                    std::shared_ptr<TermCollector> collector) 
  {
    std::shared_ptr<IndexReaderContext> topReaderContext = reader->getContext();
    for (auto context : topReaderContext->leaves()) {
      std::shared_ptr<Terms> *const terms =
          context->reader()->terms(query->field);
      if (terms == nullptr) {
        // field does not exist
        continue;
      }

      std::shared_ptr<TermsEnum> *const termsEnum =
          getTermsEnum(query, terms, collector->attributes);
      assert(termsEnum != nullptr);

      if (termsEnum == TermsEnum::EMPTY) {
        continue;
      }

      collector->setReaderContext(topReaderContext, context);
      collector->setNextEnum(termsEnum);
      std::shared_ptr<BytesRef> bytes;
      while ((bytes = termsEnum->next()) != nullptr) {
        if (!collector->collect(bytes)) {
          return; // interrupt whole term collection, so also don't iterate
                  // other subReaders
        }
      }
    }
  }

public:
  class TermCollector : public std::enable_shared_from_this<TermCollector>
  {
    GET_CLASS_NAME(TermCollector)

  protected:
    std::shared_ptr<LeafReaderContext> readerContext;
    std::shared_ptr<IndexReaderContext> topReaderContext;

  public:
    virtual void
    setReaderContext(std::shared_ptr<IndexReaderContext> topReaderContext,
                     std::shared_ptr<LeafReaderContext> readerContext);
    /** attributes used for communication with the enum */
    const std::shared_ptr<AttributeSource> attributes =
        std::make_shared<AttributeSource>();

    /** return false to stop collecting */
    virtual bool collect(std::shared_ptr<BytesRef> bytes) = 0;

    /** the next segment's {@link TermsEnum} that is used to collect terms */
    virtual void setNextEnum(std::shared_ptr<TermsEnum> termsEnum) = 0;
  };

protected:
  std::shared_ptr<TermCollectingRewrite> shared_from_this()
  {
    return std::static_pointer_cast<TermCollectingRewrite>(
        MultiTermQuery.RewriteMethod::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
