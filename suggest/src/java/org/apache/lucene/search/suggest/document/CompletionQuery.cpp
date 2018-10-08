using namespace std;

#include "CompletionQuery.h"

namespace org::apache::lucene::search::suggest::document
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using Query = org::apache::lucene::search::Query;
using BitsProducer = org::apache::lucene::search::suggest::BitsProducer;
//    import static
//    org.apache.lucene.search.suggest.document.CompletionAnalyzer.HOLE_CHARACTER;
//    import static
//    org.apache.lucene.analysis.miscellaneous.ConcatenateGraphFilter.SEP_LABEL;

CompletionQuery::CompletionQuery(shared_ptr<Term> term,
                                 shared_ptr<BitsProducer> filter)
    : term(term), filter(filter)
{
  validate(term->text());
}

shared_ptr<BitsProducer> CompletionQuery::getFilter() { return filter; }

wstring CompletionQuery::getField() { return term->field(); }

shared_ptr<Term> CompletionQuery::getTerm() { return term; }

shared_ptr<Query>
CompletionQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  char type = 0;
  bool first = true;
  shared_ptr<Terms> terms;
  for (auto context : reader->leaves()) {
    shared_ptr<LeafReader> leafReader = context->reader();
    try {
      if ((terms = leafReader->terms(getField())) == nullptr) {
        continue;
      }
    } catch (const IOException &e) {
      continue;
    }
    if (std::dynamic_pointer_cast<CompletionTerms>(terms) != nullptr) {
      shared_ptr<CompletionTerms> completionTerms =
          std::static_pointer_cast<CompletionTerms>(terms);
      char t = completionTerms->getType();
      if (first) {
        type = t;
        first = false;
      } else if (type != t) {
        throw make_shared<IllegalStateException>(
            getField() + L" has values of multiple types");
      }
    }
  }

  if (first == false) {
    if (std::dynamic_pointer_cast<ContextQuery>(shared_from_this()) !=
        nullptr) {
      if (type == SuggestField::TYPE) {
        throw make_shared<IllegalStateException>(
            this->getClass().getSimpleName() +
            L" can not be executed against a non context-enabled "
            L"SuggestField: " +
            getField());
      }
    } else {
      if (type == ContextSuggestField::TYPE) {
        return make_shared<ContextQuery>(shared_from_this());
      }
    }
  }
  return Query::rewrite(reader);
}

wstring CompletionQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  if (term->field() != field) {
    buffer->append(term->field());
    buffer->append(L":");
  }
  buffer->append(term->text());
  buffer->append(L'*');
  if (filter != nullptr) {
    buffer->append(L",");
    buffer->append(L"filter");
    buffer->append(L":");
    // C++ TODO: There is no native C++ equivalent to 'toString':
    buffer->append(filter->toString());
  }
  return buffer->toString();
}

void CompletionQuery::validate(const wstring &termText)
{
  for (int i = 0; i < termText.length(); i++) {
    switch (termText[i]) {
    case HOLE_CHARACTER:
      throw invalid_argument(L"Term text cannot contain HOLE character U+001E; "
                             L"this character is reserved");
    case SEP_LABEL:
      throw invalid_argument(L"Term text cannot contain unit separator "
                             L"character U+001F; this character is reserved");
    default:
      break;
    }
  }
}
} // namespace org::apache::lucene::search::suggest::document