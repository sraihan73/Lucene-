#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/util/AttributeImpl.h"
#include  "core/src/java/org/apache/lucene/util/AttributeReflector.h"
#include  "core/src/java/org/apache/lucene/util/AttributeFactory.h"

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
namespace org::apache::lucene::analysis
{

using FlagsAttribute =
    org::apache::lucene::analysis::tokenattributes::FlagsAttribute;
using PackedTokenAttributeImpl =
    org::apache::lucene::analysis::tokenattributes::PackedTokenAttributeImpl;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
  A Token is an occurrence of a term from the text of a field.  It consists of
  a term's text, the start and end offset of the term in the text of the field,
  and a type string.
  <p>
  The start and end offsets permit applications to re-associate a token with
  its source text, e.g., to display highlighted query terms in a document
  browser, or to show matching text fragments in a <a
  href="http://en.wikipedia.org/wiki/Key_Word_in_Context">KWIC</a> display, etc.
  <p>
  The type is a string, assigned by a lexical analyzer
  (a.k.a. tokenizer), naming the lexical or syntactic class that the token
  belongs to.  For example an end of sentence marker token might be implemented
  with type "eos".  The default token type is "word".
  <p>
  A Token can optionally have metadata (a.k.a. payload) in the form of a
  variable length byte array. Use {@link
  org.apache.lucene.index.PostingsEnum#getPayload()} to retrieve the payloads
  from the index.

  A few things to note:
  <ul>
  <li>clear() initializes all of the fields to default values. This was changed
  in contrast to Lucene 2.4, but should affect no one.</li> <li>Because
  <code>TokenStreams</code> can be chained, one cannot assume that the
  <code>Token's</code> current type is correct.</li> <li>The startOffset and
  endOffset represent the start and offset in the source text, so be careful in
  adjusting them.</li> <li>When caching a reusable token, clone it. When
  injecting a cached token into a stream that can be reset, clone it again.</li>
  </ul>
*/
class Token : public PackedTokenAttributeImpl,
              public FlagsAttribute,
              public PayloadAttribute
{
  GET_CLASS_NAME(Token)

private:
  int flags = 0;
  std::shared_ptr<BytesRef> payload;

  /** Constructs a Token will null text. */
public:
  Token();

  /** Constructs a Token with the given term text, start
   *  and end offsets.  The type defaults to "word."
   *  <b>NOTE:</b> for better indexing speed you should
   *  instead use the char[] termBuffer methods to set the
   *  term text.
   *  @param text term text
   *  @param start start offset in the source text
   *  @param end end offset in the source text
   */
  Token(std::shared_ptr<std::wstring> text, int start, int end);

  /** Constructs a Token with the given term text, position increment, start and
   * end offsets */
  Token(std::shared_ptr<std::wstring> text, int posInc, int start, int end);

  /**
   * {@inheritDoc}
   * @see FlagsAttribute
   */
  int getFlags() override;

  /**
   * {@inheritDoc}
   * @see FlagsAttribute
   */
  void setFlags(int flags) override;

  /**
   * {@inheritDoc}
   * @see PayloadAttribute
   */
  std::shared_ptr<BytesRef> getPayload() override;

  /**
   * {@inheritDoc}
   * @see PayloadAttribute
   */
  void setPayload(std::shared_ptr<BytesRef> payload) override;

  /** Resets the term text, payload, flags, positionIncrement, positionLength,
   * startOffset, endOffset and token type to default.
   */
  void clear() override;

  bool equals(std::any obj) override;

  virtual int hashCode();

  std::shared_ptr<Token> clone() override;

  /**
   * Copy the prototype token's fields into this one. Note: Payloads are shared.
   * @param prototype source Token to copy fields from
   */
  virtual void reinit(std::shared_ptr<Token> prototype);

private:
  void copyToWithoutPayloadClone(std::shared_ptr<AttributeImpl> target);

public:
  void copyTo(std::shared_ptr<AttributeImpl> target) override;

  void reflectWith(AttributeReflector reflector) override;

  /** Convenience factory that returns <code>Token</code> as implementation for
   * the basic attributes and return the default impl (with &quot;Impl&quot;
   * appended) for all other attributes.
   * @since 3.0
   */
  static const std::shared_ptr<AttributeFactory> TOKEN_ATTRIBUTE_FACTORY;

protected:
  std::shared_ptr<Token> shared_from_this()
  {
    return std::static_pointer_cast<Token>(
        org.apache.lucene.analysis.tokenattributes
            .PackedTokenAttributeImpl::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/
