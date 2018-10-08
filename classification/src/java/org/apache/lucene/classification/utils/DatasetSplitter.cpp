using namespace std;

#include "DatasetSplitter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/Field.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/FieldType.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/TextField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexableField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SortedSetDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/MatchAllDocsQuery.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/ScoreDoc.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/Sort.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../grouping/src/java/org/apache/lucene/search/grouping/GroupingSearch.h"
#include "../../../../../../../../grouping/src/java/org/apache/lucene/search/grouping/TopGroups.h"

namespace org::apache::lucene::classification::utils
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using IndexableField = org::apache::lucene::index::IndexableField;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Sort = org::apache::lucene::search::Sort;
using GroupDocs = org::apache::lucene::search::grouping::GroupDocs;
using GroupingSearch = org::apache::lucene::search::grouping::GroupingSearch;
using TopGroups = org::apache::lucene::search::grouping::TopGroups;
using Directory = org::apache::lucene::store::Directory;

DatasetSplitter::DatasetSplitter(double testRatio, double crossValidationRatio)
    : crossValidationRatio(crossValidationRatio), testRatio(testRatio)
{
}

void DatasetSplitter::split(shared_ptr<IndexReader> originalIndex,
                            shared_ptr<Directory> trainingIndex,
                            shared_ptr<Directory> testIndex,
                            shared_ptr<Directory> crossValidationIndex,
                            shared_ptr<Analyzer> analyzer, bool termVectors,
                            const wstring &classFieldName,
                            deque<wstring> &fieldNames) 
{

  // create IWs for train / test / cv IDXs
  shared_ptr<IndexWriter> testWriter = make_shared<IndexWriter>(
      testIndex, make_shared<IndexWriterConfig>(analyzer));
  shared_ptr<IndexWriter> cvWriter = make_shared<IndexWriter>(
      crossValidationIndex, make_shared<IndexWriterConfig>(analyzer));
  shared_ptr<IndexWriter> trainingWriter = make_shared<IndexWriter>(
      trainingIndex, make_shared<IndexWriterConfig>(analyzer));

  // get the exact no. of existing classes
  int noOfClasses = 0;
  for (auto leave : originalIndex->leaves()) {
    int64_t valueCount = 0;
    shared_ptr<SortedDocValues> classValues =
        leave->reader()->getSortedDocValues(classFieldName);
    if (classValues != nullptr) {
      valueCount = classValues->getValueCount();
    } else {
      shared_ptr<SortedSetDocValues> sortedSetDocValues =
          leave->reader()->getSortedSetDocValues(classFieldName);
      if (sortedSetDocValues != nullptr) {
        valueCount = sortedSetDocValues->getValueCount();
      }
    }
    if (classValues == nullptr) {
      // approximate with no. of terms
      noOfClasses += leave->reader()->terms(classFieldName)->size();
    }
    noOfClasses += valueCount;
  }

  try {

    shared_ptr<IndexSearcher> indexSearcher =
        make_shared<IndexSearcher>(originalIndex);
    shared_ptr<GroupingSearch> gs = make_shared<GroupingSearch>(classFieldName);
    gs->setGroupSort(Sort::INDEXORDER);
    gs->setSortWithinGroup(Sort::INDEXORDER);
    gs->setAllGroups(true);
    gs->setGroupDocsLimit(originalIndex->maxDoc());
    shared_ptr<TopGroups<any>> topGroups = gs->search(
        indexSearcher, make_shared<MatchAllDocsQuery>(), 0, noOfClasses);

    // set the type to be indexed, stored, with term vectors
    shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_STORED);
    if (termVectors) {
      ft->setStoreTermVectors(true);
      ft->setStoreTermVectorOffsets(true);
      ft->setStoreTermVectorPositions(true);
    }

    int b = 0;

    // iterate over existing documents
    for (auto group : topGroups->groups) {
      int64_t totalHits = group->totalHits;
      double testSize = totalHits * testRatio;
      int tc = 0;
      double cvSize = totalHits * crossValidationRatio;
      int cvc = 0;
      for (auto scoreDoc : group->scoreDocs) {

        // create a new document for indexing
        shared_ptr<Document> doc =
            createNewDoc(originalIndex, ft, scoreDoc, fieldNames);

        // add it to one of the IDXs
        if (b % 2 == 0 && tc < testSize) {
          testWriter->addDocument(doc);
          tc++;
        } else if (cvc < cvSize) {
          cvWriter->addDocument(doc);
          cvc++;
        } else {
          trainingWriter->addDocument(doc);
        }
        b++;
      }
    }
    // commit
    testWriter->commit();
    cvWriter->commit();
    trainingWriter->commit();

    // merge
    testWriter->forceMerge(3);
    cvWriter->forceMerge(3);
    trainingWriter->forceMerge(3);
  } catch (const runtime_error &e) {
    throw make_shared<IOException>(e);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    // close IWs
    delete testWriter;
    delete cvWriter;
    delete trainingWriter;
    delete originalIndex;
  }
}

shared_ptr<Document> DatasetSplitter::createNewDoc(
    shared_ptr<IndexReader> originalIndex, shared_ptr<FieldType> ft,
    shared_ptr<ScoreDoc> scoreDoc,
    std::deque<wstring> &fieldNames) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Document> document = originalIndex->document(scoreDoc->doc);
  if (fieldNames.size() > 0 && fieldNames.size() > 0) {
    for (auto fieldName : fieldNames) {
      shared_ptr<IndexableField> field = document->getField(fieldName);
      if (field != nullptr) {
        doc->push_back(make_shared<Field>(fieldName, field->stringValue(), ft));
      }
    }
  } else {
    for (auto field : document->getFields()) {
      if (field->readerValue() != nullptr) {
        doc->push_back(
            make_shared<Field>(field->name(), field->readerValue(), ft));
      } else if (field->binaryValue() != nullptr) {
        doc->push_back(
            make_shared<Field>(field->name(), field->binaryValue(), ft));
      } else if (field->stringValue() != L"") {
        doc->push_back(
            make_shared<Field>(field->name(), field->stringValue(), ft));
      } else if (field->numericValue() != nullptr) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        doc->push_back(make_shared<Field>(
            field->name(), field->numericValue()->toString(), ft));
      }
    }
  }
  return doc;
}
} // namespace org::apache::lucene::classification::utils