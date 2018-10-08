using namespace std;

#include "DocHelper.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using StoredField = org::apache::lucene::document::StoredField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
const shared_ptr<org::apache::lucene::document::FieldType>
    DocHelper::customType;
const wstring DocHelper::FIELD_1_TEXT = L"field one text";
const wstring DocHelper::TEXT_FIELD_1_KEY = L"textField1";
shared_ptr<org::apache::lucene::document::Field> DocHelper::textField1;

DocHelper::StaticConstructor::StaticConstructor()
{
  customType = make_shared<FieldType>(TextField::TYPE_STORED);
  textField1 = make_shared<Field>(TEXT_FIELD_1_KEY, FIELD_1_TEXT, customType);
  customType2 = make_shared<FieldType>(TextField::TYPE_STORED);
  customType2->setStoreTermVectors(true);
  customType2->setStoreTermVectorPositions(true);
  customType2->setStoreTermVectorOffsets(true);
  textField2 = make_shared<Field>(TEXT_FIELD_2_KEY, FIELD_2_TEXT, customType2);
  customType3 = make_shared<FieldType>(TextField::TYPE_STORED);
  customType3->setOmitNorms(true);
  textField3 = make_shared<Field>(TEXT_FIELD_3_KEY, FIELD_3_TEXT, customType3);
  keyField = make_shared<StringField>(KEYWORD_FIELD_KEY, KEYWORD_TEXT,
                                      Field::Store::YES);
  customType5 = make_shared<FieldType>(TextField::TYPE_STORED);
  customType5->setOmitNorms(true);
  customType5->setTokenized(false);
  noNormsField = make_shared<Field>(NO_NORMS_KEY, NO_NORMS_TEXT, customType5);
  customType6 = make_shared<FieldType>(TextField::TYPE_STORED);
  customType6->setIndexOptions(IndexOptions::DOCS);
  noTFField = make_shared<Field>(NO_TF_KEY, NO_TF_TEXT, customType6);
  customType7 = make_shared<FieldType>();
  customType7->setStored(true);
  unIndField = make_shared<Field>(UNINDEXED_FIELD_KEY, UNINDEXED_FIELD_TEXT,
                                  customType7);
  customType8 = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType8->setStoreTermVectors(true);
  unStoredField2 = make_shared<Field>(UNSTORED_FIELD_2_KEY,
                                      UNSTORED_2_FIELD_TEXT, customType8);
  // Initialize the large Lazy Field
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  for (int i = 0; i < 10000; i++) {
    buffer->append(L"Lazily loading lengths of language in lieu of laughing ");
  }

  LAZY_FIELD_BINARY_BYTES = (wstring(L"These are some binary field bytes"))
                                .getBytes(StandardCharsets::UTF_8);
  lazyFieldBinary =
      make_shared<StoredField>(LAZY_FIELD_BINARY_KEY, LAZY_FIELD_BINARY_BYTES);
  fields[fields.size() - 2] = lazyFieldBinary;
  LARGE_LAZY_FIELD_TEXT = buffer->toString();
  largeLazyField = make_shared<Field>(LARGE_LAZY_FIELD_KEY,
                                      LARGE_LAZY_FIELD_TEXT, customType);
  fields[fields.size() - 1] = largeLazyField;
  for (int i = 0; i < fields.size(); i++) {
    shared_ptr<IndexableField> f = fields[i];
    add(all, f);
    if (f->fieldType()->indexOptions() != IndexOptions::NONE) {
      add(indexed, f);
    } else {
      add(unindexed, f);
    }
    if (f->fieldType()->storeTermVectors()) {
      add(termvector, f);
    }
    if (f->fieldType()->indexOptions() != IndexOptions::NONE &&
        !f->fieldType()->storeTermVectors()) {
      add(notermvector, f);
    }
    if (f->fieldType()->stored()) {
      add(stored, f);
    } else {
      add(unstored, f);
    }
    if (f->fieldType()->indexOptions() == IndexOptions::DOCS) {
      add(noTf, f);
    }
    if (f->fieldType()->omitNorms()) {
      add(noNorms, f);
    }
    if (f->fieldType()->indexOptions() == IndexOptions::DOCS) {
      add(noTf, f);
    }
    // if (f.isLazy()) add(lazy, f);
  }
  nameValues = unordered_map<>();
  nameValues.emplace(TEXT_FIELD_1_KEY, FIELD_1_TEXT);
  nameValues.emplace(TEXT_FIELD_2_KEY, FIELD_2_TEXT);
  nameValues.emplace(TEXT_FIELD_3_KEY, FIELD_3_TEXT);
  nameValues.emplace(KEYWORD_FIELD_KEY, KEYWORD_TEXT);
  nameValues.emplace(NO_NORMS_KEY, NO_NORMS_TEXT);
  nameValues.emplace(NO_TF_KEY, NO_TF_TEXT);
  nameValues.emplace(UNINDEXED_FIELD_KEY, UNINDEXED_FIELD_TEXT);
  nameValues.emplace(UNSTORED_FIELD_1_KEY, UNSTORED_1_FIELD_TEXT);
  nameValues.emplace(UNSTORED_FIELD_2_KEY, UNSTORED_2_FIELD_TEXT);
  nameValues.emplace(LAZY_FIELD_KEY, LAZY_FIELD_TEXT);
  nameValues.emplace(LAZY_FIELD_BINARY_KEY, LAZY_FIELD_BINARY_BYTES);
  nameValues.emplace(LARGE_LAZY_FIELD_KEY, LARGE_LAZY_FIELD_TEXT);
  nameValues.emplace(TEXT_FIELD_UTF1_KEY, FIELD_UTF1_TEXT);
  nameValues.emplace(TEXT_FIELD_UTF2_KEY, FIELD_UTF2_TEXT);
}

