using namespace std;

#include "SoftDeletesRetentionMergePolicy.h"

namespace org::apache::lucene::index
{
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using DocValuesFieldExistsQuery =
    org::apache::lucene::search::DocValuesFieldExistsQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;
using Bits = org::apache::lucene::util::Bits;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOSupplier = org::apache::lucene::util::IOSupplier;

SoftDeletesRetentionMergePolicy::SoftDeletesRetentionMergePolicy(
    const wstring &field, function<Query *()> &retentionQuerySupplier,
    shared_ptr<MergePolicy> in_)
    : field(field), retentionQuerySupplier(retentionQuerySupplier)
{
      OneMergeWrappingMergePolicy(in_, [&] (any toWrap)
      {
    make_shared<MergePolicy::OneMerge>(toWrap::segments);
      };
      {
        CodecReader wrapForMerge(CodecReader reader) throws IOException
        {
          shared_ptr<CodecReader> wrapped = toWrap::wrapForMerge(reader);
          shared_ptr<Bits> liveDocs = reader::getLiveDocs();
          if (liveDocs == nullptr)
          { // no deletes - just keep going
    return wrapped;
          }
          return applyRetentionQuery(field, retentionQuerySupplier(), wrapped);
}
} // namespace org::apache::lucene::index
     );
     Objects::requireNonNull(field, L"field must not be null");
     Objects::requireNonNull(retentionQuerySupplier,
                             L"retentionQuerySupplier must not be null");
     }

     bool SoftDeletesRetentionMergePolicy::keepFullyDeletedSegment(
         IOSupplier<std::shared_ptr<CodecReader>>
             readerIOSupplier) 
     {
       shared_ptr<CodecReader> reader = readerIOSupplier();
       /* we only need a single hit to keep it no need for soft deletes to be
        * checked*/
       shared_ptr<Scorer> scorer =
           getScorer(retentionQuerySupplier->get(),
                     wrapLiveDocs(reader, nullptr, reader->maxDoc()));
       if (scorer != nullptr) {
         shared_ptr<DocIdSetIterator> iterator = scorer->begin();
         bool atLeastOneHit =
             iterator->nextDoc() != DocIdSetIterator::NO_MORE_DOCS;
         return atLeastOneHit;
       }
       return OneMergeWrappingMergePolicy::keepFullyDeletedSegment(
           readerIOSupplier);
     }

     shared_ptr<CodecReader>
     SoftDeletesRetentionMergePolicy::applyRetentionQuery(
         const wstring &softDeleteField, shared_ptr<Query> retentionQuery,
         shared_ptr<CodecReader> reader) 
     {
       shared_ptr<Bits> liveDocs = reader->getLiveDocs();
       if (liveDocs == nullptr) { // no deletes - just keep going
         return reader;
       }
       shared_ptr<CodecReader> wrappedReader =
           wrapLiveDocs(reader, make_shared<BitsAnonymousInnerClass>(liveDocs),
                        reader->maxDoc() - reader->numDocs());
       shared_ptr<BooleanQuery::Builder> builder =
           make_shared<BooleanQuery::Builder>();
       builder->add(make_shared<DocValuesFieldExistsQuery>(softDeleteField),
                    BooleanClause::Occur::FILTER);
       builder->add(retentionQuery, BooleanClause::Occur::FILTER);
       shared_ptr<Scorer> scorer = getScorer(builder->build(), wrappedReader);
       if (scorer != nullptr) {
         shared_ptr<FixedBitSet> cloneLiveDocs =
             SoftDeletesRetentionMergePolicy::cloneLiveDocs(liveDocs);
         shared_ptr<DocIdSetIterator> iterator = scorer->begin();
         int numExtraLiveDocs = 0;
         while (iterator->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
           if (cloneLiveDocs->getAndSet(iterator->docID()) == false) {
             // if we bring one back to live we need to account for it
             numExtraLiveDocs++;
           }
         }
         assert((reader->numDocs() + numExtraLiveDocs <= reader->maxDoc(),
                 L"numDocs: " + to_wstring(reader->numDocs()) +
                     L" numExtraLiveDocs: " + to_wstring(numExtraLiveDocs) +
                     L" maxDoc: " + to_wstring(reader->maxDoc())));
         return wrapLiveDocs(reader, cloneLiveDocs,
                             reader->numDocs() + numExtraLiveDocs);
       } else {
         return reader;
       }
     }

     SoftDeletesRetentionMergePolicy::BitsAnonymousInnerClass::
         BitsAnonymousInnerClass(shared_ptr<Bits> liveDocs)
     {
       this->liveDocs = liveDocs;
     }

     bool
     SoftDeletesRetentionMergePolicy::BitsAnonymousInnerClass::get(int index)
     {
       return liveDocs->get(index) == false;
     }

