using namespace std;

#include "FilteredTermsEnum.h"

namespace org::apache::lucene::index
{
using BytesRef = org::apache::lucene::util::BytesRef;
using AttributeSource = org::apache::lucene::util::AttributeSource;

FilteredTermsEnum::FilteredTermsEnum(shared_ptr<TermsEnum> tenum)
    : FilteredTermsEnum(tenum, true)
{
}

FilteredTermsEnum::FilteredTermsEnum(shared_ptr<TermsEnum> tenum,
                                     bool const startWithSeek)
    : tenum(tenum)
{
  assert(tenum != nullptr);
  doSeek = startWithSeek;
}

void FilteredTermsEnum::setInitialSeekTerm(shared_ptr<BytesRef> term)
{
  this->initialSeekTerm = term;
}

shared_ptr<BytesRef> FilteredTermsEnum::nextSeekTerm(
    shared_ptr<BytesRef> currentTerm) 
{
  shared_ptr<BytesRef> *const t = initialSeekTerm;
  initialSeekTerm.reset();
  return t;
}

shared_ptr<AttributeSource> FilteredTermsEnum::attributes()
{
  return tenum->attributes();
}

shared_ptr<BytesRef> FilteredTermsEnum::term() 
{
  return tenum->term();
}

int FilteredTermsEnum::docFreq()  { return tenum->docFreq(); }

int64_t FilteredTermsEnum::totalTermFreq() 
{
  return tenum->totalTermFreq();
}

bool FilteredTermsEnum::seekExact(shared_ptr<BytesRef> term) 
{
  throw make_shared<UnsupportedOperationException>(
      getClassName() + L" does not support seeking");
}

SeekStatus
FilteredTermsEnum::seekCeil(shared_ptr<BytesRef> term) 
{
  throw make_shared<UnsupportedOperationException>(
      getClassName() + L" does not support seeking");
}

void FilteredTermsEnum::seekExact(int64_t ord) 
{
  throw make_shared<UnsupportedOperationException>(
      getClassName() + L" does not support seeking");
}

int64_t FilteredTermsEnum::ord()  { return tenum->ord(); }

shared_ptr<PostingsEnum>
FilteredTermsEnum::postings(shared_ptr<PostingsEnum> reuse,
                            int flags) 
{
  return tenum->postings(reuse, flags);
}

void FilteredTermsEnum::seekExact(
    shared_ptr<BytesRef> term, shared_ptr<TermState> state) 
{
  throw make_shared<UnsupportedOperationException>(
      getClassName() + L" does not support seeking");
}

shared_ptr<TermState> FilteredTermsEnum::termState() 
{
  assert(tenum != nullptr);
  return tenum->termState();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("fallthrough") @Override public
// org.apache.lucene.util.BytesRef next() throws java.io.IOException
shared_ptr<BytesRef> FilteredTermsEnum::next() 
{
  // System.out.println("FTE.next doSeek=" + doSeek);
  // new Throwable().printStackTrace(System.out);
  for (;;) {
    // Seek or forward the iterator
    if (doSeek) {
      doSeek = false;
      shared_ptr<BytesRef> *const t = nextSeekTerm(actualTerm);
      // System.out.println("  seek to t=" + (t == null ? "null" :
      // t.utf8ToString()) + " tenum=" + tenum);
      // Make sure we always seek forward:
      assert((actualTerm == nullptr || t == nullptr ||
                  t->compareTo(actualTerm) > 0,
              L"curTerm=" + actualTerm + L" seekTerm=" + t));
      if (t == nullptr || tenum->seekCeil(t) == SeekStatus::END) {
        // no more terms to seek to or enum exhausted
        // System.out.println("  return null");
        return nullptr;
      }
      actualTerm = tenum->term();
      // System.out.println("  got term=" + actualTerm.utf8ToString());
    } else {
      actualTerm = tenum->next();
      if (actualTerm == nullptr) {
        // enum exhausted
        return nullptr;
      }
    }

    // check if term is accepted
    switch (accept(actualTerm)) {
    case YES_AND_SEEK:
      doSeek = true;
      // term accepted, but we need to seek so fall-through
    case YES:
      // term accepted
      return actualTerm;
    case NO_AND_SEEK:
      // invalid term, seek next time
      doSeek = true;
      break;
    case END:
      // we are supposed to end the enum
      return nullptr;
    case NO:
      // we just iterate again
      break;
    }
  }
}
} // namespace org::apache::lucene::index