DocHelper::StaticConstructor DocHelper::staticConstructor;
const shared_ptr<org::apache::lucene::document::FieldType>
    DocHelper::customType2;
const wstring DocHelper::FIELD_2_TEXT = L"field field field two text";
std::deque<int> const DocHelper::FIELD_2_FREQS = {3, 1, 1};
const wstring DocHelper::TEXT_FIELD_2_KEY = L"textField2";
shared_ptr<org::apache::lucene::document::Field> DocHelper::textField2;
const shared_ptr<org::apache::lucene::document::FieldType>
    DocHelper::customType3;
const wstring DocHelper::FIELD_3_TEXT = L"aaaNoNorms aaaNoNorms bbbNoNorms";
const wstring DocHelper::TEXT_FIELD_3_KEY = L"textField3";
shared_ptr<org::apache::lucene::document::Field> DocHelper::textField3;
const wstring DocHelper::KEYWORD_TEXT = L"Keyword";
const wstring DocHelper::KEYWORD_FIELD_KEY = L"keyField";
shared_ptr<org::apache::lucene::document::Field> DocHelper::keyField;
const shared_ptr<org::apache::lucene::document::FieldType>
    DocHelper::customType5;
const wstring DocHelper::NO_NORMS_TEXT = L"omitNormsText";
const wstring DocHelper::NO_NORMS_KEY = L"omitNorms";
shared_ptr<org::apache::lucene::document::Field> DocHelper::noNormsField;
const shared_ptr<org::apache::lucene::document::FieldType>
    DocHelper::customType6;
const wstring DocHelper::NO_TF_TEXT = L"analyzed with no tf and positions";
const wstring DocHelper::NO_TF_KEY = L"omitTermFreqAndPositions";
shared_ptr<org::apache::lucene::document::Field> DocHelper::noTFField;
const shared_ptr<org::apache::lucene::document::FieldType>
    DocHelper::customType7;