     int SoftDeletesRetentionMergePolicy::BitsAnonymousInnerClass::length()
     {
       return liveDocs->length();
     }

     shared_ptr<FixedBitSet>
     SoftDeletesRetentionMergePolicy::cloneLiveDocs(shared_ptr<Bits> liveDocs)
     {
       if (std::dynamic_pointer_cast<FixedBitSet>(liveDocs) != nullptr) {
         return (std::static_pointer_cast<FixedBitSet>(liveDocs))->clone();
       } else { // mainly if we have asserting codec
         shared_ptr<FixedBitSet> mutableBits =
             make_shared<FixedBitSet>(liveDocs->length());
         for (int i = 0; i < liveDocs->length(); i++) {
           if (liveDocs->get(i)) {
             mutableBits->set(i);
           }
         }
         return mutableBits;
       }
     }

     shared_ptr<Scorer> SoftDeletesRetentionMergePolicy::getScorer(
         shared_ptr<Query> query,
         shared_ptr<CodecReader> reader) 
     {
       shared_ptr<IndexSearcher> s = make_shared<IndexSearcher>(reader);
       s->setQueryCache(nullptr);
       shared_ptr<Weight> weight = s->createWeight(query, false, 1.0f);
       return weight->scorer(reader->getContext());
     }

     shared_ptr<CodecReader> SoftDeletesRetentionMergePolicy::wrapLiveDocs(
         shared_ptr<CodecReader> reader, shared_ptr<Bits> liveDocs, int numDocs)
     {
       return make_shared<FilterCodecReaderAnonymousInnerClass>(
           reader, liveDocs, numDocs);
     }

     SoftDeletesRetentionMergePolicy::FilterCodecReaderAnonymousInnerClass::
         FilterCodecReaderAnonymousInnerClass(
             shared_ptr<org::apache::lucene::index::CodecReader> reader,
             shared_ptr<Bits> liveDocs, int numDocs)
         : FilterCodecReader(reader)
     {
       this->reader = reader;
       this->liveDocs = liveDocs;
       this->numDocs = numDocs;
     }

     shared_ptr<CacheHelper> SoftDeletesRetentionMergePolicy::
         FilterCodecReaderAnonymousInnerClass::getCoreCacheHelper()
     {
       return reader->getCoreCacheHelper();
     }

     shared_ptr<CacheHelper> SoftDeletesRetentionMergePolicy::
         FilterCodecReaderAnonymousInnerClass::getReaderCacheHelper()
     {
       return nullptr; // we are altering live docs
     }

     shared_ptr<Bits> SoftDeletesRetentionMergePolicy::
         FilterCodecReaderAnonymousInnerClass::getLiveDocs()
     {
       return liveDocs;
     }

     int SoftDeletesRetentionMergePolicy::FilterCodecReaderAnonymousInnerClass::
         numDocs()
     {
       return numDocs;
     }

     int SoftDeletesRetentionMergePolicy::numDeletesToMerge(
         shared_ptr<SegmentCommitInfo> info, int delCount,
         IOSupplier<std::shared_ptr<CodecReader>>
             readerSupplier) 
     {
       constexpr int numDeletesToMerge =
           OneMergeWrappingMergePolicy::numDeletesToMerge(info, delCount,
                                                          readerSupplier);
       if (numDeletesToMerge != 0 && info->getSoftDelCount() > 0) {
         shared_ptr<CodecReader> *const reader = readerSupplier();
         if (reader->getLiveDocs() != nullptr) {
           shared_ptr<BooleanQuery::Builder> builder =
               make_shared<BooleanQuery::Builder>();
           builder->add(make_shared<DocValuesFieldExistsQuery>(field),
                        BooleanClause::Occur::FILTER);
           builder->add(retentionQuerySupplier->get(),
                        BooleanClause::Occur::FILTER);
           shared_ptr<Scorer> scorer =
               getScorer(builder->build(),
                         wrapLiveDocs(reader, nullptr, reader->maxDoc()));
           if (scorer != nullptr) {
             shared_ptr<DocIdSetIterator> iterator = scorer->begin();
             shared_ptr<Bits> liveDocs = reader->getLiveDocs();
             int numDeletedDocs = reader->numDeletedDocs();
             while (iterator->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
               if (liveDocs->get(iterator->docID()) == false) {
                 numDeletedDocs--;
               }
             }
             return numDeletedDocs;
           }
         }
       }
       assert((numDeletesToMerge >= 0,
               L"numDeletesToMerge: " + to_wstring(numDeletesToMerge)));
       assert((numDeletesToMerge <= info->info->maxDoc(),
               L"numDeletesToMerge: " + to_wstring(numDeletesToMerge) +
                   L" maxDoc:" + to_wstring(info->info->maxDoc())));
       return numDeletesToMerge;
     }
     }