#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::search
{

/** Tracks live field values across NRT reader reopens.
 *  This holds a map_obj for all updated ids since
 *  the last reader reopen.  Once the NRT reader is reopened,
 *  it prunes the map_obj.  This means you must reopen your NRT
 *  reader periodically otherwise the RAM consumption of
 *  this class will grow unbounded!
 *
 *  <p>NOTE: you must ensure the same id is never updated at
 *  the same time by two threads, because in this case you
 *  cannot in general know which thread "won". */

// TODO: should this class handle deletions better...?
template <typename S, typename T>
class LiveFieldValues : public std::enable_shared_from_this<LiveFieldValues>,
                        public ReferenceManager::RefreshListener
{
  GET_CLASS_NAME(LiveFieldValues)

private:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile java.util.Map<std::wstring,T> current = new
  // java.util.concurrent.ConcurrentHashMap<>();
  std::unordered_map<std::wstring, T> current =
      std::make_shared<ConcurrentHashMap<std::wstring, T>>();
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile java.util.Map<std::wstring,T> old = new
  // java.util.concurrent.ConcurrentHashMap<>();
  std::unordered_map<std::wstring, T> old =
      std::make_shared<ConcurrentHashMap<std::wstring, T>>();
  const std::shared_ptr<ReferenceManager<S>> mgr;
  const T missingValue;

  /** The missingValue must be non-null. */
public:
  LiveFieldValues(std::shared_ptr<ReferenceManager<S>> mgr, T missingValue)
      : mgr(mgr), missingValue(missingValue)
  {
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    mgr->addListener(shared_from_this());
  }

  virtual ~LiveFieldValues() { mgr->removeListener(shared_from_this()); }

  void beforeRefresh()  override
  {
    old = current;
    // Start sending all updates after this point to the new
    // map_obj.  While reopen is running, any lookup will first
    // try this new map_obj, then fallback to old, then to the
    // current searcher:
    current = std::make_shared<ConcurrentHashMap<>>();
  }

  void afterRefresh(bool didRefresh)  override
  {
    // Now drop all the old values because they are now
    // visible via the searcher that was just opened; if
    // didRefresh is false, it's possible old has some
    // entries in it, which is fine: it means they were
    // actually already included in the previously opened
    // reader.  So we can safely clear old here:
    old = std::make_shared<ConcurrentHashMap<>>();
  }

  /** Call this after you've successfully added a document
   *  to the index, to record what value you just set the
   *  field to. */
  virtual void add(const std::wstring &id, T value)
  {
    current.emplace(id, value);
  }

  /** Call this after you've successfully deleted a document
   *  from the index. */
  virtual void delete_(const std::wstring &id)
  {
    current.emplace(id, missingValue);
  }

  /** Returns the [approximate] number of id/value pairs
   *  buffered in RAM. */
  virtual int size() { return current.size() + old.size(); }

  /** Returns the current value for this id, or null if the
   *  id isn't in the index or was deleted. */
  virtual T get(const std::wstring &id) 
  {
    // First try to get the "live" value:
    T value = current[id];
    if (value == missingValue) {
      // Deleted but the deletion is not yet reflected in
      // the reader:
      return nullptr;
    } else if (value != nullptr) {
      return value;
    } else {
      value = old[id];
      if (value == missingValue) {
        // Deleted but the deletion is not yet reflected in
        // the reader:
        return nullptr;
      } else if (value != nullptr) {
        return value;
      } else {
        // It either does not exist in the index, or, it was
        // already flushed & NRT reader was opened on the
        // segment, so fallback to current searcher:
        S s = mgr->acquire();
        try {
          return lookupFromSearcher(s, id);
        }
        // C++ TODO: There is no native C++ equivalent to the exception
        // 'finally' clause:
        finally {
          mgr->release(s);
        }
      }
    }
  }

  /** This is called when the id/value was already flushed and opened
   *  in an NRT IndexSearcher.  You must implement this to
   *  go look up the value (eg, via doc values, field cache,
   *  stored fields, etc.). */
protected:
  virtual T lookupFromSearcher(S s, const std::wstring &id) = 0;
};

} // namespace org::apache::lucene::search
