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

#pragma once
#include <algorithm>
#include <vector>
#include <optional>

#include "braft/configuration.h"

namespace braft {

class Ballot {
   public:
    struct PosHint {
        PosHint() : pos0(0), pos1(0) {}
        size_t pos0;
        size_t pos1;
    };

    Ballot() : _quorum(0), _old_quorum(0){};
    //FIXME(ehds): Remove optional.
    // void init(const Configuration& conf, const Configuration& old_conf);
    void init(const Configuration& conf, std::optional<const Configuration> old_conf);
    PosHint grant(const PeerId& peer, PosHint hint);
    void grant(const PeerId& peer);
    bool granted() const { return _quorum <= 0 && _old_quorum <= 0; }

   private:
    struct UnfoundPeerId {
        UnfoundPeerId(const PeerId& peer_id) : peer_id(peer_id), found(false) {}
        PeerId peer_id;
        bool found;
        bool operator==(const PeerId& id) const { return peer_id == id; }
    };

    using UnfoundPeerIds = std::vector<UnfoundPeerId>;
    using UnfoundPeerIter = UnfoundPeerIds::iterator;

    static UnfoundPeerIter find_peer(const PeerId& peer, UnfoundPeerIds& peers,
                                     size_t pos_hint) {
        if (pos_hint >= peers.size() || peers[pos_hint].peer_id != peer) {
            return std::find(peers.begin(), peers.end(), peer);
        }
        return peers.begin() + pos_hint;
    }

    UnfoundPeerIds _peers;
    int _quorum;
    UnfoundPeerIds _old_peers;
    int _old_quorum;
};

};  // namespace braft
