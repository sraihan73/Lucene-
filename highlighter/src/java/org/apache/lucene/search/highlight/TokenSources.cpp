using namespace std;

#include "TokenSources.h"

namespace org::apache::lucene::search::highlight
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using LimitTokenOffsetFilter =
    org::apache::lucene::analysis::miscellaneous::LimitTokenOffsetFilter;
using Document = org::apache::lucene::document::Document;
using Fields = org::apache::lucene::index::Fields;
using IndexReader = org::apache::lucene::index::IndexReader;
using Terms = org::apache::lucene::index::Terms;

TokenSources::TokenSources() {}

shared_ptr<TokenStream>
TokenSources::getTokenStream(const wstring &field, shared_ptr<Fields> tvFields,
                             const wstring &text, shared_ptr<Analyzer> analyzer,
                             int maxStartOffset) 
{
  shared_ptr<TokenStream> tokenStream =
      getTermVectorTokenStreamOrNull(field, tvFields, maxStartOffset);
  if (tokenStream != nullptr) {
    return tokenStream;
  }
  tokenStream = analyzer->tokenStream(field, text);
  if (maxStartOffset >= 0 && maxStartOffset < text.length() - 1) {
    tokenStream =
        make_shared<LimitTokenOffsetFilter>(tokenStream, maxStartOffset);
  }
  return tokenStream;
}

shared_ptr<TokenStream> TokenSources::getTermVectorTokenStreamOrNull(
    const wstring &field, shared_ptr<Fields> tvFields,
    int maxStartOffset) 
{
  if (tvFields->empty()) {
    return nullptr;
  }
  shared_ptr<Terms> *const tvTerms = tvFields->terms(field);
  if (tvTerms == nullptr || !tvTerms->hasOffsets()) {
    return nullptr;
  }
  return make_shared<TokenStreamFromTermVector>(tvTerms, maxStartOffset);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static
// org.apache.lucene.analysis.TokenStream
// getAnyTokenStream(org.apache.lucene.index.IndexReader reader, int docId,
// std::wstring field, org.apache.lucene.document.Document document,
// org.apache.lucene.analysis.Analyzer analyzer) throws java.io.IOException
shared_ptr<TokenStream> TokenSources::getAnyTokenStream(
    shared_ptr<IndexReader> reader, int docId, const wstring &field,
    shared_ptr<Document> document,
    shared_ptr<Analyzer> analyzer) 
{
  shared_ptr<TokenStream> ts = nullptr;

  shared_ptr<Fields> vectors = reader->getTermVectors(docId);
  if (vectors->size() > 0) {
    shared_ptr<Terms> deque = vectors->terms(field);
    if (deque != nullptr) {
      ts = getTokenStream(deque);
    }
  }

  // No token info stored so fall back to analyzing raw content
  if (ts == nullptr) {
    ts = getTokenStream(document, field, analyzer);
  }
  return ts;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static
// org.apache.lucene.analysis.TokenStream
// getAnyTokenStream(org.apache.lucene.index.IndexReader reader, int docId,
// std::wstring field, org.apache.lucene.analysis.Analyzer analyzer) throws
// java.io.IOException
shared_ptr<TokenStream> TokenSources::getAnyTokenStream(
    shared_ptr<IndexReader> reader, int docId, const wstring &field,
    shared_ptr<Analyzer> analyzer) 
{
  shared_ptr<TokenStream> ts = nullptr;

  shared_ptr<Fields> vectors = reader->getTermVectors(docId);
  if (vectors->size() > 0) {
    shared_ptr<Terms> deque = vectors->terms(field);
    if (deque != nullptr) {
      ts = getTokenStream(deque);
    }
  }

  // No token info stored so fall back to analyzing raw content
  if (ts == nullptr) {
    ts = getTokenStream(reader, docId, field, analyzer);
  }
  return ts;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static
// org.apache.lucene.analysis.TokenStream
// getTokenStream(org.apache.lucene.index.Terms deque, bool
// tokenPositionsGuaranteedContiguous) throws java.io.IOException
shared_ptr<TokenStream> TokenSources::getTokenStream(
    shared_ptr<Terms> deque,
    bool tokenPositionsGuaranteedContiguous) 
{
  return getTokenStream(deque);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static
// org.apache.lucene.analysis.TokenStream getTokenStream(final
// org.apache.lucene.index.Terms tpv) throws java.io.IOException
shared_ptr<TokenStream>
TokenSources::getTokenStream(shared_ptr<Terms> tpv) 
{

  if (!tpv->hasOffsets()) {
    throw invalid_argument(
        L"Highlighting requires offsets from the TokenStream.");
    // TokenStreamFromTermVector can handle a lack of offsets if there are
    // positions. But
    // highlighters require offsets, so we insist here.
  }

  return make_shared<TokenStreamFromTermVector>(
      tpv, -1); // TODO propagate maxStartOffset; see LUCENE-6445
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static
// org.apache.lucene.analysis.TokenStream
// getTokenStreamWithOffsets(org.apache.lucene.index.IndexReader reader, int
// docId, std::wstring field) throws java.io.IOException
shared_ptr<TokenStream>
TokenSources::getTokenStreamWithOffsets(shared_ptr<IndexReader> reader,
                                        int docId,
                                        const wstring &field) 
{

  shared_ptr<Fields> vectors = reader->getTermVectors(docId);
  if (vectors->empty()) {
    return nullptr;
  }

  shared_ptr<Terms> deque = vectors->terms(field);
  if (deque == nullptr) {
    return nullptr;
  }

  if (!deque->hasOffsets()) {
    return nullptr;
  }

  return getTokenStream(deque);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static
// org.apache.lucene.analysis.TokenStream
// getTokenStream(org.apache.lucene.index.IndexReader reader, int docId, std::wstring
// field, org.apache.lucene.analysis.Analyzer analyzer) throws
// java.io.IOException
shared_ptr<TokenStream>
TokenSources::getTokenStream(shared_ptr<IndexReader> reader, int docId,
                             const wstring &field,
                             shared_ptr<Analyzer> analyzer) 
{
  shared_ptr<Document> doc = reader->document(docId);
  return getTokenStream(doc, field, analyzer);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static
// org.apache.lucene.analysis.TokenStream
// getTokenStream(org.apache.lucene.document.Document doc, std::wstring field,
// org.apache.lucene.analysis.Analyzer analyzer)
shared_ptr<TokenStream>
TokenSources::getTokenStream(shared_ptr<Document> doc, const wstring &field,
                             shared_ptr<Analyzer> analyzer)
{
  wstring contents = doc[field];
  if (contents == L"") {
    throw invalid_argument(
        L"Field " + field +
        L" in document is not stored and cannot be analyzed");
  }
  return getTokenStream(field, contents, analyzer);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static
// org.apache.lucene.analysis.TokenStream getTokenStream(std::wstring field, std::wstring
// contents, org.apache.lucene.analysis.Analyzer analyzer)
shared_ptr<TokenStream>
TokenSources::getTokenStream(const wstring &field, const wstring &contents,
                             shared_ptr<Analyzer> analyzer)
{
  return analyzer->tokenStream(field, contents);
}
} // namespace org::apache::lucene::search::highlight