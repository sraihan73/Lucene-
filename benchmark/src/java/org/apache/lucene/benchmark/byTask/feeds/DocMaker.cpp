using namespace std;

#include "DocMaker.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/DoublePoint.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Field.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/FieldType.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/FloatPoint.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/IntPoint.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/LongPoint.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/StringField.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/TextField.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexOptions.h"
#include "../utils/Config.h"
#include "ContentSource.h"
#include "DocData.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using Document = org::apache::lucene::document::Document;
using DoublePoint = org::apache::lucene::document::DoublePoint;
using Field = org::apache::lucene::document::Field;
using FloatPoint = org::apache::lucene::document::FloatPoint;
using IntPoint = org::apache::lucene::document::IntPoint;
using FieldType = org::apache::lucene::document::FieldType;
using LongPoint = org::apache::lucene::document::LongPoint;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using IndexOptions = org::apache::lucene::index::IndexOptions;

DocMaker::DocState::DocState(bool reuseFields, shared_ptr<FieldType> ft,
                             shared_ptr<FieldType> bodyFt)
    : reuseFields(reuseFields)
{

  if (reuseFields) {
    fields = unordered_map<>();
    numericFields = unordered_map<>();

    // Initialize the map_obj with the default fields.
    fields.emplace(BODY_FIELD, make_shared<Field>(BODY_FIELD, L"", bodyFt));
    fields.emplace(TITLE_FIELD, make_shared<Field>(TITLE_FIELD, L"", ft));
    fields.emplace(DATE_FIELD, make_shared<Field>(DATE_FIELD, L"", ft));
    fields.emplace(ID_FIELD,
                   make_shared<StringField>(ID_FIELD, L"", Field::Store::YES));
    fields.emplace(NAME_FIELD, make_shared<Field>(NAME_FIELD, L"", ft));

    numericFields.emplace(DATE_MSEC_FIELD,
                          make_shared<LongPoint>(DATE_MSEC_FIELD, 0LL));
    numericFields.emplace(TIME_SEC_FIELD,
                          make_shared<IntPoint>(TIME_SEC_FIELD, 0));

    doc = make_shared<Document>();
  } else {
    numericFields.clear();
    fields.clear();
    doc.reset();
  }
}

shared_ptr<Field> DocMaker::DocState::getField(const wstring &name,
                                               shared_ptr<FieldType> ft)
{
  if (!reuseFields) {
    return make_shared<Field>(name, L"", ft);
  }

  shared_ptr<Field> f = fields[name];
  if (f == nullptr) {
    f = make_shared<Field>(name, L"", ft);
    fields.emplace(name, f);
  }
  return f;
}

shared_ptr<Field> DocMaker::DocState::getNumericField(const wstring &name,
                                                      type_info numericType)
{
  shared_ptr<Field> f;
  if (reuseFields) {
    f = numericFields[name];
  } else {
    f.reset();
  }

  if (f == nullptr) {
    if (numericType.equals(Integer::typeid)) {
      f = make_shared<IntPoint>(name, 0);
    } else if (numericType.equals(Long::typeid)) {
      f = make_shared<LongPoint>(name, 0LL);
    } else if (numericType.equals(Float::typeid)) {
      f = make_shared<FloatPoint>(name, 0.0F);
    } else if (numericType.equals(Double::typeid)) {
      f = make_shared<DoublePoint>(name, 0.0);
    } else {
      throw make_shared<UnsupportedOperationException>(
          L"Unsupported numeric type: " + numericType);
    }
    if (reuseFields) {
      numericFields.emplace(name, f);
    }
  }
  return f;
}

DocMaker::DateUtil::DateUtil() { parser->setLenient(true); }

const wstring DocMaker::BODY_FIELD = L"body";
const wstring DocMaker::TITLE_FIELD = L"doctitle";
const wstring DocMaker::DATE_FIELD = L"docdate";
const wstring DocMaker::DATE_MSEC_FIELD = L"docdatenum";
const wstring DocMaker::TIME_SEC_FIELD = L"doctimesecnum";
const wstring DocMaker::ID_FIELD = L"docid";
const wstring DocMaker::BYTES_FIELD = L"bytes";
const wstring DocMaker::NAME_FIELD = L"docname";

DocMaker::DocMaker() {}