const wstring DocHelper::UNINDEXED_FIELD_TEXT = L"unindexed field text";
const wstring DocHelper::UNINDEXED_FIELD_KEY = L"unIndField";
shared_ptr<org::apache::lucene::document::Field> DocHelper::unIndField;
const wstring DocHelper::UNSTORED_1_FIELD_TEXT = L"unstored field text";
const wstring DocHelper::UNSTORED_FIELD_1_KEY = L"unStoredField1";
shared_ptr<org::apache::lucene::document::Field> DocHelper::unStoredField1 =
    make_shared<org::apache::lucene::document::TextField>(
        UNSTORED_FIELD_1_KEY, UNSTORED_1_FIELD_TEXT,
        org::apache::lucene::document::Field::Store::NO);
const shared_ptr<org::apache::lucene::document::FieldType>
    DocHelper::customType8;
const wstring DocHelper::UNSTORED_2_FIELD_TEXT = L"unstored field text";
const wstring DocHelper::UNSTORED_FIELD_2_KEY = L"unStoredField2";
shared_ptr<org::apache::lucene::document::Field> DocHelper::unStoredField2;
const wstring DocHelper::LAZY_FIELD_BINARY_KEY = L"lazyFieldBinary";
std::deque<char> DocHelper::LAZY_FIELD_BINARY_BYTES;
shared_ptr<org::apache::lucene::document::Field> DocHelper::lazyFieldBinary;
const wstring DocHelper::LAZY_FIELD_KEY = L"lazyField";
const wstring DocHelper::LAZY_FIELD_TEXT = L"These are some field bytes";
shared_ptr<org::apache::lucene::document::Field> DocHelper::lazyField =
    make_shared<org::apache::lucene::document::Field>(
        LAZY_FIELD_KEY, LAZY_FIELD_TEXT, customType);
const wstring DocHelper::LARGE_LAZY_FIELD_KEY = L"largeLazyField";
wstring DocHelper::LARGE_LAZY_FIELD_TEXT;
shared_ptr<org::apache::lucene::document::Field> DocHelper::largeLazyField;
const wstring DocHelper::FIELD_UTF1_TEXT = L"field one \u4e00text";
const wstring DocHelper::TEXT_FIELD_UTF1_KEY = L"textField1Utf8";
shared_ptr<org::apache::lucene::document::Field> DocHelper::textUtfField1 =
    make_shared<org::apache::lucene::document::Field>(
        TEXT_FIELD_UTF1_KEY, FIELD_UTF1_TEXT, customType);
const wstring DocHelper::FIELD_UTF2_TEXT = L"field field field \u4e00two text";
std::deque<int> const DocHelper::FIELD_UTF2_FREQS = {3, 1, 1};
const wstring DocHelper::TEXT_FIELD_UTF2_KEY = L"textField2Utf8";
shared_ptr<org::apache::lucene::document::Field> DocHelper::textUtfField2 =
    make_shared<org::apache::lucene::document::Field>(
        TEXT_FIELD_UTF2_KEY, FIELD_UTF2_TEXT, customType2);
unordered_map<wstring, any> DocHelper::nameValues = nullptr;
std::deque<std::shared_ptr<org::apache::lucene::document::Field>>
    DocHelper::fields = {textField1,      textField2,     textField3,
                         keyField,        noNormsField,   noTFField,
                         unIndField,      unStoredField1, unStoredField2,
                         textUtfField1,   textUtfField2,  lazyField,
                         lazyFieldBinary, largeLazyField};
unordered_map<wstring, std::shared_ptr<IndexableField>> DocHelper::all =
    unordered_map<wstring, std::shared_ptr<IndexableField>>();
unordered_map<wstring, std::shared_ptr<IndexableField>> DocHelper::indexed =
    unordered_map<wstring, std::shared_ptr<IndexableField>>();
unordered_map<wstring, std::shared_ptr<IndexableField>> DocHelper::stored =
    unordered_map<wstring, std::shared_ptr<IndexableField>>();
