using namespace std;

#include "Tokenizer.h"
#include "../util/AttributeFactory.h"
#include "CharFilter.h"

namespace org::apache::lucene::analysis
{
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using AttributeSource = org::apache::lucene::util::AttributeSource;

Tokenizer::Tokenizer()
{
  //
}

Tokenizer::Tokenizer(shared_ptr<AttributeFactory> factory)
    : TokenStream(factory)
{
}

Tokenizer::~Tokenizer()
{
  input->close();
  // LUCENE-2387: don't hold onto Reader after close, so
  // GC can reclaim
  inputPending = input = ILLEGAL_STATE_READER;
}

int Tokenizer::correctOffset(int currentOff)
{
  return (std::dynamic_pointer_cast<CharFilter>(input) != nullptr)
             ? (std::static_pointer_cast<CharFilter>(input))
                   ->correctOffset(currentOff)
             : currentOff;
}

void Tokenizer::setReader(shared_ptr<Reader> input)
{
  if (input == nullptr) {
    throw make_shared<NullPointerException>(L"input must not be null");
  } else if (this->input != ILLEGAL_STATE_READER) {
    throw make_shared<IllegalStateException>(
        L"TokenStream contract violation: close() call missing");
  }
  this->inputPending = input;
  setReaderTestPoint();
}

void Tokenizer::reset() 
{
  TokenStream::reset();
  input = inputPending;
  inputPending = ILLEGAL_STATE_READER;
}

void Tokenizer::setReaderTestPoint() {}

const shared_ptr<java::io::Reader> Tokenizer::ILLEGAL_STATE_READER =
    make_shared<ReaderAnonymousInnerClass>();

Tokenizer::ReaderAnonymousInnerClass::ReaderAnonymousInnerClass() {}

int Tokenizer::ReaderAnonymousInnerClass::read(std::deque<wchar_t> &cbuf,
                                               int off, int len)
{
  throw make_shared<IllegalStateException>(
      wstring(
          L"TokenStream contract violation: reset()/close() call missing, ") +
      L"reset() called multiple times, or subclass does not call "
      L"super.reset(). " +
      L"Please see Javadocs of TokenStream class for more information about "
      L"the correct consuming workflow.");
}

Tokenizer::ReaderAnonymousInnerClass::~ReaderAnonymousInnerClass() {}
} // namespace org::apache::lucene::analysis