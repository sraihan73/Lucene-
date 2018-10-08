using namespace std;

#include "Analyzer.h"
#include "../store/AlreadyClosedException.h"
#include "../util/AttributeFactory.h"
#include "../util/BytesRef.h"
#include "../util/Version.h"
#include "ReusableStringReader.h"
#include "Tokenizer.h"
#include "tokenattributes/CharTermAttribute.h"
#include "tokenattributes/OffsetAttribute.h"
#include "tokenattributes/TermToBytesRefAttribute.h"

namespace org::apache::lucene::analysis
{
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using TermToBytesRefAttribute =
    org::apache::lucene::analysis::tokenattributes::TermToBytesRefAttribute;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using BytesRef = org::apache::lucene::util::BytesRef;
using Version = org::apache::lucene::util::Version;

Analyzer::Analyzer() : Analyzer(GLOBAL_REUSE_STRATEGY) {}

Analyzer::Analyzer(shared_ptr<ReuseStrategy> reuseStrategy)
    : reuseStrategy(reuseStrategy)
{
}

shared_ptr<TokenStream> Analyzer::normalize(const wstring &fieldName,
                                            shared_ptr<TokenStream> in_)
{
  return in_;
}

shared_ptr<TokenStream> Analyzer::tokenStream(const wstring &fieldName,
                                              shared_ptr<Reader> reader)
{
  shared_ptr<TokenStreamComponents> components =
      reuseStrategy->getReusableComponents(shared_from_this(), fieldName);
  shared_ptr<Reader> *const r = initReader(fieldName, reader);
  if (components == nullptr) {
    components = createComponents(fieldName);
    reuseStrategy->setReusableComponents(shared_from_this(), fieldName,
                                         components);
  }
  components->setReader(r);
  return components->getTokenStream();
}

shared_ptr<TokenStream> Analyzer::tokenStream(const wstring &fieldName,
                                              const wstring &text)
{
  shared_ptr<TokenStreamComponents> components =
      reuseStrategy->getReusableComponents(shared_from_this(), fieldName);
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("resource") final ReusableStringReader
  // strReader = (components == null || components.reusableStringReader == null)
  // ? new ReusableStringReader() : components.reusableStringReader;
  shared_ptr<ReusableStringReader> *const strReader =
      (components == nullptr || components->reusableStringReader == nullptr)
          ? make_shared<ReusableStringReader>()
          : components->reusableStringReader;
  strReader->setValue(text);
  shared_ptr<Reader> *const r = initReader(fieldName, strReader);
  if (components == nullptr) {
    components = createComponents(fieldName);
    reuseStrategy->setReusableComponents(shared_from_this(), fieldName,
                                         components);
  }

  components->setReader(r);
  components->reusableStringReader = strReader;
  return components->getTokenStream();
}

shared_ptr<BytesRef> Analyzer::normalize(const wstring &fieldName,
                                         const wstring &text)
{
  try {
    // apply char filters
    const wstring filteredText;
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (java.io.Reader reader = new
    // java.io.StringReader(text))
    {
      java::io::Reader reader = java::io::StringReader(text);
      try {
        shared_ptr<Reader> filterReader =
            initReaderForNormalization(fieldName, reader);
        std::deque<wchar_t> buffer(64);
        shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
        for (;;) {
          constexpr int read = filterReader->read(buffer, 0, buffer.size());
          if (read == -1) {
            break;
          }
          builder->append(buffer, 0, read);
        }
        filteredText = builder->toString();
      } catch (const IOException &e) {
        throw make_shared<IllegalStateException>(
            L"Normalization threw an unexpected exception", e);
      }
    }

    shared_ptr<AttributeFactory> *const attributeFactory =
        this->attributeFactory(fieldName);
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (TokenStream ts = normalize(fieldName, new
    // StringTokenStream(attributeFactory, filteredText, text.length())))
    {
      TokenStream ts = normalize(
          fieldName, make_shared<StringTokenStream>(
                         attributeFactory, filteredText, text.length()));
      shared_ptr<TermToBytesRefAttribute> *const termAtt =
          ts->addAttribute(TermToBytesRefAttribute::typeid);
      ts->reset();
      if (ts->incrementToken() == false) {
        throw make_shared<IllegalStateException>(
            wstring(L"The normalization token stream is ") +
            L"expected to produce exactly 1 token, but got 0 for analyzer " +
            shared_from_this() + L" and input \"" + text + L"\"");
      }
      shared_ptr<BytesRef> *const term =
          BytesRef::deepCopyOf(termAtt->getBytesRef());
      if (ts->incrementToken()) {
        throw make_shared<IllegalStateException>(
            wstring(L"The normalization token stream is ") +
            L"expected to produce exactly 1 token, but got 2+ for analyzer " +
            shared_from_this() + L" and input \"" + text + L"\"");
      }
      ts->end();
      return term;
    }
  } catch (const IOException &e) {
    throw make_shared<IllegalStateException>(
        L"Normalization threw an unexpected exception", e);
  }
}

shared_ptr<Reader> Analyzer::initReader(const wstring &fieldName,
                                        shared_ptr<Reader> reader)
{
  return reader;
}

shared_ptr<Reader>
Analyzer::initReaderForNormalization(const wstring &fieldName,
                                     shared_ptr<Reader> reader)
{
  return reader;
}

shared_ptr<AttributeFactory>
Analyzer::attributeFactory(const wstring &fieldName)
{
  return TokenStream::DEFAULT_TOKEN_ATTRIBUTE_FACTORY;
}

int Analyzer::getPositionIncrementGap(const wstring &fieldName) { return 0; }

int Analyzer::getOffsetGap(const wstring &fieldName) { return 1; }

shared_ptr<ReuseStrategy> Analyzer::getReuseStrategy() { return reuseStrategy; }

void Analyzer::setVersion(shared_ptr<Version> v)
{
  version = v; // TODO: make write once?
}

shared_ptr<Version> Analyzer::getVersion() { return version; }

Analyzer::~Analyzer()
{
  if (storedValue != nullptr) {
    delete storedValue;
    storedValue.reset();
  }
}

Analyzer::TokenStreamComponents::TokenStreamComponents(
    shared_ptr<Tokenizer> source, shared_ptr<TokenStream> result)
    : source(source), sink(result)
{
}

Analyzer::TokenStreamComponents::TokenStreamComponents(
    shared_ptr<Tokenizer> source)
    : source(source), sink(source)
{
}

void Analyzer::TokenStreamComponents::setReader(shared_ptr<Reader> reader)
{
  source->setReader(reader);
}

shared_ptr<TokenStream> Analyzer::TokenStreamComponents::getTokenStream()
{
  return sink;
}

shared_ptr<Tokenizer> Analyzer::TokenStreamComponents::getTokenizer()
{
  return source;
}

Analyzer::ReuseStrategy::ReuseStrategy() {}

any Analyzer::ReuseStrategy::getStoredValue(shared_ptr<Analyzer> analyzer)
{
  if (analyzer->storedValue == nullptr) {
    throw make_shared<AlreadyClosedException>(L"this Analyzer is closed");
  }
  return analyzer->storedValue->get();
}

void Analyzer::ReuseStrategy::setStoredValue(shared_ptr<Analyzer> analyzer,
                                             any storedValue)
{
  if (analyzer->storedValue == nullptr) {
    throw make_shared<AlreadyClosedException>(L"this Analyzer is closed");
  }
  analyzer->storedValue->set(storedValue);
}

const shared_ptr<ReuseStrategy> Analyzer::GLOBAL_REUSE_STRATEGY =
    make_shared<ReuseStrategyAnonymousInnerClass>();

Analyzer::ReuseStrategyAnonymousInnerClass::ReuseStrategyAnonymousInnerClass()
{
}

shared_ptr<TokenStreamComponents>
Analyzer::ReuseStrategyAnonymousInnerClass::getReusableComponents(
    shared_ptr<Analyzer> analyzer, const wstring &fieldName)
{
  return std::static_pointer_cast<TokenStreamComponents>(
      getStoredValue(analyzer));
}

void Analyzer::ReuseStrategyAnonymousInnerClass::setReusableComponents(
    shared_ptr<Analyzer> analyzer, const wstring &fieldName,
    shared_ptr<TokenStreamComponents> components)
{
  setStoredValue(analyzer, components);
}

const shared_ptr<ReuseStrategy> Analyzer::PER_FIELD_REUSE_STRATEGY =
    make_shared<ReuseStrategyAnonymousInnerClass2>();

Analyzer::ReuseStrategyAnonymousInnerClass2::ReuseStrategyAnonymousInnerClass2()
{
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unchecked") @Override public
// TokenStreamComponents getReusableComponents(Analyzer analyzer, std::wstring
// fieldName)
shared_ptr<TokenStreamComponents>
Analyzer::ReuseStrategyAnonymousInnerClass2::getReusableComponents(
    shared_ptr<Analyzer> analyzer, const wstring &fieldName)
{
  unordered_map<wstring, std::shared_ptr<TokenStreamComponents>>
      componentsPerField = static_cast<
          unordered_map<wstring, std::shared_ptr<TokenStreamComponents>>>(
          getStoredValue(analyzer));
  return componentsPerField.size() > 0 ? componentsPerField[fieldName]
                                       : nullptr;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unchecked") @Override public void
// setReusableComponents(Analyzer analyzer, std::wstring fieldName,
// TokenStreamComponents components)
void Analyzer::ReuseStrategyAnonymousInnerClass2::setReusableComponents(
    shared_ptr<Analyzer> analyzer, const wstring &fieldName,
    shared_ptr<TokenStreamComponents> components)
{
  unordered_map<wstring, std::shared_ptr<TokenStreamComponents>>
      componentsPerField = static_cast<
          unordered_map<wstring, std::shared_ptr<TokenStreamComponents>>>(
          getStoredValue(analyzer));
  if (componentsPerField.empty()) {
    componentsPerField = unordered_map<>();
    setStoredValue(analyzer, componentsPerField);
  }
  componentsPerField.emplace(fieldName, components);
}

Analyzer::StringTokenStream::StringTokenStream(
    shared_ptr<AttributeFactory> attributeFactory, const wstring &value,
    int length)
    : TokenStream(attributeFactory), value(value), length(length)
{
}

void Analyzer::StringTokenStream::reset() { used = false; }

bool Analyzer::StringTokenStream::incrementToken()
{
  if (used) {
    return false;
  }
  clearAttributes();
  termAttribute->append(value);
  offsetAttribute->setOffset(0, length);
  used = true;
  return true;
}

void Analyzer::StringTokenStream::end() 
{
  TokenStream::end();
  offsetAttribute->setOffset(length, length);
}
} // namespace org::apache::lucene::analysis