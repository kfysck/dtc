/*
* Copyright [2021] JD.com, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#pragma once

#ifndef ROCKSDB_LITE

#include "rocksdb/iterator.h"
#include "rocksdb/options.h"
#include "rocksdb/slice.h"
#include "rocksdb/table_properties.h"

namespace rocksdb {

// SstFileReader is used to read sst files that are generated by DB or
// SstFileWriter.
class SstFileReader {
 public:
  SstFileReader(const Options& options);

  ~SstFileReader();

  // Prepares to read from the file located at "file_path".
  Status Open(const std::string& file_path);

  // Returns a new iterator over the table contents.
  // Most read options provide the same control as we read from DB.
  // If "snapshot" is nullptr, the iterator returns only the latest keys.
  Iterator* NewIterator(const ReadOptions& options);

  std::shared_ptr<const TableProperties> GetTableProperties() const;

  // Verifies whether there is corruption in this table.
  Status VerifyChecksum(const ReadOptions& /*read_options*/);

  Status VerifyChecksum() { return VerifyChecksum(ReadOptions()); }

 private:
  struct Rep;
  std::unique_ptr<Rep> rep_;
};

}  // namespace rocksdb

#endif  // !ROCKSDB_LITE