unordered_map<wstring, std::shared_ptr<IndexableField>> DocHelper::unstored =
    unordered_map<wstring, std::shared_ptr<IndexableField>>();
unordered_map<wstring, std::shared_ptr<IndexableField>> DocHelper::unindexed =
    unordered_map<wstring, std::shared_ptr<IndexableField>>();
unordered_map<wstring, std::shared_ptr<IndexableField>> DocHelper::termvector =
    unordered_map<wstring, std::shared_ptr<IndexableField>>();
unordered_map<wstring, std::shared_ptr<IndexableField>>
    DocHelper::notermvector =
        unordered_map<wstring, std::shared_ptr<IndexableField>>();
unordered_map<wstring, std::shared_ptr<IndexableField>> DocHelper::lazy =
    unordered_map<wstring, std::shared_ptr<IndexableField>>();
unordered_map<wstring, std::shared_ptr<IndexableField>> DocHelper::noNorms =
    unordered_map<wstring, std::shared_ptr<IndexableField>>();
unordered_map<wstring, std::shared_ptr<IndexableField>> DocHelper::noTf =
    unordered_map<wstring, std::shared_ptr<IndexableField>>();

void DocHelper::add(
    unordered_map<wstring, std::shared_ptr<IndexableField>> &map_obj,
    shared_ptr<IndexableField> field)
{
  map_obj.emplace(field->name(), field);
}

void DocHelper::setupDoc(shared_ptr<Document> doc)
{
  for (int i = 0; i < fields.size(); i++) {
    doc->push_back(fields[i]);
  }
}

shared_ptr<SegmentCommitInfo>
DocHelper::writeDoc(shared_ptr<Random> random, shared_ptr<Directory> dir,
                    shared_ptr<Document> doc) 
{
  return writeDoc(
      random, dir,
      make_shared<MockAnalyzer>(random, MockTokenizer::WHITESPACE, false),
      nullptr, doc);
}

shared_ptr<SegmentCommitInfo>
DocHelper::writeDoc(shared_ptr<Random> random, shared_ptr<Directory> dir,
                    shared_ptr<Analyzer> analyzer,
                    shared_ptr<Similarity> similarity,
                    shared_ptr<Document> doc) 
{
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, (make_shared<IndexWriterConfig>(analyzer))
               ->setSimilarity(similarity == nullptr
                                   ? IndexSearcher::getDefaultSimilarity()
                                   : similarity));
  // writer.setNoCFSRatio(0.0);
  writer->addDocument(doc);
  writer->commit();
  shared_ptr<SegmentCommitInfo> info = writer->newestSegment();
  delete writer;
  return info;
}

int DocHelper::numFields(shared_ptr<Document> doc)
{
  return doc->getFields().size();
}

shared_ptr<Document> DocHelper::createDocument(int n, const wstring &indexName,
                                               int numFields)
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorPositions(true);
  customType->setStoreTermVectorOffsets(true);

  shared_ptr<FieldType> customType1 =
      make_shared<FieldType>(StringField::TYPE_STORED);
  customType1->setStoreTermVectors(true);
  customType1->setStoreTermVectorPositions(true);
  customType1->setStoreTermVectorOffsets(true);

  shared_ptr<Document> *const doc = make_shared<Document>();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  doc->push_back(make_shared<Field>(L"id", Integer::toString(n), customType1));
  doc->push_back(make_shared<Field>(L"indexname", indexName, customType1));
  sb->append(L"a");
  sb->append(n);
  doc->push_back(make_shared<Field>(L"field1", sb->toString(), customType));
  sb->append(L" b");
  sb->append(n);
  for (int i = 1; i < numFields; i++) {
    doc->push_back(make_shared<Field>(L"field" + to_wstring(i + 1),
                                      sb->toString(), customType));
  }
  return doc;
}
} // namespace org::apache::lucene::index