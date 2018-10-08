using namespace std;

#include "FastVectorHighlighter.h"

namespace org::apache::lucene::search::vectorhighlight
{
using IndexReader = org::apache::lucene::index::IndexReader;
using Query = org::apache::lucene::search::Query;
using Encoder = org::apache::lucene::search::highlight::Encoder;

FastVectorHighlighter::FastVectorHighlighter()
    : FastVectorHighlighter(DEFAULT_PHRASE_HIGHLIGHT, DEFAULT_FIELD_MATCH)
{
}

FastVectorHighlighter::FastVectorHighlighter(bool phraseHighlight,
                                             bool fieldMatch)
    : FastVectorHighlighter(phraseHighlight, fieldMatch,
                            new SimpleFragListBuilder(),
                            new ScoreOrderFragmentsBuilder())
{
}

FastVectorHighlighter::FastVectorHighlighter(
    bool phraseHighlight, bool fieldMatch,
    shared_ptr<FragListBuilder> fragListBuilder,
    shared_ptr<FragmentsBuilder> fragmentsBuilder)
    : phraseHighlight(phraseHighlight), fieldMatch(fieldMatch),
      fragListBuilder(fragListBuilder), fragmentsBuilder(fragmentsBuilder)
{
}

shared_ptr<FieldQuery>
FastVectorHighlighter::getFieldQuery(shared_ptr<Query> query)
{
  // TODO: should we deprecate this?
  // because if there is no reader, then we cannot rewrite MTQ.
  try {
    return make_shared<FieldQuery>(query, nullptr, phraseHighlight, fieldMatch);
  } catch (const IOException &e) {
    // should never be thrown when reader is null
    throw runtime_error(e);
  }
}

shared_ptr<FieldQuery> FastVectorHighlighter::getFieldQuery(
    shared_ptr<Query> query, shared_ptr<IndexReader> reader) 
{
  return make_shared<FieldQuery>(query, reader, phraseHighlight, fieldMatch);
}

wstring FastVectorHighlighter::getBestFragment(
    shared_ptr<FieldQuery> fieldQuery, shared_ptr<IndexReader> reader,
    int docId, const wstring &fieldName, int fragCharSize) 
{
  shared_ptr<FieldFragList> fieldFragList = getFieldFragList(
      fragListBuilder, fieldQuery, reader, docId, fieldName, fragCharSize);
  return fragmentsBuilder->createFragment(reader, docId, fieldName,
                                          fieldFragList);
}

std::deque<wstring> FastVectorHighlighter::getBestFragments(
    shared_ptr<FieldQuery> fieldQuery, shared_ptr<IndexReader> reader,
    int docId, const wstring &fieldName, int fragCharSize,
    int maxNumFragments) 
{
  shared_ptr<FieldFragList> fieldFragList = getFieldFragList(
      fragListBuilder, fieldQuery, reader, docId, fieldName, fragCharSize);
  return fragmentsBuilder->createFragments(reader, docId, fieldName,
                                           fieldFragList, maxNumFragments);
}

wstring FastVectorHighlighter::getBestFragment(
    shared_ptr<FieldQuery> fieldQuery, shared_ptr<IndexReader> reader,
    int docId, const wstring &fieldName, int fragCharSize,
    shared_ptr<FragListBuilder> fragListBuilder,
    shared_ptr<FragmentsBuilder> fragmentsBuilder,
    std::deque<wstring> &preTags, std::deque<wstring> &postTags,
    shared_ptr<Encoder> encoder) 
{
  shared_ptr<FieldFragList> fieldFragList = getFieldFragList(
      fragListBuilder, fieldQuery, reader, docId, fieldName, fragCharSize);
  return fragmentsBuilder->createFragment(
      reader, docId, fieldName, fieldFragList, preTags, postTags, encoder);
}

std::deque<wstring> FastVectorHighlighter::getBestFragments(
    shared_ptr<FieldQuery> fieldQuery, shared_ptr<IndexReader> reader,
    int docId, const wstring &fieldName, int fragCharSize, int maxNumFragments,
    shared_ptr<FragListBuilder> fragListBuilder,
    shared_ptr<FragmentsBuilder> fragmentsBuilder,
    std::deque<wstring> &preTags, std::deque<wstring> &postTags,
    shared_ptr<Encoder> encoder) 
{
  shared_ptr<FieldFragList> fieldFragList = getFieldFragList(
      fragListBuilder, fieldQuery, reader, docId, fieldName, fragCharSize);
  return fragmentsBuilder->createFragments(reader, docId, fieldName,
                                           fieldFragList, maxNumFragments,
                                           preTags, postTags, encoder);
}

std::deque<wstring> FastVectorHighlighter::getBestFragments(
    shared_ptr<FieldQuery> fieldQuery, shared_ptr<IndexReader> reader,
    int docId, const wstring &storedField,
    shared_ptr<Set<wstring>> matchedFields, int fragCharSize,
    int maxNumFragments, shared_ptr<FragListBuilder> fragListBuilder,
    shared_ptr<FragmentsBuilder> fragmentsBuilder,
    std::deque<wstring> &preTags, std::deque<wstring> &postTags,
    shared_ptr<Encoder> encoder) 
{
  shared_ptr<FieldFragList> fieldFragList = getFieldFragList(
      fragListBuilder, fieldQuery, reader, docId, matchedFields, fragCharSize);
  return fragmentsBuilder->createFragments(reader, docId, storedField,
                                           fieldFragList, maxNumFragments,
                                           preTags, postTags, encoder);
}

shared_ptr<FieldFragList> FastVectorHighlighter::getFieldFragList(
    shared_ptr<FragListBuilder> fragListBuilder,
    shared_ptr<FieldQuery> fieldQuery, shared_ptr<IndexReader> reader,
    int docId, const wstring &matchedField, int fragCharSize) 
{
  shared_ptr<FieldTermStack> fieldTermStack =
      make_shared<FieldTermStack>(reader, docId, matchedField, fieldQuery);
  shared_ptr<FieldPhraseList> fieldPhraseList =
      make_shared<FieldPhraseList>(fieldTermStack, fieldQuery, phraseLimit);
  return fragListBuilder->createFieldFragList(fieldPhraseList, fragCharSize);
}

shared_ptr<FieldFragList> FastVectorHighlighter::getFieldFragList(
    shared_ptr<FragListBuilder> fragListBuilder,
    shared_ptr<FieldQuery> fieldQuery, shared_ptr<IndexReader> reader,
    int docId, shared_ptr<Set<wstring>> matchedFields,
    int fragCharSize) 
{
  Set<wstring>::const_iterator matchedFieldsItr = matchedFields->begin();
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  if (!matchedFieldsItr->hasNext()) {
    throw invalid_argument(
        L"matchedFields must contain at least on field name.");
  }
  std::deque<std::shared_ptr<FieldPhraseList>> toMerge(matchedFields->size());
  int i = 0;
  while (matchedFieldsItr != matchedFields->end()) {
    shared_ptr<FieldTermStack> stack = make_shared<FieldTermStack>(
        reader, docId, *matchedFieldsItr, fieldQuery);
    toMerge[i++] = make_shared<FieldPhraseList>(stack, fieldQuery, phraseLimit);
    matchedFieldsItr++;
  }
  return fragListBuilder->createFieldFragList(
      make_shared<FieldPhraseList>(toMerge), fragCharSize);
}

bool FastVectorHighlighter::isPhraseHighlight() { return phraseHighlight; }

bool FastVectorHighlighter::isFieldMatch() { return fieldMatch; }

int FastVectorHighlighter::getPhraseLimit() { return phraseLimit; }

void FastVectorHighlighter::setPhraseLimit(int phraseLimit)
{
  this->phraseLimit = phraseLimit;
}
} // namespace org::apache::lucene::search::vectorhighlight