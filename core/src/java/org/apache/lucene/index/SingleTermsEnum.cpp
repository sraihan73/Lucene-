using namespace std;

#include "SingleTermsEnum.h"

namespace org::apache::lucene::index
{
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;
using BytesRef = org::apache::lucene::util::BytesRef;

SingleTermsEnum::SingleTermsEnum(shared_ptr<TermsEnum> tenum,
                                 shared_ptr<BytesRef> termText)
    : FilteredTermsEnum(tenum), singleRef(termText)
{
  setInitialSeekTerm(termText);
}

AcceptStatus SingleTermsEnum::accept(shared_ptr<BytesRef> term)
{
  return term->equals(singleRef) ? AcceptStatus::YES : AcceptStatus::END;
}
} // namespace org::apache::lucene::index