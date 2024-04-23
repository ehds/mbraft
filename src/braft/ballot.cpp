// Copyright (c) 2018 Baidu.com, Inc. All Rights Reserved
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

#include "braft/ballot.h"

#include <algorithm>
#include <cassert>
#include <iterator>

#include "braft/configuration.h"

namespace braft {

void Ballot::init(const Configuration& conf,
                  std::optional<const Configuration> old_conf) {
    _peers.clear();
    _old_peers.clear();
    _quorum = 0;
    _old_quorum = 0;

    CHECK_GT(conf.size(), 0);
    _peers.reserve(conf.size());
    std::copy(conf.begin(), conf.end(), std::back_inserter(_peers));
    _quorum = _peers.size() / 2 + 1;

    if (!old_conf.has_value()) {
        return;
    }
    _old_peers.reserve(old_conf->size());
    std::copy(old_conf->begin(), old_conf->end(),
              std::back_inserter(_old_peers));
    _old_quorum = _old_peers.size() / 2 + 1;
}

Ballot::PosHint Ballot::grant(const PeerId& peer, PosHint hint) {
    std::vector<UnfoundPeerId>::iterator iter;
    iter = find_peer(peer, _peers, hint.pos0);
    if (iter != _peers.end()) {
        if (!iter->found) {
            iter->found = true;
            --_quorum;
        }
        hint.pos0 = iter - _peers.begin();
    } else {
        hint.pos0 = 0;
    }

    if (_old_peers.empty()) {
        hint.pos1 = 0;
        return hint;
    }

    iter = find_peer(peer, _old_peers, hint.pos1);

    if (iter != _old_peers.end()) {
        if (!iter->found) {
            iter->found = true;
            --_old_quorum;
        }
        hint.pos1 = iter - _old_peers.begin();
    } else {
        hint.pos1 = 0;
    }

    return hint;
}

void Ballot::grant(const PeerId& peer) { grant(peer, PosHint()); }

}  // namespace braft
