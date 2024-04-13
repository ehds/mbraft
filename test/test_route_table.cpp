#include <gflags/gflags.h>

#include <vector>

#include "../test/util.h"
#include "braft/configuration.h"
#include "braft/route_table.h"
#include "common.h"

class RouteTableTest : public testing::Test {
  void SetUp() {
    GFLAGS_NS::SetCommandLineOption("minloglevel", "1");

    ::system("rm -rf data");
  }
  void TearDown() { ::system("rm -rf data"); }
};

TEST_F(RouteTableTest, BasicTest) {
    std::string group = "unittest";
    std::vector<braft::PeerId> peers;
    Configuration conf;
    for (int i = 0; i < 3; i++) {
        braft::PeerId peer;
        peer.addr.ip = butil::my_ip();
        peer.addr.port = 5011 + i;
        peer.idx = 0;
        peers.push_back(peer);
        conf.add_peer(peer);
    }

    // start cluster
    Cluster cluster(group, peers);
    for (size_t i = 0; i < peers.size(); i++) {
        ASSERT_EQ(0, cluster.start(peers[i].addr));
    }
    cluster.wait_leader();
    braft::Node* leader = cluster.leader();

    PeerId leader_id;
    braft::rtb::update_configuration("unittest", conf);
    braft::rtb::select_leader(group, &leader_id);
    ASSERT_EQ(PeerId(), leader_id); // unvalid peerid. 

    braft::rtb::refresh_leader(group, 3000 /*timeout ms*/);
    braft::rtb::select_leader(group, &leader_id);
    ASSERT_EQ(leader->node_id().peer_id, leader_id); // leader id.

    braft::rtb::update_leader(group, peers[1]);
    braft::rtb::select_leader(group, &leader_id);
    ASSERT_EQ(peers[1], leader_id); // leader id.

    braft::rtb::remove_group(group);
    ASSERT_EQ(braft::rtb::select_leader(group, &leader_id), -1);
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  GFLAGS_NS::SetCommandLineOption("minloglevel", "1");
  GFLAGS_NS::ParseCommandLineFlags(&argc, &argv, true);
  return RUN_ALL_TESTS();
}
