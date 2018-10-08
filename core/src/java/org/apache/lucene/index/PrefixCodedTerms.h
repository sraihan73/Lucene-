#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class RAMFile;
}

namespace org::apache::lucene::store
{
class RAMOutputStream;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::store
{
class IndexInput;
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
namespace org::apache::lucene::index
{

using IndexInput = org::apache::lucene::store::IndexInput;
using RAMFile = org::apache::lucene::store::RAMFile;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

/**
 * Prefix codes term instances (prefixes are shared). This is expected to be
 * faster to build than a FST and might also be more compact if there are no
 * common suffixes.
 * @lucene.internal
 */
class PrefixCodedTerms : public std::enable_shared_from_this<PrefixCodedTerms>,
                         public Accountable
{
  GET_CLASS_NAME(PrefixCodedTerms)
public:
  const std::shared_ptr<RAMFile> buffer;

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  const int64_t size_;
  int64_t delGen = 0;

  PrefixCodedTerms(std::shared_ptr<RAMFile> buffer, int64_t size);

public:
  int64_t ramBytesUsed() override;

  /** Records del gen for this packet. */
  virtual void setDelGen(int64_t delGen);

  /** Builds a PrefixCodedTerms: call add repeatedly, then finish. */
public:
  class Builder : public std::enable_shared_from_this<Builder>
  {
    GET_CLASS_NAME(Builder)
  private:
    std::shared_ptr<RAMFile> buffer = std::make_shared<RAMFile>();
    std::shared_ptr<RAMOutputStream> output =
        std::make_shared<RAMOutputStream>(buffer, false);
    std::shared_ptr<Term> lastTerm = std::make_shared<Term>(L"");
    std::shared_ptr<BytesRefBuilder> lastTermBytes =
        std::make_shared<BytesRefBuilder>();
    int64_t size = 0;

    /** Sole constructor. */
  public:
    Builder();

    /** add a term */
    virtual void add(std::shared_ptr<Term> term);

    /** add a term.  This fully consumes in the incoming {@link BytesRef}. */
    virtual void add(const std::wstring &field,
                     std::shared_ptr<BytesRef> bytes);

    /** return finalized form */
    virtual std::shared_ptr<PrefixCodedTerms> finish();
  };

  /** An iterator over the deque of terms stored in a {@link PrefixCodedTerms}.
   */
public:
  class TermIterator : public FieldTermIterator
  {
    GET_CLASS_NAME(TermIterator)
  public:
    const std::shared_ptr<IndexInput> input;
    const std::shared_ptr<BytesRefBuilder> builder =
        std::make_shared<BytesRefBuilder>();
    const std::shared_ptr<BytesRef> bytes = builder->get();
    const int64_t end;
    // C++ NOTE: Fields cannot have the same name as methods:
    const int64_t delGen_;
    // C++ NOTE: Fields cannot have the same name as methods:
    std::wstring field_ = L"";

  private:
    TermIterator(int64_t delGen, std::shared_ptr<RAMFile> buffer);

  public:
    std::shared_ptr<BytesRef> next() override;

    // TODO: maybe we should freeze to FST or automaton instead?
  private:
    void readTermBytes(int prefix, int suffix) ;

  public:
    std::wstring field() override;

    int64_t delGen() override;

  protected:
    std::shared_ptr<TermIterator> shared_from_this()
    {
      return std::static_pointer_cast<TermIterator>(
          FieldTermIterator::shared_from_this());
    }
  };

  /** Return an iterator over the terms stored in this {@link PrefixCodedTerms}.
   */
public:
  virtual std::shared_ptr<TermIterator> iterator();

  /** Return the number of terms stored in this {@link PrefixCodedTerms}. */
  virtual int64_t size();

  virtual int hashCode();

  bool equals(std::any obj) override;
};

} // namespace org::apache::lucene::index
