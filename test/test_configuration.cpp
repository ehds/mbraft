/*
 * =====================================================================================
 *
 *       Filename:  test_configuration.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2015年10月22日 15时16分31秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  WangYao (fisherman), wangyao02@baidu.com
 *        Company:  Baidu, Inc
 *
 * =====================================================================================
 */

#include <butil/logging.h>

#include "braft/configuration.h"
#include "braft/configuration_manager.h"
#include "braft/log_entry.h"
#include "braft/raft.h"
#include "common.h"

namespace braft {
namespace test {

class TestUsageSuits : public testing::Test {
   protected:
    void SetUp() {}
    void TearDown() {}
};

TEST_F(TestUsageSuits, PeerId) {
    PeerId id1;
    ASSERT_TRUE(id1.is_empty());

    ASSERT_NE(0, id1.parse("1.1.1.1::"));
    ASSERT_TRUE(id1.is_empty());

    ASSERT_EQ(0, id1.parse("1.1.1.1:1000:"));
    LOG(INFO) << "id:" << id1.to_string();
    LOG(INFO) << "id:" << id1;

    ASSERT_EQ(0, id1.parse("1.1.1.1:1000:0"));
    LOG(INFO) << "id:" << id1.to_string();
    LOG(INFO) << "id:" << id1;

    ASSERT_EQ(0, id1.parse("1.1.1.1:1000:0:0"));
    LOG(INFO) << "id:" << id1.to_string();
    LOG(INFO) << "id:" << id1;
    ASSERT_FALSE(id1.is_witness());

    ASSERT_EQ(0, id1.parse("1.1.1.1:1000:0:1"));
    LOG(INFO) << "id:" << id1.to_string();
    LOG(INFO) << "id:" << id1;
    ASSERT_TRUE(id1.is_witness());

    ASSERT_EQ(-1, id1.parse("1.1.1.1:1000:0:2"));

    ASSERT_EQ(0, id1.parse("1.1.1.1:1000"));
    LOG(INFO) << "id:" << id1.to_string();
    LOG(INFO) << "id:" << id1;

    PeerId id2(id1);
    LOG(INFO) << "id:" << id2;

    PeerId id3("1.2.3.4:1000:0");
    LOG(INFO) << "id:" << id3;
}

TEST_F(TestUsageSuits, Configuration) {
    Configuration conf1;
    ASSERT_TRUE(conf1.empty());
    std::vector<PeerId> peers;
    peers.push_back({"1.1.1.1:1000:0"});
    peers.emplace_back("1.1.1.1:1000:0");
    peers.emplace_back("1.1.1.1:1000:1");
    peers.emplace_back("1.1.1.1:1000:2");
    conf1 = peers;
    LOG(INFO) << conf1;

    ASSERT_TRUE(conf1.contains({"1.1.1.1:1000:0"}));
    ASSERT_FALSE(conf1.contains({"1.1.1.1:2000:0"}));

    std::vector<PeerId> peers2;
    peers2.emplace_back("1.1.1.1:1000:0");
    peers2.emplace_back("1.1.1.1:1000:1");
    ASSERT_TRUE(conf1.contains(peers2));
    peers2.emplace_back("1.1.1.1:2000:1");
    ASSERT_FALSE(conf1.contains(peers2));

    ASSERT_FALSE(conf1.equals(peers2));
    ASSERT_TRUE(conf1.equals(peers));

    Configuration conf2(peers);
    conf2.remove_peer({"1.1.1.1:1000:1"});
    conf2.add_peer({"1.1.1.1:1000:3"});
    ASSERT_FALSE(conf2.contains({"1.1.1.1:1000:1"}));
    ASSERT_TRUE(conf2.contains({"1.1.1.1:1000:3"}));

    std::set<PeerId> peer_set;
    conf2.list_peers(&peer_set);
    ASSERT_EQ(peer_set.size(), 3);
    std::vector<PeerId> peer_vector;
    conf2.list_peers(&peer_vector);
    ASSERT_EQ(peer_vector.size(), 3);

    Configuration conf3;
    ASSERT_EQ(conf3.parse_from("1.1.1.1:1000:1,1.1.1.1:1000:2,1.1.1.1:1000:3"),
              0);
    std::set<PeerId> peer_set3;
    conf2.list_peers(&peer_set3);
    ASSERT_EQ(peer_set.size(), 3);
    
    // invalid format.
    Configuration conf4;
    ASSERT_EQ(conf4.parse_from("1.1,1.1:100,1.1.1:100:3,aaabbbccc"), -1);
}

TEST_F(TestUsageSuits, ConfigurationManager) {
    ConfigurationManager conf_manager;

    ConfigurationEntry it1;
    conf_manager.get(10, &it1);
    ASSERT_EQ(it1.id, LogId(0, 0));
    ASSERT_TRUE(it1.conf.empty());
    ASSERT_EQ(LogId(0, 0), conf_manager.last_configuration().id);
    ConfigurationEntry entry;
    std::vector<PeerId> peers;
    peers.emplace_back("1.1.1.1:1000:0");
    peers.emplace_back("1.1.1.1:1000:1");
    peers.emplace_back("1.1.1.1:1000:2");
    entry.conf = peers;
    entry.id = {8, 1};
    conf_manager.add(entry);
    ASSERT_EQ(LogId(8, 1), conf_manager.last_configuration().id);

    conf_manager.get(10, &it1);
    ASSERT_EQ(it1.id, entry.id);

    conf_manager.truncate_suffix(7);
    ASSERT_EQ(LogId(0, 0), conf_manager.last_configuration().id);

    entry.id = LogId(10, 1);
    entry.conf = peers;
    conf_manager.add(entry);
    peers.emplace_back("1.1.1.1:1000:3");
    entry.id = LogId(20, 1);
    entry.conf = peers;
    conf_manager.add(entry);
    ASSERT_EQ(LogId(20, 1), conf_manager.last_configuration().id);

    conf_manager.truncate_prefix(15);
    ASSERT_EQ(LogId(20, 1), conf_manager.last_configuration().id);

    conf_manager.truncate_prefix(25);
    ASSERT_EQ(LogId(0, 0), conf_manager.last_configuration().id);
}
}  // namespace test
}  // namespace braft
