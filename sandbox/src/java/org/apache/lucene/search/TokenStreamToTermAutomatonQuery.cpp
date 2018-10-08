using namespace std;

#include "TokenStreamToTermAutomatonQuery.h"

namespace org::apache::lucene::search
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using TermToBytesRefAttribute =
    org::apache::lucene::analysis::tokenattributes::TermToBytesRefAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;

TokenStreamToTermAutomatonQuery::TokenStreamToTermAutomatonQuery()
{
  this->preservePositionIncrements = true;
}

void TokenStreamToTermAutomatonQuery::setPreservePositionIncrements(
    bool enablePositionIncrements)
{
  this->preservePositionIncrements = enablePositionIncrements;
}

shared_ptr<TermAutomatonQuery> TokenStreamToTermAutomatonQuery::toQuery(
    const wstring &field, shared_ptr<TokenStream> in_) 
{

  shared_ptr<TermToBytesRefAttribute> *const termBytesAtt =
      in_->addAttribute(TermToBytesRefAttribute::typeid);
  shared_ptr<PositionIncrementAttribute> *const posIncAtt =
      in_->addAttribute(PositionIncrementAttribute::typeid);
  shared_ptr<PositionLengthAttribute> *const posLengthAtt =
      in_->addAttribute(PositionLengthAttribute::typeid);
  shared_ptr<OffsetAttribute> *const offsetAtt =
      in_->addAttribute(OffsetAttribute::typeid);

  in_->reset();

  shared_ptr<TermAutomatonQuery> query = make_shared<TermAutomatonQuery>(field);

  int pos = -1;
  int lastPos = 0;
  int maxOffset = 0;
  int maxPos = -1;
  int state = -1;
  while (in_->incrementToken()) {
    int posInc = posIncAtt->getPositionIncrement();
    if (preservePositionIncrements == false && posInc > 1) {
      posInc = 1;
    }
    assert(pos > -1 || posInc > 0);

    if (posInc > 1) {
      throw invalid_argument(
          L"cannot handle holes; to accept any term, use '*' term");
    }

    if (posInc > 0) {
      // New node:
      pos += posInc;
    }

    int endPos = pos + posLengthAtt->getPositionLength();
    while (state < endPos) {
      state = query->createState();
    }

    shared_ptr<BytesRef> term = termBytesAtt->getBytesRef();
    // System.out.println(pos + "-" + endPos + ": " + term.utf8ToString() + ":
    // posInc=" + posInc);
    if (term->length == 1 &&
        term->bytes[term->offset] == static_cast<char>(L'*')) {
      query->addAnyTransition(pos, endPos);
    } else {
      query->addTransition(pos, endPos, term);
    }

    maxOffset = max(maxOffset, offsetAtt->endOffset());
    maxPos = max(maxPos, endPos);
  }

  in_->end();

  // TODO: look at endOffset?  ts2a did...

  // TODO: this (setting "last" state as the only accept state) may be too
  // simplistic?
  query->setAccept(state, true);
  query->finish();

  return query;
}
} // namespace org::apache::lucene::search