using namespace std;

#include "BooleanPerceptronClassifier.h"
#include "../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/IndexableField.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/Term.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/BooleanClause.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/BooleanQuery.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/Query.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/WildcardQuery.h"
#include "../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../core/src/java/org/apache/lucene/util/IntsRefBuilder.h"
#include "../../../../../../../core/src/java/org/apache/lucene/util/fst/Builder.h"
#include "../../../../../../../core/src/java/org/apache/lucene/util/fst/PositiveIntOutputs.h"
#include "../../../../../../../core/src/java/org/apache/lucene/util/fst/Util.h"
#include "ClassificationResult.h"

namespace org::apache::lucene::classification
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexableField = org::apache::lucene::index::IndexableField;
using MultiFields = org::apache::lucene::index::MultiFields;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using WildcardQuery = org::apache::lucene::search::WildcardQuery;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using Builder = org::apache::lucene::util::fst::Builder;
using FST = org::apache::lucene::util::fst::FST;
using PositiveIntOutputs = org::apache::lucene::util::fst::PositiveIntOutputs;
using Util = org::apache::lucene::util::fst::Util;

BooleanPerceptronClassifier::BooleanPerceptronClassifier(
    shared_ptr<IndexReader> indexReader, shared_ptr<Analyzer> analyzer,
    shared_ptr<Query> query, optional<int> &batchSize, optional<double> &bias,
    const wstring &classFieldName,
    const wstring &textFieldName) 
    : textTerms(MultiFields::getTerms(indexReader, textFieldName)),
      analyzer(analyzer), textFieldName(textFieldName)
{

  if (textTerms == nullptr) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"term vectors need to be available for field " +
                              textFieldName);
  }

  if (!bias || bias == 0) {
    // automatic assign the bias to be the average total term freq
    double t =
        static_cast<double>(indexReader->getSumTotalTermFreq(textFieldName)) /
        static_cast<double>(indexReader->getDocCount(textFieldName));
    if (t != -1) {
      this->bias = t;
    } else {
      // C++ TODO: The following line could not be converted:
      throw java.io.IOException(
          L"bias cannot be assigned since term vectors for field " +
          textFieldName + L" do not exist");
    }
  } else {
    this->bias = bias;
  }

  // TODO : remove this map_obj as soon as we have a writable FST
  shared_ptr<SortedMap<wstring, double>> weights =
      make_shared<ConcurrentSkipListMap<wstring, double>>();

  shared_ptr<TermsEnum> termsEnum = textTerms->begin();
  shared_ptr<BytesRef> textTerm;
  while ((textTerm = termsEnum->next()) != nullptr) {
    weights->put(textTerm->utf8ToString(),
                 static_cast<double>(termsEnum->totalTermFreq()));
  }
  updateFST(weights);

  shared_ptr<IndexSearcher> indexSearcher =
      make_shared<IndexSearcher>(indexReader);

  int batchCount = 0;

  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(make_shared<BooleanClause>(
      make_shared<WildcardQuery>(make_shared<Term>(classFieldName, L"*")),
      BooleanClause::Occur::MUST));
  if (query != nullptr) {
    q->add(make_shared<BooleanClause>(query, BooleanClause::Occur::MUST));
  }
  // run the search and use stored field values
  for (auto scoreDoc :
       indexSearcher->search(q->build(), numeric_limits<int>::max())
           ->scoreDocs) {
    shared_ptr<Document> doc = indexSearcher->doc(scoreDoc->doc);

    shared_ptr<IndexableField> textField = doc->getField(textFieldName);

    // get the expected result
    shared_ptr<IndexableField> classField = doc->getField(classFieldName);

    if (textField != nullptr && classField != nullptr) {
      // assign class to the doc
      shared_ptr<ClassificationResult<bool>> classificationResult =
          assignClass(textField->stringValue());
      optional<bool> assignedClass = classificationResult->getAssignedClass();

      optional<bool> correctClass =
          StringHelper::fromString<bool>(classField->stringValue());
      int64_t modifier = correctClass.compareTo(assignedClass);
      if (modifier != 0) {
        updateWeights(indexReader, scoreDoc->doc, assignedClass, weights,
                      modifier, batchCount % batchSize == 0);
      }
      batchCount++;
    }
  }
  weights->clear(); // free memory while waiting for GC
}

void BooleanPerceptronClassifier::updateWeights(
    shared_ptr<IndexReader> indexReader, int docId,
    optional<bool> &assignedClass,
    shared_ptr<SortedMap<wstring, double>> weights, double modifier,
    bool updateFST) 
{
  shared_ptr<TermsEnum> cte = textTerms->begin();

  // get the doc term vectors
  shared_ptr<Terms> terms = indexReader->getTermVector(docId, textFieldName);

  if (terms == nullptr) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"term vectors must be stored for field " +
                              textFieldName);
  }

  shared_ptr<TermsEnum> termsEnum = terms->begin();

  shared_ptr<BytesRef> term;

  while ((term = termsEnum->next()) != nullptr) {
    cte->seekExact(term);
    if (assignedClass) {
      int64_t termFreqLocal = termsEnum->totalTermFreq();
      // update weights
      optional<int64_t> previousValue = Util::get(fst, term);
      wstring termString = term->utf8ToString();
      weights->put(termString,
                   !previousValue
                       ? 0
                       : max(0, previousValue + modifier * termFreqLocal));
    }
  }
  if (updateFST) {
    this->updateFST(weights);
  }
}

void BooleanPerceptronClassifier::updateFST(
    shared_ptr<SortedMap<wstring, double>> weights) 
{
  shared_ptr<PositiveIntOutputs> outputs = PositiveIntOutputs::getSingleton();
  shared_ptr<Builder<int64_t>> fstBuilder =
      make_shared<Builder<int64_t>>(FST::INPUT_TYPE::BYTE1, outputs);
  shared_ptr<BytesRefBuilder> scratchBytes = make_shared<BytesRefBuilder>();
  shared_ptr<IntsRefBuilder> scratchInts = make_shared<IntsRefBuilder>();
  for (auto entry : weights) {
    scratchBytes->copyChars(entry.first);
    fstBuilder->add(Util::toIntsRef(scratchBytes->get(), scratchInts),
                    entry.second::longValue());
  }
  fst = fstBuilder->finish();
}

shared_ptr<ClassificationResult<bool>>
BooleanPerceptronClassifier::assignClass(const wstring &text) 
{
  optional<int64_t> output = 0LL;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream
  // tokenStream = analyzer.tokenStream(textFieldName, text))
  {
    org::apache::lucene::analysis::TokenStream tokenStream =
        analyzer->tokenStream(textFieldName, text);
    shared_ptr<CharTermAttribute> charTermAttribute =
        tokenStream->addAttribute(CharTermAttribute::typeid);
    tokenStream->reset();
    while (tokenStream->incrementToken()) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring s = charTermAttribute->toString();
      optional<int64_t> d = Util::get(fst, make_shared<BytesRef>(s));
      if (d) {
        output += d;
      }
    }
    tokenStream->end();
  }

  double score = 1 - exp(-1 * abs(bias - output.value()) / bias);
  return make_shared<ClassificationResult<bool>>(output >= bias, score);
}

deque<ClassificationResult<bool>>
BooleanPerceptronClassifier::getClasses(const wstring &text) 
{
  return nullptr;
}

deque<ClassificationResult<bool>>
BooleanPerceptronClassifier::getClasses(const wstring &text,
                                        int max) 
{
  return nullptr;
}
} // namespace org::apache::lucene::classification