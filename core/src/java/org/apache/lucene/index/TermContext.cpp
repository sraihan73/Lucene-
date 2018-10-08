using namespace std;

#include "TermContext.h"

namespace org::apache::lucene::index
{
using BytesRef = org::apache::lucene::util::BytesRef;

TermContext::TermContext(shared_ptr<IndexReaderContext> context)
    : topReaderContextIdentity(context->identity),
      states(std::deque<std::shared_ptr<TermState>>(len))
{
  assert(context != nullptr && context->isTopLevel);
  docFreq_ = 0;
  totalTermFreq_ = 0;
  constexpr int len;
  if (context->leaves().empty()) {
    len = 1;
  } else {
    len = context->leaves().size();
  }
}

bool TermContext::wasBuiltFor(shared_ptr<IndexReaderContext> context)
{
  return topReaderContextIdentity == context->identity;
}

TermContext::TermContext(shared_ptr<IndexReaderContext> context,
                         shared_ptr<TermState> state, int ord, int docFreq,
                         int64_t totalTermFreq)
    : TermContext(context)
{
  register_(state, ord, docFreq, totalTermFreq);
}

shared_ptr<TermContext>
TermContext::build(shared_ptr<IndexReaderContext> context,
                   shared_ptr<Term> term) 
{
  assert(context != nullptr && context->isTopLevel);
  const wstring field = term->field();
  shared_ptr<BytesRef> *const bytes = term->bytes();
  shared_ptr<TermContext> *const perReaderTermState =
      make_shared<TermContext>(context);
  // if (DEBUG) System.out.println("prts.build term=" + term);
  for (auto ctx : context->leaves()) {
    // if (DEBUG) System.out.println("  r=" + leaves[i].reader);
    shared_ptr<Terms> *const terms = ctx->reader()->terms(field);
    if (terms != nullptr) {
      shared_ptr<TermsEnum> *const termsEnum = terms->begin();
      if (termsEnum->seekExact(bytes)) {
        shared_ptr<TermState> *const termState = termsEnum->termState();
        // if (DEBUG) System.out.println("    found");
        perReaderTermState->register_(termState, ctx->ord, termsEnum->docFreq(),
                                      termsEnum->totalTermFreq());
      }
    }
  }
  return perReaderTermState;
}

void TermContext::clear()
{
  docFreq_ = 0;
  totalTermFreq_ = 0;
  Arrays::fill(states, nullptr);
}

void TermContext::register_(shared_ptr<TermState> state, int const ord,
                            int const docFreq, int64_t const totalTermFreq)
{
  register_(state, ord);
  accumulateStatistics(docFreq, totalTermFreq);
}

void TermContext::register_(shared_ptr<TermState> state, int const ord)
{
  assert((state != nullptr, L"state must not be null"));
  assert(ord >= 0 && ord < states.size());
  assert((states[ord] == nullptr,
          L"state for ord: " + to_wstring(ord) + L" already registered"));
  states[ord] = state;
}

void TermContext::accumulateStatistics(int const docFreq,
                                       int64_t const totalTermFreq)
{
  this->docFreq_ += docFreq;
  if (this->totalTermFreq_ >= 0 && totalTermFreq >= 0) {
    this->totalTermFreq_ += totalTermFreq;
  } else {
    this->totalTermFreq_ = -1;
  }
}

shared_ptr<TermState> TermContext::get(int ord)
{
  assert(ord >= 0 && ord < states.size());
  return states[ord];
}

int TermContext::docFreq() { return docFreq_; }

int64_t TermContext::totalTermFreq() { return totalTermFreq_; }

wstring TermContext::toString()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"TermContext\n");
  for (auto termState : states) {
    sb->append(L"  state=");
    sb->append(termState);
    sb->append(L'\n');
  }

  return sb->toString();
}
} // namespace org::apache::lucene::index