shared_ptr<Document>
DocMaker::createDocument(shared_ptr<DocData> docData, int size,
                         int cnt) 
{

  shared_ptr<DocState> *const ds = getDocState();
  shared_ptr<Document> *const doc =
      reuseFields ? ds->doc : make_shared<Document>();
  doc->clear();

  // Set ID_FIELD
  shared_ptr<FieldType> ft = make_shared<FieldType>(valType);
  ft->setStored(true);

  shared_ptr<Field> idField = ds->getField(ID_FIELD, ft);
  int id;
  if (r != nullptr) {
    id = r->nextInt(updateDocIDLimit);
  } else {
    id = docData->getID();
    if (id == -1) {
      id = numDocsCreated->getAndIncrement();
    }
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  idField->setStringValue(Integer::toString(id));
  doc->push_back(idField);

  // Set NAME_FIELD
  wstring name = docData->getName();
  if (name == L"") {
    name = L"";
  }
  name = cnt < 0 ? name : name + L"_" + to_wstring(cnt);
  shared_ptr<Field> nameField = ds->getField(NAME_FIELD, valType);
  nameField->setStringValue(name);
  doc->push_back(nameField);

  // Set DATE_FIELD
  shared_ptr<DateUtil> util = dateParsers->get();
  if (util == nullptr) {
    util = make_shared<DateUtil>();
    dateParsers->set(util);
  }
  Date date = nullptr;
  wstring dateString = docData->getDate();
  if (dateString != L"") {
    util->pos->setIndex(0);
    date = util->parser->parse(dateString, util->pos);
    // System.out.println(dateString + " parsed to " + date);
  } else {
    dateString = L"";
  }
  shared_ptr<Field> dateStringField = ds->getField(DATE_FIELD, valType);
  dateStringField->setStringValue(dateString);
  doc->push_back(dateStringField);

  if (date == nullptr) {
    // just set to right now
    date = Date();
  }

  shared_ptr<Field> dateField =
      ds->getNumericField(DATE_MSEC_FIELD, Long::typeid);
  dateField->setLongValue(date.getTime());
  doc->push_back(dateField);

  util->cal->setTime(date);
  constexpr int sec = util->cal->get(Calendar::HOUR_OF_DAY) * 3600 +
                      util->cal->get(Calendar::MINUTE) * 60 +
                      util->cal->get(Calendar::SECOND);

  shared_ptr<Field> timeSecField =
      ds->getNumericField(TIME_SEC_FIELD, Integer::typeid);
  timeSecField->setIntValue(sec);
  doc->push_back(timeSecField);

  // Set TITLE_FIELD
  wstring title = docData->getTitle();
  shared_ptr<Field> titleField = ds->getField(TITLE_FIELD, valType);
  titleField->setStringValue(title == L"" ? L"" : title);
  doc->push_back(titleField);

  wstring body = docData->getBody();
  if (body != L"" && body.length() > 0) {
    wstring bdy;
    if (size <= 0 || size >= body.length()) {
      bdy = body;            // use all
      docData->setBody(L""); // nothing left
    } else {
      // attempt not to break words - if whitespace found within next 20
      // chars...
      for (int n = size - 1; n < size + 20 && n < body.length(); n++) {
        if (isspace(body[n])) {
          size = n;
          break;
        }
      }
      bdy = body.substr(0, size);          // use part
      docData->setBody(body.substr(size)); // some left
    }
    shared_ptr<Field> bodyField = ds->getField(BODY_FIELD, bodyValType);
    bodyField->setStringValue(bdy);
    doc->push_back(bodyField);

    if (storeBytes) {
      shared_ptr<Field> bytesField =
          ds->getField(BYTES_FIELD, StringField::TYPE_STORED);
      bytesField->setBytesValue(bdy.getBytes(StandardCharsets::UTF_8));
      doc->push_back(bytesField);
    }
  }

  if (indexProperties) {
    shared_ptr<Properties> props = docData->getProps();
    if (props != nullptr) {
      for (auto entry : props) {
        shared_ptr<Field> f =
            ds->getField(static_cast<wstring>(entry.first), valType);
        f->setStringValue(static_cast<wstring>(entry.second));
        doc->push_back(f);
      }
      docData->setProps(nullptr);
    }
  }

  // System.out.println("============== Created doc "+numDocsCreated+"
  // :\n"+doc+"\n==========");
  return doc;
}

void DocMaker::resetLeftovers() { leftovr->set(nullptr); }

shared_ptr<DocState> DocMaker::getDocState()
{
  shared_ptr<DocState> ds = docState->get();
  if (ds == nullptr) {
    ds = make_shared<DocState>(reuseFields, valType, bodyValType);
    docState->set(ds);
  }
  return ds;
}

DocMaker::~DocMaker() { delete source; }

shared_ptr<Document> DocMaker::makeDocument() 
{
  resetLeftovers();
  shared_ptr<DocData> docData = source->getNextDocData(getDocState()->docData);
  shared_ptr<Document> doc = createDocument(docData, 0, -1);
  return doc;
}

shared_ptr<Document> DocMaker::makeDocument(int size) 
{
  shared_ptr<LeftOver> lvr = leftovr->get();
  if (lvr == nullptr || lvr->docdata == nullptr ||
      lvr->docdata.getBody() == nullptr ||
      lvr->docdata.getBody()->length() == 0) {
    resetLeftovers();
  }
  shared_ptr<DocData> docData = getDocState()->docData;
  shared_ptr<DocData> dd =
      (lvr == nullptr ? source->getNextDocData(docData) : lvr->docdata);
  int cnt = (lvr == nullptr ? 0 : lvr->cnt);
  while (dd->getBody() == L"" || dd->getBody().length() < size) {
    shared_ptr<DocData> dd2 = dd;
    dd = source->getNextDocData(make_shared<DocData>());
    cnt = 0;
    dd->setBody(dd2->getBody() + dd->getBody());
  }
  shared_ptr<Document> doc = createDocument(dd, size, cnt);
  if (dd->getBody() == L"" || dd->getBody().length() == 0) {
    resetLeftovers();
  } else {
    if (lvr == nullptr) {
      lvr = make_shared<LeftOver>();
      leftovr->set(lvr);
    }
    lvr->docdata = dd;
    lvr->cnt = ++cnt;
  }
  return doc;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void DocMaker::resetInputs() 
{
  source->printStatistics(L"docs");
  // re-initiate since properties by round may have changed.
  setConfig(config, source);
  source->resetInputs();
  numDocsCreated->set(0);
  resetLeftovers();
}

void DocMaker::setConfig(shared_ptr<Config> config,
                         shared_ptr<ContentSource> source)
{
  this->config = config;
  this->source = source;

  bool stored = config->get(L"doc.stored", false);
  bool bodyStored = config->get(L"doc.body.stored", stored);
  bool tokenized = config->get(L"doc.tokenized", true);
  bool bodyTokenized = config->get(L"doc.body.tokenized", tokenized);
  bool norms = config->get(L"doc.tokenized.norms", false);
  bool bodyNorms = config->get(L"doc.body.tokenized.norms", true);
  bool bodyOffsets = config->get(L"doc.body.offsets", false);
  bool termVec = config->get(L"doc.term.deque", false);
  bool termVecPositions = config->get(L"doc.term.deque.positions", false);
  bool termVecOffsets = config->get(L"doc.term.deque.offsets", false);

  valType = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  valType->setStored(stored);
  valType->setTokenized(tokenized);
  valType->setOmitNorms(!norms);
  valType->setStoreTermVectors(termVec);
  valType->setStoreTermVectorPositions(termVecPositions);
  valType->setStoreTermVectorOffsets(termVecOffsets);
  valType->freeze();

  bodyValType = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  bodyValType->setStored(bodyStored);
  bodyValType->setTokenized(bodyTokenized);
  bodyValType->setOmitNorms(!bodyNorms);
  if (bodyTokenized && bodyOffsets) {
    bodyValType->setIndexOptions(
        IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS);
  }
  bodyValType->setStoreTermVectors(termVec);
  bodyValType->setStoreTermVectorPositions(termVecPositions);
  bodyValType->setStoreTermVectorOffsets(termVecOffsets);
  bodyValType->freeze();

  storeBytes = config->get(L"doc.store.body.bytes", false);

  reuseFields = config->get(L"doc.reuse.fields", true);

  // In a multi-rounds run, it is important to reset DocState since settings
  // of fields may change between rounds, and this is the only way to reset
  // the cache of all threads.
  docState = make_shared<ThreadLocal<DocState>>();

  indexProperties = config->get(L"doc.index.props", false);

  updateDocIDLimit = config->get(L"doc.random.id.limit", -1);
  if (updateDocIDLimit != -1) {
    r = make_shared<Random>(179);
  }
}
} // namespace org::apache::lucene::benchmark::byTask::feeds