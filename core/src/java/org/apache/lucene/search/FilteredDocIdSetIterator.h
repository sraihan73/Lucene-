#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class DocIdSetIterator;
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

/**
 * Abstract decorator class of a DocIdSetIterator
 * implementation that provides on-demand filter/validation
 * mechanism on an underlying DocIdSetIterator.
 */
class FilteredDocIdSetIterator : public DocIdSetIterator
{
  GET_CLASS_NAME(FilteredDocIdSetIterator)
protected:
  std::shared_ptr<DocIdSetIterator> _innerIter;

private:
  int doc = 0;

  /**
   * Constructor.
   * @param innerIter Underlying DocIdSetIterator.
   */
public:
  FilteredDocIdSetIterator(std::shared_ptr<DocIdSetIterator> innerIter);

  /** Return the wrapped {@link DocIdSetIterator}. */
  virtual std::shared_ptr<DocIdSetIterator> getDelegate();

  /**
   * Validation method to determine whether a docid should be in the result set.
   * @param doc docid to be tested
   * @return true if input docid should be in the result set, false otherwise.
   * @see #FilteredDocIdSetIterator(DocIdSetIterator)
   */
protected:
  virtual bool match(int doc) = 0;

public:
  int docID() override;

  int nextDoc()  override;

  int advance(int target)  override;

  int64_t cost() override;

protected:
  std::shared_ptr<FilteredDocIdSetIterator> shared_from_this()
  {
    return std::static_pointer_cast<FilteredDocIdSetIterator>(
        DocIdSetIterator::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
