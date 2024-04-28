// Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Authors: Zhangyi Chen(chenzhangyi01@baidu.com)

#include "braft/configuration_manager.h"

#include <algorithm>

namespace braft {

int ConfigurationManager::add(ConfigurationEntry&& entry) {
    if (!_configurations.empty()) {
        if (_configurations.back().id.index >= entry.id.index) {
            CHECK(false) << "Did you forget to call truncate_suffix before "
                            " the last log index goes back";
            return -1;
        }
    }
    _configurations.push_back(std::move(entry));
    return 0;
}

void ConfigurationManager::truncate_prefix(const int64_t first_index_kept) {
    // Find the first element which index >= `first_index_kept`.
    auto it = std::lower_bound(
        _configurations.begin(), _configurations.end(), first_index_kept,
        [](const ConfigurationEntry& entry, int64_t index) {
            return entry.id.index < index;
        });

    // Remove prefix [begin, index).
    _configurations.erase(_configurations.begin(), it);
}

void ConfigurationManager::truncate_suffix(const int64_t last_index_kept) {
    // Find the first element which index > `last_index_kept`.
    auto it = std::upper_bound(
        _configurations.begin(), _configurations.end(), last_index_kept,
        [](int64_t index, const ConfigurationEntry& entry) {
            return index < entry.id.index;
        });

    // Remove suffix [index, end].
    _configurations.erase(it, _configurations.end());
}

void ConfigurationManager::set_snapshot(ConfigurationEntry&& entry) {
    CHECK_GE(entry.id, _snapshot.id);
    _snapshot = std::move(entry);
}

void ConfigurationManager::get(int64_t last_included_index,
                               ConfigurationEntry* conf) {
    if (_configurations.empty()) {
        CHECK_GE(last_included_index, _snapshot.id.index);
        *conf = _snapshot;
        return;
    }

    // Entries index is strictly increase monotonically.
    // We can use binary search to find the i which
    // _configurations[i-1].index < `last_included_index` <=
    // _configurations[i].id.index
    // From end to begin, the first element which index <= `last_included_index`
    auto it = std::lower_bound(
        _configurations.rbegin(), _configurations.rend(), last_included_index,
        [](const ConfigurationEntry& rhs, int64_t index) {
            return rhs.id.index > index;
        });

    CHECK(it != _configurations.rend());
    *conf = *it;
}

const ConfigurationEntry& ConfigurationManager::last_configuration() const {
    if (!_configurations.empty()) {
        return _configurations.back();
    }
    return _snapshot;
}

}  //  namespace braft
