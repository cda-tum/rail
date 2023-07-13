#include "Definitions.hpp"
#include "datastructure/RailwayNetwork.hpp"
#include "datastructure/Route.hpp"
#include "datastructure/Timetable.hpp"
#include "nlohmann/json.hpp"

#include "gtest/gtest.h"
#include <algorithm>

using json = nlohmann::json;

struct EdgeTarget {
  std::string source;
  std::string target;
  double      length;
  double      max_speed;
  bool        breakable;
  double      min_block_length;
};

TEST(Functionality, NetworkFunctions) {
  cda_rail::Network network;
  int v0 = network.add_vertex("v0", cda_rail::VertexType::NoBorder);
  int v1 = network.add_vertex("v1", cda_rail::VertexType::VSS);
  int v2 = network.add_vertex("v2", cda_rail::VertexType::TTD);

  int e0 = network.add_edge("v0", "v1", 1, 2, false, 0);
  int e1 = network.add_edge("v1", "v2", 3, 4, true, 1.5);
  int e2 = network.add_edge("v1", "v0", 1, 2, false, 0);
  int e3 = network.add_edge("v2", "v0", 10, 20, true, 2);

  network.add_successor(network.get_edge_index("v0", "v1"),
                        network.get_edge_index("v1", "v2"));
  network.add_successor(network.get_edge_index("v2", "v0"),
                        network.get_edge_index("v0", "v1"));

  // check vertex indices
  EXPECT_EQ(network.get_vertex_index("v0"), v0);
  EXPECT_EQ(network.get_vertex_index("v1"), v1);
  EXPECT_EQ(network.get_vertex_index("v2"), v2);

  // check edge indices
  EXPECT_EQ(network.get_edge_index("v0", "v1"), e0);
  EXPECT_EQ(network.get_edge_index("v1", "v2"), e1);
  EXPECT_EQ(network.get_edge_index("v1", "v0"), e2);
  EXPECT_EQ(network.get_edge_index("v2", "v0"), e3);

  // get Vertex tests
  EXPECT_EQ(network.get_vertex(0).name, "v0");
  EXPECT_EQ(network.get_vertex("v0").name, "v0");
  EXPECT_EQ(network.get_vertex_index("v0"), 0);

  // get Edge tests
  EXPECT_EQ(network.get_edge(0).source, 0);
  EXPECT_EQ(network.get_edge(0).target, 1);
  EXPECT_EQ(network.get_edge(0, 1).source, 0);
  EXPECT_EQ(network.get_edge(0, 1).target, 1);
  EXPECT_EQ(network.get_edge("v0", "v1").source, 0);
  EXPECT_EQ(network.get_edge("v0", "v1").target, 1);
  EXPECT_EQ(network.get_edge_index(0, 1), 0);
  EXPECT_EQ(network.get_edge_index("v0", "v1"), 0);

  // has vertex tests
  EXPECT_TRUE(network.has_vertex(0));
  EXPECT_FALSE(network.has_vertex(3));
  EXPECT_TRUE(network.has_vertex("v0"));
  EXPECT_FALSE(network.has_vertex("v3"));

  // has edge tests
  EXPECT_TRUE(network.has_edge(0));
  EXPECT_FALSE(network.has_edge(4));
  EXPECT_TRUE(network.has_edge(0, 1));
  EXPECT_FALSE(network.has_edge(0, 2));
  EXPECT_TRUE(network.has_edge("v0", "v1"));
  EXPECT_FALSE(network.has_edge("v0", "v2"));

  // Maximum number of VSS test
  EXPECT_EQ(network.max_vss_on_edge(e0), 0);
  EXPECT_EQ(network.max_vss_on_edge(e1), 2);
  EXPECT_EQ(network.max_vss_on_edge(e2), 0);
  EXPECT_EQ(network.max_vss_on_edge(e3), 5);

  // change vertex name tests
  network.change_vertex_name(0, "v0_tmp");
  EXPECT_EQ(network.get_vertex(0).name, "v0_tmp");
  EXPECT_EQ(network.get_vertex("v0_tmp").name, "v0_tmp");
  EXPECT_EQ(network.get_vertex_index("v0_tmp"), 0);
  EXPECT_FALSE(network.has_vertex("v0"));
  EXPECT_TRUE(network.has_vertex("v0_tmp"));
  network.change_vertex_name("v0_tmp", "v0");
  EXPECT_EQ(network.get_vertex(0).name, "v0");
  EXPECT_EQ(network.get_vertex("v0").name, "v0");
  EXPECT_EQ(network.get_vertex_index("v0"), 0);
  EXPECT_FALSE(network.has_vertex("v0_tmp"));
  EXPECT_TRUE(network.has_vertex("v0"));

  // change edge properties tests
  network.change_edge_property(0, 2, "length");
  EXPECT_EQ(network.get_edge(0).length, 2);
  network.change_edge_property(0, 3, "max_speed");
  EXPECT_EQ(network.get_edge(0).max_speed, 3);
  network.change_edge_property(0, 4, "min_block_length");
  EXPECT_EQ(network.get_edge(0).min_block_length, 4);
  network.change_edge_property(0, 1, 5, "length");
  EXPECT_EQ(network.get_edge(0).length, 5);
  network.change_edge_property(0, 1, 6, "max_speed");
  EXPECT_EQ(network.get_edge(0).max_speed, 6);
  network.change_edge_property(0, 1, 7, "min_block_length");
  EXPECT_EQ(network.get_edge(0).min_block_length, 7);
  network.change_edge_property("v0", "v1", 8, "length");
  EXPECT_EQ(network.get_edge(0).length, 8);
  network.change_edge_property("v0", "v1", 9, "max_speed");
  EXPECT_EQ(network.get_edge(0).max_speed, 9);
  network.change_edge_property("v0", "v1", 10, "min_block_length");
  EXPECT_EQ(network.get_edge(0).min_block_length, 10);
  network.change_edge_breakable(1, true);
  EXPECT_TRUE(network.get_edge(1).breakable);
  network.change_edge_breakable(1, 2, false);
  EXPECT_FALSE(network.get_edge(1).breakable);
  network.change_edge_breakable("v1", "v2", true);
  EXPECT_TRUE(network.get_edge(1).breakable);

  // out and in edges tests
  std::vector<int> expected_out{1, 2};
  std::vector<int> expected_in{0};
  std::vector<int> expected_neighbors{0, 2};
  auto             out_edges_1 = network.out_edges(1);
  std::sort(out_edges_1.begin(), out_edges_1.end());
  EXPECT_EQ(out_edges_1, expected_out);
  auto out_edges_v1 = network.out_edges("v1");
  std::sort(out_edges_v1.begin(), out_edges_v1.end());
  EXPECT_EQ(out_edges_v1, expected_out);
  auto in_edges_1 = network.in_edges(1);
  std::sort(in_edges_1.begin(), in_edges_1.end());
  EXPECT_EQ(in_edges_1, expected_in);
  auto in_edges_v1 = network.in_edges("v1");
  std::sort(in_edges_v1.begin(), in_edges_v1.end());
  EXPECT_EQ(in_edges_v1, expected_in);
  auto neighbors_1 = network.neighbors(1);
  std::sort(neighbors_1.begin(), neighbors_1.end());
  EXPECT_EQ(neighbors_1, expected_neighbors);
  auto neighbors_v1 = network.neighbors("v1");
  std::sort(neighbors_v1.begin(), neighbors_v1.end());
  EXPECT_EQ(neighbors_v1, expected_neighbors);

  // successor tests
  std::vector<int> expected_successors{1};
  EXPECT_EQ(network.get_successors(0), expected_successors);
  EXPECT_EQ(network.get_successors(0, 1), expected_successors);
  EXPECT_EQ(network.get_successors("v0", "v1"), expected_successors);

  // Vertex and edge numbers
  EXPECT_EQ(network.number_of_vertices(), 3);
  EXPECT_EQ(network.number_of_edges(), 4);

  // Valid successor
  EXPECT_TRUE(network.is_valid_successor(0, 1));
  EXPECT_FALSE(network.is_valid_successor(0, 2));
}

TEST(Functionality, NetworkSections) {
  cda_rail::Network network;

  // Add vertices
  network.add_vertex("v0", cda_rail::VertexType::TTD);
  network.add_vertex("v1", cda_rail::VertexType::NoBorder);
  network.add_vertex("v20", cda_rail::VertexType::TTD);
  network.add_vertex("v21", cda_rail::VertexType::NoBorder);
  network.add_vertex("v30", cda_rail::VertexType::NoBorder);
  network.add_vertex("v31", cda_rail::VertexType::VSS);
  network.add_vertex("v4", cda_rail::VertexType::TTD);
  network.add_vertex("v5", cda_rail::VertexType::VSS);
  network.add_vertex("v6", cda_rail::VertexType::NoBorderVSS);
  network.add_vertex("v7", cda_rail::VertexType::TTD);

  // Add edges v0 -> v1 -> v20 -> v30 -> v4 -> v5 -> v6 -> v7
  // All unbreakable other properties not relevant
  int v0_v1   = network.add_edge("v0", "v1", 1, 1, false);
  int v1_v20  = network.add_edge("v1", "v20", 1, 1, false);
  int v20_v30 = network.add_edge("v20", "v30", 1, 1, false);
  int v30_v4  = network.add_edge("v30", "v4", 1, 1, false);
  int v4_v5   = network.add_edge("v4", "v5", 1, 1, false);
  int v5_v6   = network.add_edge("v5", "v6", 1, 1, false);
  int v6_v7   = network.add_edge("v6", "v7", 1, 1, false);

  // Add edges v7 -> v6 -> v5 -> v4 -> v31 -> v21 -> v1 -> v0
  // v4 -> v31 breakable, all other unbreakable
  int v7_v6 = network.add_edge("v7", "v6", 1, 1, false);
  int v6_v5 = network.add_edge("v6", "v5", 1, 1, false);
  int v5_v4 = network.add_edge("v5", "v4", 1, 1, false);
  network.add_edge("v4", "v31", 1, 1, true);
  int v31_v21 = network.add_edge("v31", "v21", 1, 1, false);
  int v21_v1  = network.add_edge("v21", "v1", 1, 1, false);
  int v1_v0   = network.add_edge("v1", "v0", 1, 1, false);

  auto no_border_vss_sections = network.no_border_vss_sections();

  // Check non_border_vss_sections
  // There should be 1 section
  EXPECT_EQ(no_border_vss_sections.size(), 1);
  // The section should contain 4 edges, namely v5 -> v6 -> v7 and the reverse
  EXPECT_EQ(no_border_vss_sections[0].size(), 4);
  EXPECT_TRUE(std::find(no_border_vss_sections[0].begin(),
                        no_border_vss_sections[0].end(),
                        v5_v6) != no_border_vss_sections[0].end());
  EXPECT_TRUE(std::find(no_border_vss_sections[0].begin(),
                        no_border_vss_sections[0].end(),
                        v6_v7) != no_border_vss_sections[0].end());
  EXPECT_TRUE(std::find(no_border_vss_sections[0].begin(),
                        no_border_vss_sections[0].end(),
                        v7_v6) != no_border_vss_sections[0].end());
  EXPECT_TRUE(std::find(no_border_vss_sections[0].begin(),
                        no_border_vss_sections[0].end(),
                        v6_v5) != no_border_vss_sections[0].end());

  std::pair<int, int> pair1 = std::make_pair(v5_v6, v6_v5);
  std::pair<int, int> pair2 = std::make_pair(v6_v7, v7_v6);
  EXPECT_TRUE(network.common_vertex(pair1, pair2) ==
              network.get_vertex_index("v6"));

  auto unbreakable_sections = network.unbreakable_sections();

  // Check unbreakable_sections
  // There should be 3 sections
  EXPECT_EQ(unbreakable_sections.size(), 3);
  // One section should contain v0_v1, one should contain v20_v30, and one
  // should contain v4_v5
  int s0 = -1, s1 = -1, s2 = -1;
  for (int i = 0; i < unbreakable_sections.size(); i++) {
    if (std::find(unbreakable_sections[i].begin(),
                  unbreakable_sections[i].end(),
                  v0_v1) != unbreakable_sections[i].end()) {
      s0 = i;
    }
    if (std::find(unbreakable_sections[i].begin(),
                  unbreakable_sections[i].end(),
                  v20_v30) != unbreakable_sections[i].end()) {
      s1 = i;
    }
    if (std::find(unbreakable_sections[i].begin(),
                  unbreakable_sections[i].end(),
                  v4_v5) != unbreakable_sections[i].end()) {
      s2 = i;
    }
  }
  // s0, s1 and s2 should be all different and within [0, 2]
  EXPECT_NE(s0, s1);
  EXPECT_NE(s0, s2);
  EXPECT_NE(s1, s2);
  EXPECT_GE(s0, 0);
  EXPECT_GE(s1, 0);
  EXPECT_GE(s2, 0);
  EXPECT_LE(s0, 2);
  EXPECT_LE(s1, 2);
  EXPECT_LE(s2, 2);

  // Section s0 should contain 5 edges, namely v0 -> v1, v1 -> v20, v31 -> v21,
  // v21 -> v1, v1 -> v0
  EXPECT_EQ(unbreakable_sections[s0].size(), 5);
  EXPECT_TRUE(std::find(unbreakable_sections[s0].begin(),
                        unbreakable_sections[s0].end(),
                        v0_v1) != unbreakable_sections[s0].end());
  EXPECT_TRUE(std::find(unbreakable_sections[s0].begin(),
                        unbreakable_sections[s0].end(),
                        v1_v20) != unbreakable_sections[s0].end());
  EXPECT_TRUE(std::find(unbreakable_sections[s0].begin(),
                        unbreakable_sections[s0].end(),
                        v31_v21) != unbreakable_sections[s0].end());
  EXPECT_TRUE(std::find(unbreakable_sections[s0].begin(),
                        unbreakable_sections[s0].end(),
                        v21_v1) != unbreakable_sections[s0].end());
  EXPECT_TRUE(std::find(unbreakable_sections[s0].begin(),
                        unbreakable_sections[s0].end(),
                        v1_v0) != unbreakable_sections[s0].end());

  // Section s1 should contain 2 edges, namely v20 -> v30 -> v4
  EXPECT_EQ(unbreakable_sections[s1].size(), 2);
  EXPECT_TRUE(std::find(unbreakable_sections[s1].begin(),
                        unbreakable_sections[s1].end(),
                        v20_v30) != unbreakable_sections[s1].end());
  EXPECT_TRUE(std::find(unbreakable_sections[s1].begin(),
                        unbreakable_sections[s1].end(),
                        v30_v4) != unbreakable_sections[s1].end());

  // Section s2 should contain 2 edges, namely v4 -> v5 and the reverse
  EXPECT_EQ(unbreakable_sections[s2].size(), 2);
  EXPECT_TRUE(std::find(unbreakable_sections[s2].begin(),
                        unbreakable_sections[s2].end(),
                        v4_v5) != unbreakable_sections[s2].end());
  EXPECT_TRUE(std::find(unbreakable_sections[s2].begin(),
                        unbreakable_sections[s2].end(),
                        v5_v4) != unbreakable_sections[s2].end());
}

TEST(Functionality, NetworkConsistency) {
  cda_rail::Network network;

  // Add vertices
  network.add_vertex("v0", cda_rail::VertexType::TTD);
  network.add_vertex("v1", cda_rail::VertexType::NoBorderVSS);
  network.add_vertex("v2", cda_rail::VertexType::TTD);
  network.add_vertex("v3", cda_rail::VertexType::VSS);

  network.add_edge("v0", "v1", 100, 100, false);
  network.add_edge("v1", "v2", 100, 100, false);
  network.add_edge("v1", "v3", 100, 100, false);

  EXPECT_FALSE(network.is_consistent_for_transformation());

  network.change_vertex_type("v1", cda_rail::VertexType::NoBorder);

  EXPECT_TRUE(network.is_consistent_for_transformation());

  network.add_vertex("v4", cda_rail::VertexType::NoBorder);
  network.add_vertex("v5", cda_rail::VertexType::NoBorderVSS);
  network.add_vertex("v6", cda_rail::VertexType::VSS);

  network.add_edge("v2", "v4", 100, 100, false);
  network.add_edge("v4", "v5", 100, 100, false);
  network.add_edge("v5", "v6", 100, 100, false);

  EXPECT_FALSE(network.is_consistent_for_transformation());

  network.change_vertex_type("v5", cda_rail::VertexType::NoBorder);

  EXPECT_TRUE(network.is_consistent_for_transformation());

  network.add_vertex("v7", cda_rail::VertexType::TTD);

  network.add_edge("v6", "v7", 100, 100, true, 0);

  EXPECT_FALSE(network.is_consistent_for_transformation());

  network.change_edge_property("v6", "v7", 1, "min_block_length");

  EXPECT_TRUE(network.is_consistent_for_transformation());

  network.change_vertex_type("v7", cda_rail::VertexType::NoBorder);

  EXPECT_FALSE(network.is_consistent_for_transformation());

  network.change_vertex_type("v7", cda_rail::VertexType::VSS);

  EXPECT_TRUE(network.is_consistent_for_transformation());

  network.add_vertex("v8", cda_rail::VertexType::TTD);

  network.add_edge("v7", "v8", 100, 100, false);
  network.add_edge("v8", "v7", 50, 50, false);

  EXPECT_FALSE(network.is_consistent_for_transformation());

  network.change_edge_property("v8", "v7", 100, "length");

  EXPECT_TRUE(network.is_consistent_for_transformation());

  network.change_edge_breakable("v8", "v7", true);

  EXPECT_FALSE(network.is_consistent_for_transformation());
}

TEST(Functionality, ReadNetwork) {
  cda_rail::Network network = cda_rail::Network::import_network(
      "./example-networks/SimpleStation/network/");

  // Check vertices properties
  std::vector<std::string> vertex_names = {
      "l0", "l1", "l2", "l3", "r0", "r1", "r2", "g00", "g01", "g10", "g11"};
  std::vector<cda_rail::VertexType> type = {
      cda_rail::VertexType::TTD,      cda_rail::VertexType::TTD,
      cda_rail::VertexType::TTD,      cda_rail::VertexType::NoBorder,
      cda_rail::VertexType::TTD,      cda_rail::VertexType::TTD,
      cda_rail::VertexType::NoBorder, cda_rail::VertexType::TTD,
      cda_rail::VertexType::TTD,      cda_rail::VertexType::TTD,
      cda_rail::VertexType::TTD};

  EXPECT_EQ(network.number_of_vertices(), vertex_names.size());

  for (int i = 0; i < vertex_names.size(); i++) {
    std::string      v_name = vertex_names[i];
    cda_rail::Vertex v      = network.get_vertex(v_name);
    EXPECT_EQ(v.name, v_name);
    EXPECT_EQ(v.type, type[i]);
  }

  // Check edges properties
  std::vector<EdgeTarget> edge_targets;
  edge_targets.push_back({"l0", "l1", 500, 27.77777777777778, true, 10});
  edge_targets.push_back({"l1", "l2", 500, 27.77777777777778, true, 10});
  edge_targets.push_back({"l2", "l3", 5, 27.77777777777778, false, 0});
  edge_targets.push_back({"l3", "g00", 5, 27.77777777777778, false, 0});
  edge_targets.push_back({"l3", "g10", 5, 27.77777777777778, false, 0});
  edge_targets.push_back({"g00", "g01", 300, 27.77777777777778, true, 10});
  edge_targets.push_back({"g10", "g11", 300, 27.77777777777778, true, 10});
  edge_targets.push_back({"g01", "r2", 5, 27.77777777777778, false, 0});
  edge_targets.push_back({"g11", "r2", 5, 27.77777777777778, false, 0});
  edge_targets.push_back({"r2", "r1", 5, 27.77777777777778, false, 0});
  edge_targets.push_back({"r1", "r0", 500, 27.77777777777778, true, 10});
  edge_targets.push_back({"r0", "r1", 500, 27.77777777777778, true, 10});
  edge_targets.push_back({"r1", "r2", 5, 27.77777777777778, false, 0});
  edge_targets.push_back({"r2", "g01", 5, 27.77777777777778, false, 0});
  edge_targets.push_back({"r2", "g11", 5, 27.77777777777778, false, 0});
  edge_targets.push_back({"g01", "g00", 300, 27.77777777777778, true, 10});
  edge_targets.push_back({"g11", "g10", 300, 27.77777777777778, true, 10});
  edge_targets.push_back({"g00", "l3", 5, 27.77777777777778, false, 0});
  edge_targets.push_back({"g10", "l3", 5, 27.77777777777778, false, 0});
  edge_targets.push_back({"l3", "l2", 5, 27.77777777777778, false, 0});
  edge_targets.push_back({"l2", "l1", 500, 27.77777777777778, true, 10});
  edge_targets.push_back({"l1", "l0", 500, 27.77777777777778, true, 10});

  EXPECT_EQ(network.number_of_edges(), edge_targets.size());
  for (const auto& edge : edge_targets) {
    cda_rail::Edge e = network.get_edge(edge.source, edge.target);
    EXPECT_EQ(network.get_vertex(e.source).name, edge.source);
    EXPECT_EQ(network.get_vertex(e.target).name, edge.target);
    EXPECT_EQ(e.length, edge.length);
    EXPECT_EQ(e.max_speed, edge.max_speed);
    EXPECT_EQ(e.breakable, edge.breakable);
    EXPECT_EQ(e.min_block_length, edge.min_block_length);
  }

  // Check successors
  std::vector<int> successors_target;

  // l0,l1
  successors_target.clear();
  successors_target.emplace_back(network.get_edge_index("l1", "l2"));
  EXPECT_EQ(network.get_successors("l0", "l1"), successors_target);

  // l1,l2
  successors_target.clear();
  successors_target.emplace_back(network.get_edge_index("l2", "l3"));
  EXPECT_EQ(network.get_successors("l1", "l2"), successors_target);

  // l2,l3
  successors_target.clear();
  successors_target.emplace_back(network.get_edge_index("l3", "g00"));
  successors_target.emplace_back(network.get_edge_index("l3", "g10"));
  std::sort(successors_target.begin(), successors_target.end());
  auto successors_l2_l3 = network.get_successors("l2", "l3");
  std::sort(successors_l2_l3.begin(), successors_l2_l3.end());
  EXPECT_EQ(successors_l2_l3, successors_target);

  // l3,g00
  successors_target.clear();
  successors_target.emplace_back(network.get_edge_index("g00", "g01"));
  EXPECT_EQ(network.get_successors("l3", "g00"), successors_target);

  // l3,g10
  successors_target.clear();
  successors_target.emplace_back(network.get_edge_index("g10", "g11"));
  EXPECT_EQ(network.get_successors("l3", "g10"), successors_target);

  // g00,g01
  successors_target.clear();
  successors_target.emplace_back(network.get_edge_index("g01", "r2"));
  EXPECT_EQ(network.get_successors("g00", "g01"), successors_target);

  // g10,g11
  successors_target.clear();
  successors_target.emplace_back(network.get_edge_index("g11", "r2"));
  EXPECT_EQ(network.get_successors("g10", "g11"), successors_target);

  // g01,r2
  successors_target.clear();
  successors_target.emplace_back(network.get_edge_index("r2", "r1"));
  EXPECT_EQ(network.get_successors("g01", "r2"), successors_target);

  // g11,r2
  successors_target.clear();
  successors_target.emplace_back(network.get_edge_index("r2", "r1"));
  EXPECT_EQ(network.get_successors("g11", "r2"), successors_target);

  // r2,r1
  successors_target.clear();
  successors_target.emplace_back(network.get_edge_index("r1", "r0"));
  EXPECT_EQ(network.get_successors("r2", "r1"), successors_target);

  // r1,r0
  successors_target.clear();
  EXPECT_EQ(network.get_successors("r1", "r0"), successors_target);

  // r0,r1
  successors_target.clear();
  successors_target.emplace_back(network.get_edge_index("r1", "r2"));
  EXPECT_EQ(network.get_successors("r0", "r1"), successors_target);

  // r1,r2
  successors_target.clear();
  successors_target.emplace_back(network.get_edge_index("r2", "g01"));
  successors_target.emplace_back(network.get_edge_index("r2", "g11"));
  std::sort(successors_target.begin(), successors_target.end());
  auto successors_r1_r2 = network.get_successors("r1", "r2");
  std::sort(successors_r1_r2.begin(), successors_r1_r2.end());
  EXPECT_EQ(successors_r1_r2, successors_target);

  // r2,g01
  successors_target.clear();
  successors_target.emplace_back(network.get_edge_index("g01", "g00"));
  EXPECT_EQ(network.get_successors("r2", "g01"), successors_target);

  // r2,g11
  successors_target.clear();
  successors_target.emplace_back(network.get_edge_index("g11", "g10"));
  EXPECT_EQ(network.get_successors("r2", "g11"), successors_target);

  // g01,g00
  successors_target.clear();
  successors_target.emplace_back(network.get_edge_index("g00", "l3"));
  EXPECT_EQ(network.get_successors("g01", "g00"), successors_target);

  // g11,g10
  successors_target.clear();
  successors_target.emplace_back(network.get_edge_index("g10", "l3"));
  EXPECT_EQ(network.get_successors("g11", "g10"), successors_target);

  // g00,l3
  successors_target.clear();
  successors_target.emplace_back(network.get_edge_index("l3", "l2"));
  EXPECT_EQ(network.get_successors("g00", "l3"), successors_target);

  // g10,l3
  successors_target.clear();
  successors_target.emplace_back(network.get_edge_index("l3", "l2"));
  EXPECT_EQ(network.get_successors("g10", "l3"), successors_target);

  // l3,l2
  successors_target.clear();
  successors_target.emplace_back(network.get_edge_index("l2", "l1"));
  EXPECT_EQ(network.get_successors("l3", "l2"), successors_target);

  // l2,l1
  successors_target.clear();
  successors_target.emplace_back(network.get_edge_index("l1", "l0"));
  EXPECT_EQ(network.get_successors("l2", "l1"), successors_target);

  // l1,l0
  successors_target.clear();
  EXPECT_EQ(network.get_successors("l1", "l0"), successors_target);
}

TEST(Functionality, WriteNetwork) {
  cda_rail::Network network;
  network.add_vertex("v0", cda_rail::VertexType::NoBorder);
  network.add_vertex("v1", cda_rail::VertexType::VSS);
  network.add_vertex("v2", cda_rail::VertexType::TTD);

  network.add_edge("v0", "v1", 1, 2, true, 0);
  network.add_edge("v1", "v2", 3, 4, false, 1.5);
  network.add_edge("v1", "v0", 1, 2, true, 0);
  network.add_edge("v2", "v0", 10, 20, false, 2);

  network.add_successor(network.get_edge_index("v0", "v1"),
                        network.get_edge_index("v1", "v2"));
  network.add_successor(network.get_edge_index("v0", "v1"),
                        network.get_edge_index("v1", "v0"));
  network.add_successor(network.get_edge_index("v2", "v0"),
                        network.get_edge_index("v0", "v1"));

  network.export_network("./tmp/write_network_test");

  auto network_read =
      cda_rail::Network::import_network("./tmp/write_network_test");

  // Delete created directory and everything in it
  std::filesystem::remove_all("./tmp");

  // check if both networks are equivalent

  // check vertices
  EXPECT_EQ(network.number_of_vertices(), network_read.number_of_vertices());
  for (int i = 0; i < network.number_of_vertices(); ++i) {
    EXPECT_TRUE(network_read.has_vertex(network.get_vertex(i).name));
    EXPECT_EQ(network_read.get_vertex(network.get_vertex(i).name).type,
              network.get_vertex(i).type);
  }

  // check edges
  EXPECT_EQ(network.number_of_edges(), network_read.number_of_edges());
  for (int i = 0; i < network.number_of_edges(); ++i) {
    const auto& source_vertex = network.get_vertex(network.get_edge(i).source);
    const auto& target_vertex = network.get_vertex(network.get_edge(i).target);
    EXPECT_TRUE(network_read.has_edge(source_vertex.name, target_vertex.name));
    const auto& edge_read =
        network_read.get_edge(source_vertex.name, target_vertex.name);
    EXPECT_EQ(edge_read.breakable, network.get_edge(i).breakable);
    EXPECT_EQ(edge_read.length, network.get_edge(i).length);
    EXPECT_EQ(edge_read.max_speed, network.get_edge(i).max_speed);
    EXPECT_EQ(edge_read.min_block_length, network.get_edge(i).min_block_length);
  }

  // check successors
  for (int i = 0; i < network.number_of_edges(); ++i) {
    const auto&      successors_target = network.get_successors(i);
    std::vector<int> successors_target_transformed;
    for (auto successor : successors_target) {
      const auto& e      = network.get_edge(successor);
      std::string source = network.get_vertex(e.source).name;
      std::string target = network.get_vertex(e.target).name;
      successors_target_transformed.emplace_back(
          network_read.get_edge_index(source, target));
    }
    const auto& e      = network.get_edge(i);
    std::string source = network.get_vertex(e.source).name;
    std::string target = network.get_vertex(e.target).name;
    auto        successors_target_transformed_read =
        network_read.get_successors(source, target);
    std::sort(successors_target_transformed.begin(),
              successors_target_transformed.end());
    std::sort(successors_target_transformed_read.begin(),
              successors_target_transformed_read.end());
    EXPECT_EQ(successors_target_transformed,
              successors_target_transformed_read);
  }
}

TEST(Functionality, NetworkEdgeSeparation) {
  cda_rail::Network network;
  // Add vertices
  network.add_vertex("v00", cda_rail::VertexType::TTD);
  network.add_vertex("v01", cda_rail::VertexType::TTD);
  network.add_vertex("v1", cda_rail::VertexType::TTD);
  network.add_vertex("v2", cda_rail::VertexType::TTD);
  network.add_vertex("v30", cda_rail::VertexType::TTD);
  network.add_vertex("v31", cda_rail::VertexType::TTD);

  // Add edges
  int v00_v1 = network.add_edge("v00", "v1", 100, 100, false);
  int v01_v1 = network.add_edge("v01", "v1", 100, 100, false);
  int v1_v2  = network.add_edge("v1", "v2", 44, 100, true, 10);
  int v2_v30 = network.add_edge("v2", "v30", 100, 100, false);
  int v2_v31 = network.add_edge("v2", "v31", 100, 100, false);

  // Add successors
  network.add_successor(v00_v1, v1_v2);
  network.add_successor(v01_v1, v1_v2);
  network.add_successor(v1_v2, v2_v30);
  network.add_successor(v1_v2, v2_v31);

  // Separate edge v1_v2 uniformly
  auto new_edges =
      network.separate_edge("v1", "v2", cda_rail::SeparationType::UNIFORM);

  // There are 4 new forward edges and no new reverse edges
  EXPECT_EQ(new_edges.first.size(), 4);
  EXPECT_EQ(new_edges.second.size(), 0);

  // Check if the vertices are correct
  // All old vertices still exist. Additionally vertices v1_v2_0 to v1_v2_2 have
  // been added with type NoBorder Hence, in total there are 9 vertices.
  EXPECT_EQ(network.number_of_vertices(), 9);
  EXPECT_TRUE(network.has_vertex("v1_v2_0"));
  EXPECT_TRUE(network.has_vertex("v1_v2_1"));
  EXPECT_TRUE(network.has_vertex("v1_v2_2"));
  EXPECT_EQ(network.get_vertex("v1_v2_0").type,
            cda_rail::VertexType::NoBorderVSS);
  EXPECT_EQ(network.get_vertex("v1_v2_1").type,
            cda_rail::VertexType::NoBorderVSS);
  EXPECT_EQ(network.get_vertex("v1_v2_2").type,
            cda_rail::VertexType::NoBorderVSS);
  EXPECT_TRUE(network.has_vertex("v00"));
  EXPECT_TRUE(network.has_vertex("v01"));
  EXPECT_TRUE(network.has_vertex("v1"));
  EXPECT_TRUE(network.has_vertex("v2"));
  EXPECT_TRUE(network.has_vertex("v30"));
  EXPECT_TRUE(network.has_vertex("v31"));
  EXPECT_EQ(network.get_vertex("v00").type, cda_rail::VertexType::TTD);
  EXPECT_EQ(network.get_vertex("v01").type, cda_rail::VertexType::TTD);
  EXPECT_EQ(network.get_vertex("v1").type, cda_rail::VertexType::TTD);
  EXPECT_EQ(network.get_vertex("v2").type, cda_rail::VertexType::TTD);
  EXPECT_EQ(network.get_vertex("v30").type, cda_rail::VertexType::TTD);
  EXPECT_EQ(network.get_vertex("v31").type, cda_rail::VertexType::TTD);

  // Check if the edges are correct
  // v00/v01 ->(len: 100) v1 ->(len: 11) v1_v2_0 -> (len: 11) v1_v2_1 -> (len:
  // 11) v1_v2_2 -> (len:11) v2 -> (len: 100) v30/v31
  EXPECT_EQ(network.number_of_edges(), 8);
  EXPECT_TRUE(network.has_edge("v00", "v1"));
  EXPECT_TRUE(network.has_edge("v01", "v1"));
  EXPECT_TRUE(network.has_edge("v1", "v1_v2_0"));
  EXPECT_TRUE(network.has_edge("v1_v2_0", "v1_v2_1"));
  EXPECT_TRUE(network.has_edge("v1_v2_1", "v1_v2_2"));
  EXPECT_TRUE(network.has_edge("v1_v2_2", "v2"));
  EXPECT_TRUE(network.has_edge("v2", "v30"));
  EXPECT_TRUE(network.has_edge("v2", "v31"));
  EXPECT_EQ(network.get_edge("v00", "v1").length, 100);
  EXPECT_EQ(network.get_edge("v01", "v1").length, 100);
  EXPECT_EQ(network.get_edge("v1", "v1_v2_0").length, 11);
  EXPECT_EQ(network.get_edge("v1_v2_0", "v1_v2_1").length, 11);
  EXPECT_EQ(network.get_edge("v1_v2_1", "v1_v2_2").length, 11);
  EXPECT_EQ(network.get_edge("v1_v2_2", "v2").length, 11);
  EXPECT_EQ(network.get_edge("v2", "v30").length, 100);
  EXPECT_EQ(network.get_edge("v2", "v31").length, 100);
  // The other properties are unchanged, except that all edges are unbreakable,
  // i.e., the breakable property is false
  EXPECT_FALSE(network.get_edge("v00", "v1").breakable);
  EXPECT_FALSE(network.get_edge("v01", "v1").breakable);
  EXPECT_FALSE(network.get_edge("v1", "v1_v2_0").breakable);
  EXPECT_FALSE(network.get_edge("v1_v2_0", "v1_v2_1").breakable);
  EXPECT_FALSE(network.get_edge("v1_v2_1", "v1_v2_2").breakable);
  EXPECT_FALSE(network.get_edge("v1_v2_2", "v2").breakable);
  EXPECT_FALSE(network.get_edge("v2", "v30").breakable);
  EXPECT_FALSE(network.get_edge("v2", "v31").breakable);
  EXPECT_EQ(network.get_edge("v00", "v1").max_speed, 100);
  EXPECT_EQ(network.get_edge("v01", "v1").max_speed, 100);
  EXPECT_EQ(network.get_edge("v1", "v1_v2_0").max_speed, 100);
  EXPECT_EQ(network.get_edge("v1_v2_0", "v1_v2_1").max_speed, 100);
  EXPECT_EQ(network.get_edge("v1_v2_1", "v1_v2_2").max_speed, 100);
  EXPECT_EQ(network.get_edge("v1_v2_2", "v2").max_speed, 100);
  EXPECT_EQ(network.get_edge("v2", "v30").max_speed, 100);
  EXPECT_EQ(network.get_edge("v2", "v31").max_speed, 100);
  // The edge v1 -> v2 does not exist anymore
  EXPECT_FALSE(network.has_edge("v1", "v2"));

  // The new edges are v1 -> v1_v2_0 -> v1_v2_1 -> v1_v2_2 -> v2 in this order
  EXPECT_EQ(network.get_edge_index("v1", "v1_v2_0"), new_edges.first[0]);
  EXPECT_EQ(network.get_edge_index("v1_v2_0", "v1_v2_1"), new_edges.first[1]);
  EXPECT_EQ(network.get_edge_index("v1_v2_1", "v1_v2_2"), new_edges.first[2]);
  EXPECT_EQ(network.get_edge_index("v1_v2_2", "v2"), new_edges.first[3]);

  // The last index of the new edges is identical to the old index of v1->v2
  EXPECT_EQ(new_edges.first.back(), v1_v2);

  // v00 has no incoming edges
  EXPECT_EQ(network.in_edges("v00").size(), 0);
  // v01 has no incoming edges
  EXPECT_EQ(network.in_edges("v01").size(), 0);
  // v1 has two incoming edges, namely from v00 and v01
  const auto& in_edges_v1 = network.in_edges("v1");
  EXPECT_EQ(in_edges_v1.size(), 2);
  EXPECT_TRUE(std::find(in_edges_v1.begin(), in_edges_v1.end(),
                        network.get_edge_index("v00", "v1")) !=
              in_edges_v1.end());
  EXPECT_TRUE(std::find(in_edges_v1.begin(), in_edges_v1.end(),
                        network.get_edge_index("v01", "v1")) !=
              in_edges_v1.end());
  // v1_v2_0 has one incoming edge, namely from v1
  const auto& in_edges_v1_v2_0 = network.in_edges("v1_v2_0");
  EXPECT_EQ(in_edges_v1_v2_0.size(), 1);
  EXPECT_TRUE(std::find(in_edges_v1_v2_0.begin(), in_edges_v1_v2_0.end(),
                        network.get_edge_index("v1", "v1_v2_0")) !=
              in_edges_v1_v2_0.end());
  // v1_v2_1 has one incoming edge, namely from v1_v2_0
  const auto& in_edges_v1_v2_1 = network.in_edges("v1_v2_1");
  EXPECT_EQ(in_edges_v1_v2_1.size(), 1);
  EXPECT_TRUE(std::find(in_edges_v1_v2_1.begin(), in_edges_v1_v2_1.end(),
                        network.get_edge_index("v1_v2_0", "v1_v2_1")) !=
              in_edges_v1_v2_1.end());
  // v1_v2_2 has one incoming edge, namely from v1_v2_1
  const auto& in_edges_v1_v2_2 = network.in_edges("v1_v2_2");
  EXPECT_EQ(in_edges_v1_v2_2.size(), 1);
  EXPECT_TRUE(std::find(in_edges_v1_v2_2.begin(), in_edges_v1_v2_2.end(),
                        network.get_edge_index("v1_v2_1", "v1_v2_2")) !=
              in_edges_v1_v2_2.end());
  // v2 has one incoming edge, namely from v1_v2_2
  const auto& in_edges_v2 = network.in_edges("v2");
  EXPECT_EQ(in_edges_v2.size(), 1);
  EXPECT_TRUE(std::find(in_edges_v2.begin(), in_edges_v2.end(),
                        network.get_edge_index("v1_v2_2", "v2")) !=
              in_edges_v2.end());
  // v30 has one incoming edge, namely from v2
  const auto& in_edges_v30 = network.in_edges("v30");
  EXPECT_EQ(in_edges_v30.size(), 1);
  EXPECT_TRUE(std::find(in_edges_v30.begin(), in_edges_v30.end(),
                        network.get_edge_index("v2", "v30")) !=
              in_edges_v30.end());
  // v31 has one incoming edge, namely from v2
  const auto& in_edges_v31 = network.in_edges("v31");
  EXPECT_EQ(in_edges_v31.size(), 1);
  EXPECT_TRUE(std::find(in_edges_v31.begin(), in_edges_v31.end(),
                        network.get_edge_index("v2", "v31")) !=
              in_edges_v31.end());
  // v00 has one outgoing edge, namely to v1
  const auto& out_edges_v00 = network.out_edges("v00");
  EXPECT_EQ(out_edges_v00.size(), 1);
  EXPECT_TRUE(std::find(out_edges_v00.begin(), out_edges_v00.end(),
                        network.get_edge_index("v00", "v1")) !=
              out_edges_v00.end());
  // v01 has one outgoing edge, namely to v1
  const auto& out_edges_v01 = network.out_edges("v01");
  EXPECT_EQ(out_edges_v01.size(), 1);
  EXPECT_TRUE(std::find(out_edges_v01.begin(), out_edges_v01.end(),
                        network.get_edge_index("v01", "v1")) !=
              out_edges_v01.end());
  // v1 has one outgoing edge, namely to v1_v2_0
  const auto& out_edges_v1 = network.out_edges("v1");
  EXPECT_EQ(out_edges_v1.size(), 1);
  EXPECT_TRUE(std::find(out_edges_v1.begin(), out_edges_v1.end(),
                        network.get_edge_index("v1", "v1_v2_0")) !=
              out_edges_v1.end());
  // v1_v2_0 has one outgoing edge, namely to v1_v2_1
  const auto& out_edges_v1_v2_0 = network.out_edges("v1_v2_0");
  EXPECT_EQ(out_edges_v1_v2_0.size(), 1);
  EXPECT_TRUE(std::find(out_edges_v1_v2_0.begin(), out_edges_v1_v2_0.end(),
                        network.get_edge_index("v1_v2_0", "v1_v2_1")) !=
              out_edges_v1_v2_0.end());
  // v1_v2_1 has one outgoing edge, namely to v1_v2_2
  const auto& out_edges_v1_v2_1 = network.out_edges("v1_v2_1");
  EXPECT_EQ(out_edges_v1_v2_1.size(), 1);
  EXPECT_TRUE(std::find(out_edges_v1_v2_1.begin(), out_edges_v1_v2_1.end(),
                        network.get_edge_index("v1_v2_1", "v1_v2_2")) !=
              out_edges_v1_v2_1.end());
  // v1_v2_2 has one outgoing edge, namely to v2
  const auto& out_edges_v1_v2_2 = network.out_edges("v1_v2_2");
  EXPECT_EQ(out_edges_v1_v2_2.size(), 1);
  EXPECT_TRUE(std::find(out_edges_v1_v2_2.begin(), out_edges_v1_v2_2.end(),
                        network.get_edge_index("v1_v2_2", "v2")) !=
              out_edges_v1_v2_2.end());
  // v2 has two outgoing edges, namely to v30 and v31
  const auto& out_edges_v2 = network.out_edges("v2");
  EXPECT_EQ(out_edges_v2.size(), 2);
  EXPECT_TRUE(std::find(out_edges_v2.begin(), out_edges_v2.end(),
                        network.get_edge_index("v2", "v30")) !=
              out_edges_v2.end());
  EXPECT_TRUE(std::find(out_edges_v2.begin(), out_edges_v2.end(),
                        network.get_edge_index("v2", "v31")) !=
              out_edges_v2.end());
  // v30 has no outgoing edges
  EXPECT_TRUE(network.out_edges("v30").empty());
  // v31 has no outgoing edges
  EXPECT_TRUE(network.out_edges("v31").empty());

  // The successors are essentially the same as the outgoing edges in this case
  // v00->v1 has one successor, namely v1->v1_v2_0
  const auto& successors_v00_v1 = network.get_successors("v00", "v1");
  EXPECT_EQ(successors_v00_v1.size(), 1);
  EXPECT_TRUE(std::find(successors_v00_v1.begin(), successors_v00_v1.end(),
                        network.get_edge_index("v1", "v1_v2_0")) !=
              successors_v00_v1.end());
  // v01->v1 has one successor, namely v1->v1_v2_0
  const auto& successors_v01_v1 = network.get_successors("v01", "v1");
  EXPECT_EQ(successors_v01_v1.size(), 1);
  EXPECT_TRUE(std::find(successors_v01_v1.begin(), successors_v01_v1.end(),
                        network.get_edge_index("v1", "v1_v2_0")) !=
              successors_v01_v1.end());
  // v1->v1_v2_0 has one successor, namely v1_v2_0->v1_v2_1
  const auto& successors_v1_v1_v2_0 = network.get_successors("v1", "v1_v2_0");
  EXPECT_EQ(successors_v1_v1_v2_0.size(), 1);
  EXPECT_TRUE(std::find(successors_v1_v1_v2_0.begin(),
                        successors_v1_v1_v2_0.end(),
                        network.get_edge_index("v1_v2_0", "v1_v2_1")) !=
              successors_v1_v1_v2_0.end());
  // v1_v2_0->v1_v2_1 has one successor, namely v1_v2_1->v1_v2_2
  const auto& successors_v1_v2_0_v1_v2_1 =
      network.get_successors("v1_v2_0", "v1_v2_1");
  EXPECT_EQ(successors_v1_v2_0_v1_v2_1.size(), 1);
  EXPECT_TRUE(std::find(successors_v1_v2_0_v1_v2_1.begin(),
                        successors_v1_v2_0_v1_v2_1.end(),
                        network.get_edge_index("v1_v2_1", "v1_v2_2")) !=
              successors_v1_v2_0_v1_v2_1.end());
  // v1_v2_1->v1_v2_2 has one successor, namely v1_v2_2->v2
  const auto& successors_v1_v2_1_v1_v2_2 =
      network.get_successors("v1_v2_1", "v1_v2_2");
  EXPECT_EQ(successors_v1_v2_1_v1_v2_2.size(), 1);
  EXPECT_TRUE(std::find(successors_v1_v2_1_v1_v2_2.begin(),
                        successors_v1_v2_1_v1_v2_2.end(),
                        network.get_edge_index("v1_v2_2", "v2")) !=
              successors_v1_v2_1_v1_v2_2.end());
  // v1_v2_2->v2 has two successors, namely v2->v30 and v2->v31
  const auto& successors_v1_v2_2_v2 = network.get_successors("v1_v2_2", "v2");
  EXPECT_EQ(successors_v1_v2_2_v2.size(), 2);
  EXPECT_TRUE(std::find(successors_v1_v2_2_v2.begin(),
                        successors_v1_v2_2_v2.end(),
                        network.get_edge_index("v2", "v30")) !=
              successors_v1_v2_2_v2.end());
  EXPECT_TRUE(std::find(successors_v1_v2_2_v2.begin(),
                        successors_v1_v2_2_v2.end(),
                        network.get_edge_index("v2", "v31")) !=
              successors_v1_v2_2_v2.end());
  // v2->v30 has no successors
  EXPECT_TRUE(network.get_successors("v2", "v30").empty());
  // v2->v31 has no successors
  EXPECT_TRUE(network.get_successors("v2", "v31").empty());
}

TEST(Functionality, SortPairs) {
  cda_rail::Network network;
  // Add vertices
  network.add_vertex("v0", cda_rail::VertexType::TTD);
  network.add_vertex("v1", cda_rail::VertexType::NoBorderVSS);
  network.add_vertex("v2", cda_rail::VertexType::NoBorderVSS);
  network.add_vertex("v3", cda_rail::VertexType::NoBorderVSS);
  network.add_vertex("v4", cda_rail::VertexType::TTD);
  network.add_vertex("v5", cda_rail::VertexType::TTD);

  // Add edges
  int v0_v1 = network.add_edge("v0", "v1", 100, 100, false);
  int v2_v1 = network.add_edge("v2", "v1", 100, 100, false);
  int v1_v0 = network.add_edge("v1", "v0", 100, 100, false);
  int v1_v2 = network.add_edge("v1", "v2", 100, 100, false);
  int v2_v3 = network.add_edge("v2", "v3", 100, 100, false);
  int v3_v4 = network.add_edge("v3", "v4", 100, 100, false);
  int v4_v3 = network.add_edge("v4", "v3", 100, 100, false);
  network.add_edge("v4", "v5", 100, 100, false);
  network.add_edge("v5", "v4", 100, 100, false);

  // Edge pairs
  std::vector<int> to_combine = {v3_v4, v4_v3, v2_v1, v1_v2,
                                 v1_v0, v0_v1, v2_v3};
  const auto& combined_edges  = network.combine_reverse_edges(to_combine, true);

  // Check correctness
  std::vector<std::pair<int, int>> expected_combined_edges = {
      {v0_v1, v1_v0}, {v1_v2, v2_v1}, {v2_v3, -1}, {v3_v4, v4_v3}};
  EXPECT_EQ(combined_edges.size(), expected_combined_edges.size());
  int expected_index = 0;
  int expected_incr  = 1;
  if (combined_edges[0] != expected_combined_edges[0]) {
    expected_index = combined_edges.size() - 1;
    expected_incr  = -1;
  }
  for (int i = 0; i < combined_edges.size(); ++i) {
    EXPECT_EQ(combined_edges[i], expected_combined_edges[expected_index]);
    expected_index += expected_incr;
  }
}

TEST(Functionality, NetworkEdgeSeparationReverse) {
  cda_rail::Network network;
  // Add vertices
  network.add_vertex("v00", cda_rail::VertexType::TTD);
  network.add_vertex("v01", cda_rail::VertexType::TTD);
  network.add_vertex("v1", cda_rail::VertexType::TTD);
  network.add_vertex("v2", cda_rail::VertexType::TTD);
  network.add_vertex("v30", cda_rail::VertexType::TTD);
  network.add_vertex("v31", cda_rail::VertexType::TTD);

  // Add edges
  int v00_v1 = network.add_edge("v00", "v1", 100, 100, false);
  int v01_v1 = network.add_edge("v01", "v1", 100, 100, false);
  int v1_v2  = network.add_edge("v1", "v2", 44, 100, true, 10);
  int v2_v30 = network.add_edge("v2", "v30", 100, 100, false);
  int v2_v31 = network.add_edge("v2", "v31", 100, 100, false);
  // Add reverse edges
  int v1_v00 = network.add_edge("v1", "v00", 100, 100, false);
  int v1_v01 = network.add_edge("v1", "v01", 100, 100, false);
  int v2_v1  = network.add_edge("v2", "v1", 44, 100, true, 10);
  int v30_v2 = network.add_edge("v30", "v2", 100, 100, false);
  int v31_v2 = network.add_edge("v31", "v2", 100, 100, false);

  // Add successors
  network.add_successor(v00_v1, v1_v2);
  network.add_successor(v01_v1, v1_v2);
  network.add_successor(v1_v2, v2_v30);
  network.add_successor(v1_v2, v2_v31);
  // Add reverse successors
  network.add_successor(v31_v2, v2_v1);
  network.add_successor(v30_v2, v2_v1);
  network.add_successor(v2_v1, v1_v00);
  network.add_successor(v2_v1, v1_v01);

  // Separate edge v1_v2 uniformly
  auto new_edges =
      network.separate_edge("v1", "v2", cda_rail::SeparationType::UNIFORM);

  // There are 4 new edges forward and 4 new edges reverse
  EXPECT_EQ(new_edges.first.size(), 4);
  EXPECT_EQ(new_edges.second.size(), 4);

  // Check if the vertices are correct
  // All old vertices still exist. Additionally vertices v1_v2_0 to v1_v2_2 have
  // been added with type NoBorder Hence, in total there are 9 vertices.
  EXPECT_EQ(network.number_of_vertices(), 9);
  EXPECT_TRUE(network.has_vertex("v1_v2_0"));
  EXPECT_TRUE(network.has_vertex("v1_v2_1"));
  EXPECT_TRUE(network.has_vertex("v1_v2_2"));
  EXPECT_EQ(network.get_vertex("v1_v2_0").type,
            cda_rail::VertexType::NoBorderVSS);
  EXPECT_EQ(network.get_vertex("v1_v2_1").type,
            cda_rail::VertexType::NoBorderVSS);
  EXPECT_EQ(network.get_vertex("v1_v2_2").type,
            cda_rail::VertexType::NoBorderVSS);
  EXPECT_TRUE(network.has_vertex("v00"));
  EXPECT_TRUE(network.has_vertex("v01"));
  EXPECT_TRUE(network.has_vertex("v1"));
  EXPECT_TRUE(network.has_vertex("v2"));
  EXPECT_TRUE(network.has_vertex("v30"));
  EXPECT_TRUE(network.has_vertex("v31"));
  EXPECT_EQ(network.get_vertex("v00").type, cda_rail::VertexType::TTD);
  EXPECT_EQ(network.get_vertex("v01").type, cda_rail::VertexType::TTD);
  EXPECT_EQ(network.get_vertex("v1").type, cda_rail::VertexType::TTD);
  EXPECT_EQ(network.get_vertex("v2").type, cda_rail::VertexType::TTD);
  EXPECT_EQ(network.get_vertex("v30").type, cda_rail::VertexType::TTD);
  EXPECT_EQ(network.get_vertex("v31").type, cda_rail::VertexType::TTD);

  // Check if the edges are correct
  EXPECT_EQ(network.number_of_edges(), 16);
  // v00/v01 ->(len: 100) v1 ->(len: 11) v1_v2_0 -> (len: 11) v1_v2_1 -> (len:
  // 11) v1_v2_2 -> (len:11) v2 -> (len: 100) v30/v31
  EXPECT_TRUE(network.has_edge("v00", "v1"));
  EXPECT_TRUE(network.has_edge("v01", "v1"));
  EXPECT_TRUE(network.has_edge("v1", "v1_v2_0"));
  EXPECT_TRUE(network.has_edge("v1_v2_0", "v1_v2_1"));
  EXPECT_TRUE(network.has_edge("v1_v2_1", "v1_v2_2"));
  EXPECT_TRUE(network.has_edge("v1_v2_2", "v2"));
  EXPECT_TRUE(network.has_edge("v2", "v30"));
  EXPECT_TRUE(network.has_edge("v2", "v31"));
  EXPECT_EQ(network.get_edge("v00", "v1").length, 100);
  EXPECT_EQ(network.get_edge("v01", "v1").length, 100);
  EXPECT_EQ(network.get_edge("v1", "v1_v2_0").length, 11);
  EXPECT_EQ(network.get_edge("v1_v2_0", "v1_v2_1").length, 11);
  EXPECT_EQ(network.get_edge("v1_v2_1", "v1_v2_2").length, 11);
  EXPECT_EQ(network.get_edge("v1_v2_2", "v2").length, 11);
  EXPECT_EQ(network.get_edge("v2", "v30").length, 100);
  EXPECT_EQ(network.get_edge("v2", "v31").length, 100);
  // The other properties are unchanged, except that all edges are unbreakable,
  // i.e., the breakable property is false
  EXPECT_FALSE(network.get_edge("v00", "v1").breakable);
  EXPECT_FALSE(network.get_edge("v01", "v1").breakable);
  EXPECT_FALSE(network.get_edge("v1", "v1_v2_0").breakable);
  EXPECT_FALSE(network.get_edge("v1_v2_0", "v1_v2_1").breakable);
  EXPECT_FALSE(network.get_edge("v1_v2_1", "v1_v2_2").breakable);
  EXPECT_FALSE(network.get_edge("v1_v2_2", "v2").breakable);
  EXPECT_FALSE(network.get_edge("v2", "v30").breakable);
  EXPECT_FALSE(network.get_edge("v2", "v31").breakable);
  EXPECT_EQ(network.get_edge("v00", "v1").max_speed, 100);
  EXPECT_EQ(network.get_edge("v01", "v1").max_speed, 100);
  EXPECT_EQ(network.get_edge("v1", "v1_v2_0").max_speed, 100);
  EXPECT_EQ(network.get_edge("v1_v2_0", "v1_v2_1").max_speed, 100);
  EXPECT_EQ(network.get_edge("v1_v2_1", "v1_v2_2").max_speed, 100);
  EXPECT_EQ(network.get_edge("v1_v2_2", "v2").max_speed, 100);
  EXPECT_EQ(network.get_edge("v2", "v30").max_speed, 100);
  EXPECT_EQ(network.get_edge("v2", "v31").max_speed, 100);
  // The edge v1 -> v2 does not exist anymore
  EXPECT_FALSE(network.has_edge("v1", "v2"));

  // The new edges are v1 -> v1_v2_0 -> v1_v2_1 -> v1_v2_2 -> v2 in this order
  EXPECT_EQ(network.get_edge_index("v1", "v1_v2_0"), new_edges.first[0]);
  EXPECT_EQ(network.get_edge_index("v1_v2_0", "v1_v2_1"), new_edges.first[1]);
  EXPECT_EQ(network.get_edge_index("v1_v2_1", "v1_v2_2"), new_edges.first[2]);
  EXPECT_EQ(network.get_edge_index("v1_v2_2", "v2"), new_edges.first[3]);

  // The last index of the new edges is identical to the old index of v1->v2
  EXPECT_EQ(new_edges.first.back(), v1_v2);

  // Check if the reverse edges are correct
  // v30/v31 ->(len: 100) v2 ->(len: 11) v1_v2_2 -> (len: 11) v1_v2_1 -> (len:
  // 11) v1_v2_0 -> (len:11) v1 -> (len: 100) v00/v01
  EXPECT_TRUE(network.has_edge("v30", "v2"));
  EXPECT_TRUE(network.has_edge("v31", "v2"));
  EXPECT_TRUE(network.has_edge("v2", "v1_v2_2"));
  EXPECT_TRUE(network.has_edge("v1_v2_2", "v1_v2_1"));
  EXPECT_TRUE(network.has_edge("v1_v2_1", "v1_v2_0"));
  EXPECT_TRUE(network.has_edge("v1_v2_0", "v1"));
  EXPECT_TRUE(network.has_edge("v1", "v00"));
  EXPECT_TRUE(network.has_edge("v1", "v01"));
  EXPECT_EQ(network.get_edge("v30", "v2").length, 100);
  EXPECT_EQ(network.get_edge("v31", "v2").length, 100);
  EXPECT_EQ(network.get_edge("v2", "v1_v2_2").length, 11);
  EXPECT_EQ(network.get_edge("v1_v2_2", "v1_v2_1").length, 11);
  EXPECT_EQ(network.get_edge("v1_v2_1", "v1_v2_0").length, 11);
  EXPECT_EQ(network.get_edge("v1_v2_0", "v1").length, 11);
  EXPECT_EQ(network.get_edge("v1", "v00").length, 100);
  EXPECT_EQ(network.get_edge("v1", "v01").length, 100);
  // The other properties are unchanged, except that all edges are unbreakable,
  // i.e., the breakable property is false
  EXPECT_FALSE(network.get_edge("v30", "v2").breakable);
  EXPECT_FALSE(network.get_edge("v31", "v2").breakable);
  EXPECT_FALSE(network.get_edge("v2", "v1_v2_2").breakable);
  EXPECT_FALSE(network.get_edge("v1_v2_2", "v1_v2_1").breakable);
  EXPECT_FALSE(network.get_edge("v1_v2_1", "v1_v2_0").breakable);
  EXPECT_FALSE(network.get_edge("v1_v2_0", "v1").breakable);
  EXPECT_FALSE(network.get_edge("v1", "v00").breakable);
  EXPECT_FALSE(network.get_edge("v1", "v01").breakable);
  EXPECT_EQ(network.get_edge("v30", "v2").max_speed, 100);
  EXPECT_EQ(network.get_edge("v31", "v2").max_speed, 100);
  EXPECT_EQ(network.get_edge("v2", "v1_v2_2").max_speed, 100);
  EXPECT_EQ(network.get_edge("v1_v2_2", "v1_v2_1").max_speed, 100);
  EXPECT_EQ(network.get_edge("v1_v2_1", "v1_v2_0").max_speed, 100);
  EXPECT_EQ(network.get_edge("v1_v2_0", "v1").max_speed, 100);
  EXPECT_EQ(network.get_edge("v1", "v00").max_speed, 100);
  EXPECT_EQ(network.get_edge("v1", "v01").max_speed, 100);
  // The edge v2 -> v1 does not exist anymore
  EXPECT_FALSE(network.has_edge("v2", "v1"));

  // The new edges are v2 -> v1_v2_2 -> v1_v2_1 -> v1_v2_0 -> v1 in this order
  EXPECT_EQ(network.get_edge_index("v2", "v1_v2_2"), new_edges.second[0]);
  EXPECT_EQ(network.get_edge_index("v1_v2_2", "v1_v2_1"), new_edges.second[1]);
  EXPECT_EQ(network.get_edge_index("v1_v2_1", "v1_v2_0"), new_edges.second[2]);
  EXPECT_EQ(network.get_edge_index("v1_v2_0", "v1"), new_edges.second[3]);

  // The last index of the new edges is identical to the old index of v2->v1
  EXPECT_EQ(new_edges.second.back(), v2_v1);

  // v00 has one incoming edge, namely v1 -> v00
  const auto& v00_incoming = network.in_edges("v00");
  EXPECT_EQ(v00_incoming.size(), 1);
  EXPECT_TRUE(std::find(v00_incoming.begin(), v00_incoming.end(),
                        network.get_edge_index("v1", "v00")) !=
              v00_incoming.end());
  // v01 has one incoming edge, namely v1 -> v01
  const auto& v01_incoming = network.in_edges("v01");
  EXPECT_EQ(v01_incoming.size(), 1);
  EXPECT_TRUE(std::find(v01_incoming.begin(), v01_incoming.end(),
                        network.get_edge_index("v1", "v01")) !=
              v01_incoming.end());
  // v1 has three incoming edges, namely from v00, v01 and v1_v2_0
  const auto& v1_incoming = network.in_edges("v1");
  EXPECT_EQ(v1_incoming.size(), 3);
  EXPECT_TRUE(std::find(v1_incoming.begin(), v1_incoming.end(),
                        network.get_edge_index("v00", "v1")) !=
              v1_incoming.end());
  EXPECT_TRUE(std::find(v1_incoming.begin(), v1_incoming.end(),
                        network.get_edge_index("v01", "v1")) !=
              v1_incoming.end());
  EXPECT_TRUE(std::find(v1_incoming.begin(), v1_incoming.end(),
                        network.get_edge_index("v1_v2_0", "v1")) !=
              v1_incoming.end());
  // v1_v2_0 has two incoming edges, namely from v1 and v1_v2_1
  const auto& v1_v2_0_incoming = network.in_edges("v1_v2_0");
  EXPECT_EQ(v1_v2_0_incoming.size(), 2);
  EXPECT_TRUE(std::find(v1_v2_0_incoming.begin(), v1_v2_0_incoming.end(),
                        network.get_edge_index("v1", "v1_v2_0")) !=
              v1_v2_0_incoming.end());
  EXPECT_TRUE(std::find(v1_v2_0_incoming.begin(), v1_v2_0_incoming.end(),
                        network.get_edge_index("v1_v2_1", "v1_v2_0")) !=
              v1_v2_0_incoming.end());
  // v1_v2_1 has two incoming edges, namely from v1_v2_0 and v1_v2_2
  const auto& v1_v2_1_incoming = network.in_edges("v1_v2_1");
  EXPECT_EQ(v1_v2_1_incoming.size(), 2);
  EXPECT_TRUE(std::find(v1_v2_1_incoming.begin(), v1_v2_1_incoming.end(),
                        network.get_edge_index("v1_v2_0", "v1_v2_1")) !=
              v1_v2_1_incoming.end());
  EXPECT_TRUE(std::find(v1_v2_1_incoming.begin(), v1_v2_1_incoming.end(),
                        network.get_edge_index("v1_v2_2", "v1_v2_1")) !=
              v1_v2_1_incoming.end());
  // v1_v2_2 has two incoming edges, namely from v1_v2_1 and v2
  const auto& v1_v2_2_incoming = network.in_edges("v1_v2_2");
  EXPECT_EQ(v1_v2_2_incoming.size(), 2);
  EXPECT_TRUE(std::find(v1_v2_2_incoming.begin(), v1_v2_2_incoming.end(),
                        network.get_edge_index("v1_v2_1", "v1_v2_2")) !=
              v1_v2_2_incoming.end());
  EXPECT_TRUE(std::find(v1_v2_2_incoming.begin(), v1_v2_2_incoming.end(),
                        network.get_edge_index("v2", "v1_v2_2")) !=
              v1_v2_2_incoming.end());
  // v2 has three incoming edges, namely from v1_v2_2, v30 and v31
  const auto& v2_incoming = network.in_edges("v2");
  EXPECT_EQ(v2_incoming.size(), 3);
  EXPECT_TRUE(std::find(v2_incoming.begin(), v2_incoming.end(),
                        network.get_edge_index("v1_v2_2", "v2")) !=
              v2_incoming.end());
  EXPECT_TRUE(std::find(v2_incoming.begin(), v2_incoming.end(),
                        network.get_edge_index("v30", "v2")) !=
              v2_incoming.end());
  EXPECT_TRUE(std::find(v2_incoming.begin(), v2_incoming.end(),
                        network.get_edge_index("v31", "v2")) !=
              v2_incoming.end());
  // v30 has one incoming edge, namely from v2
  const auto& v30_incoming = network.in_edges("v30");
  EXPECT_EQ(v30_incoming.size(), 1);
  EXPECT_TRUE(std::find(v30_incoming.begin(), v30_incoming.end(),
                        network.get_edge_index("v2", "v30")) !=
              v30_incoming.end());
  // v31 has one incoming edge, namely from v2
  const auto& v31_incoming = network.in_edges("v31");
  EXPECT_EQ(v31_incoming.size(), 1);
  EXPECT_TRUE(std::find(v31_incoming.begin(), v31_incoming.end(),
                        network.get_edge_index("v2", "v31")) !=
              v31_incoming.end());

  // v00 has one outgoing edge, namely to v1
  const auto& v00_outgoing = network.out_edges("v00");
  EXPECT_EQ(v00_outgoing.size(), 1);
  EXPECT_TRUE(std::find(v00_outgoing.begin(), v00_outgoing.end(),
                        network.get_edge_index("v00", "v1")) !=
              v00_outgoing.end());
  // v01 has one outgoing edge, namely to v1
  const auto& v01_outgoing = network.out_edges("v01");
  EXPECT_EQ(v01_outgoing.size(), 1);
  EXPECT_TRUE(std::find(v01_outgoing.begin(), v01_outgoing.end(),
                        network.get_edge_index("v01", "v1")) !=
              v01_outgoing.end());
  // v1 has three outgoing edges, namely to v00, v01 and v1_v2_0
  const auto& v1_outgoing = network.out_edges("v1");
  EXPECT_EQ(v1_outgoing.size(), 3);
  EXPECT_TRUE(std::find(v1_outgoing.begin(), v1_outgoing.end(),
                        network.get_edge_index("v1", "v00")) !=
              v1_outgoing.end());
  EXPECT_TRUE(std::find(v1_outgoing.begin(), v1_outgoing.end(),
                        network.get_edge_index("v1", "v01")) !=
              v1_outgoing.end());
  EXPECT_TRUE(std::find(v1_outgoing.begin(), v1_outgoing.end(),
                        network.get_edge_index("v1", "v1_v2_0")) !=
              v1_outgoing.end());
  // v1_v2_0 has two outgoing edges, namely to v1 and v1_v2_1
  const auto& v1_v2_0_outgoing = network.out_edges("v1_v2_0");
  EXPECT_EQ(v1_v2_0_outgoing.size(), 2);
  EXPECT_TRUE(std::find(v1_v2_0_outgoing.begin(), v1_v2_0_outgoing.end(),
                        network.get_edge_index("v1_v2_0", "v1")) !=
              v1_v2_0_outgoing.end());
  EXPECT_TRUE(std::find(v1_v2_0_outgoing.begin(), v1_v2_0_outgoing.end(),
                        network.get_edge_index("v1_v2_0", "v1_v2_1")) !=
              v1_v2_0_outgoing.end());
  // v1_v2_1 has two outgoing edges, namely to v1_v2_0 and v1_v2_2
  const auto& v1_v2_1_outgoing = network.out_edges("v1_v2_1");
  EXPECT_EQ(v1_v2_1_outgoing.size(), 2);
  EXPECT_TRUE(std::find(v1_v2_1_outgoing.begin(), v1_v2_1_outgoing.end(),
                        network.get_edge_index("v1_v2_1", "v1_v2_0")) !=
              v1_v2_1_outgoing.end());
  EXPECT_TRUE(std::find(v1_v2_1_outgoing.begin(), v1_v2_1_outgoing.end(),
                        network.get_edge_index("v1_v2_1", "v1_v2_2")) !=
              v1_v2_1_outgoing.end());
  // v1_v2_2 has two outgoing edges, namely to v1_v2_1 and v2
  const auto& v1_v2_2_outgoing = network.out_edges("v1_v2_2");
  EXPECT_EQ(v1_v2_2_outgoing.size(), 2);
  EXPECT_TRUE(std::find(v1_v2_2_outgoing.begin(), v1_v2_2_outgoing.end(),
                        network.get_edge_index("v1_v2_2", "v1_v2_1")) !=
              v1_v2_2_outgoing.end());
  EXPECT_TRUE(std::find(v1_v2_2_outgoing.begin(), v1_v2_2_outgoing.end(),
                        network.get_edge_index("v1_v2_2", "v2")) !=
              v1_v2_2_outgoing.end());
  // v2 has three outgoing edges, namely to v1_v2_2, v30 and v31
  const auto& v2_outgoing = network.out_edges("v2");
  EXPECT_EQ(v2_outgoing.size(), 3);
  EXPECT_TRUE(std::find(v2_outgoing.begin(), v2_outgoing.end(),
                        network.get_edge_index("v2", "v1_v2_2")) !=
              v2_outgoing.end());
  EXPECT_TRUE(std::find(v2_outgoing.begin(), v2_outgoing.end(),
                        network.get_edge_index("v2", "v30")) !=
              v2_outgoing.end());
  EXPECT_TRUE(std::find(v2_outgoing.begin(), v2_outgoing.end(),
                        network.get_edge_index("v2", "v31")) !=
              v2_outgoing.end());
  // v30 has one outgoing edge, namely to v2
  const auto& v30_outgoing = network.out_edges("v30");
  EXPECT_EQ(v30_outgoing.size(), 1);
  EXPECT_TRUE(std::find(v30_outgoing.begin(), v30_outgoing.end(),
                        network.get_edge_index("v30", "v2")) !=
              v30_outgoing.end());
  // v31 has one outgoing edge, namely to v2
  const auto& v31_outgoing = network.out_edges("v31");
  EXPECT_EQ(v31_outgoing.size(), 1);
  EXPECT_TRUE(std::find(v31_outgoing.begin(), v31_outgoing.end(),
                        network.get_edge_index("v31", "v2")) !=
              v31_outgoing.end());

  // Check the successors, they should disallow turning around
  // Successors of v00->v1 is the edge to v1_v2_0
  const auto& v00_v1_successors = network.get_successors("v00", "v1");
  EXPECT_EQ(v00_v1_successors.size(), 1);
  EXPECT_TRUE(std::find(v00_v1_successors.begin(), v00_v1_successors.end(),
                        network.get_edge_index("v1", "v1_v2_0")) !=
              v00_v1_successors.end());
  // Successors of v01->v1 is the edge to v1_v2_0
  const auto& v01_v1_successors = network.get_successors("v01", "v1");
  EXPECT_EQ(v01_v1_successors.size(), 1);
  EXPECT_TRUE(std::find(v01_v1_successors.begin(), v01_v1_successors.end(),
                        network.get_edge_index("v1", "v1_v2_0")) !=
              v01_v1_successors.end());
  // Successors of v1->v1_v2_0 is the edge to v1_v2_1
  const auto& v1_v1_v2_0_successors = network.get_successors("v1", "v1_v2_0");
  EXPECT_EQ(v1_v1_v2_0_successors.size(), 1);
  EXPECT_TRUE(std::find(v1_v1_v2_0_successors.begin(),
                        v1_v1_v2_0_successors.end(),
                        network.get_edge_index("v1_v2_0", "v1_v2_1")) !=
              v1_v1_v2_0_successors.end());
  // Successors of v1_v2_0->v1_v2_1 is the edge to v1_v2_2
  const auto& v1_v2_0_v1_v2_1_successors =
      network.get_successors("v1_v2_0", "v1_v2_1");
  EXPECT_EQ(v1_v2_0_v1_v2_1_successors.size(), 1);
  EXPECT_TRUE(std::find(v1_v2_0_v1_v2_1_successors.begin(),
                        v1_v2_0_v1_v2_1_successors.end(),
                        network.get_edge_index("v1_v2_1", "v1_v2_2")) !=
              v1_v2_0_v1_v2_1_successors.end());
  // Successors of v1_v2_1->v1_v2_2 is the edge to v2
  const auto& v1_v2_1_v1_v2_2_successors =
      network.get_successors("v1_v2_1", "v1_v2_2");
  EXPECT_EQ(v1_v2_1_v1_v2_2_successors.size(), 1);
  EXPECT_TRUE(std::find(v1_v2_1_v1_v2_2_successors.begin(),
                        v1_v2_1_v1_v2_2_successors.end(),
                        network.get_edge_index("v1_v2_2", "v2")) !=
              v1_v2_1_v1_v2_2_successors.end());
  // Successors of v1_v2_2->v2 are the edges to v30 and v31
  const auto& v1_v2_2_v2_successors = network.get_successors("v1_v2_2", "v2");
  EXPECT_EQ(v1_v2_2_v2_successors.size(), 2);
  EXPECT_TRUE(std::find(v1_v2_2_v2_successors.begin(),
                        v1_v2_2_v2_successors.end(),
                        network.get_edge_index("v2", "v30")) !=
              v1_v2_2_v2_successors.end());
  EXPECT_TRUE(std::find(v1_v2_2_v2_successors.begin(),
                        v1_v2_2_v2_successors.end(),
                        network.get_edge_index("v2", "v31")) !=
              v1_v2_2_v2_successors.end());
  // Successors of v2->v30 and v2->v31 are empty
  EXPECT_TRUE(network.get_successors("v2", "v30").empty());
  // Successors of v30->v2 is the edge to v1_v2_2
  const auto& v30_v2_successors = network.get_successors("v30", "v2");
  EXPECT_EQ(v30_v2_successors.size(), 1);
  EXPECT_TRUE(std::find(v30_v2_successors.begin(), v30_v2_successors.end(),
                        network.get_edge_index("v2", "v1_v2_2")) !=
              v30_v2_successors.end());
  // Successors of v31->v2 is the edge to v1_v2_2
  const auto& v31_v2_successors = network.get_successors("v31", "v2");
  EXPECT_EQ(v31_v2_successors.size(), 1);
  EXPECT_TRUE(std::find(v31_v2_successors.begin(), v31_v2_successors.end(),
                        network.get_edge_index("v2", "v1_v2_2")) !=
              v31_v2_successors.end());
  // Successors of v2->v1_v2_2 is the edge to v1_v2_1
  const auto& v2_v1_v2_2_successors = network.get_successors("v2", "v1_v2_2");
  EXPECT_EQ(v2_v1_v2_2_successors.size(), 1);
  EXPECT_TRUE(std::find(v2_v1_v2_2_successors.begin(),
                        v2_v1_v2_2_successors.end(),
                        network.get_edge_index("v1_v2_2", "v1_v2_1")) !=
              v2_v1_v2_2_successors.end());
  // Successors of v1_v2_2->v1_v2_1 is the edge to v1_v2_0
  const auto& v1_v2_2_v1_v2_1_successors =
      network.get_successors("v1_v2_2", "v1_v2_1");
  EXPECT_EQ(v1_v2_2_v1_v2_1_successors.size(), 1);
  EXPECT_TRUE(std::find(v1_v2_2_v1_v2_1_successors.begin(),
                        v1_v2_2_v1_v2_1_successors.end(),
                        network.get_edge_index("v1_v2_1", "v1_v2_0")) !=
              v1_v2_2_v1_v2_1_successors.end());
  // Successors of v1_v2_1->v1_v2_0 is the edge to v1
  const auto& v1_v2_1_v1_v2_0_successors =
      network.get_successors("v1_v2_1", "v1_v2_0");
  EXPECT_EQ(v1_v2_1_v1_v2_0_successors.size(), 1);
  EXPECT_TRUE(std::find(v1_v2_1_v1_v2_0_successors.begin(),
                        v1_v2_1_v1_v2_0_successors.end(),
                        network.get_edge_index("v1_v2_0", "v1")) !=
              v1_v2_1_v1_v2_0_successors.end());
  // Successors of v1_v2_0->v1 are the edges to v00 and v01
  const auto& v1_v2_0_v1_successors = network.get_successors("v1_v2_0", "v1");
  EXPECT_EQ(v1_v2_0_v1_successors.size(), 2);
  EXPECT_TRUE(std::find(v1_v2_0_v1_successors.begin(),
                        v1_v2_0_v1_successors.end(),
                        network.get_edge_index("v1", "v00")) !=
              v1_v2_0_v1_successors.end());
  EXPECT_TRUE(std::find(v1_v2_0_v1_successors.begin(),
                        v1_v2_0_v1_successors.end(),
                        network.get_edge_index("v1", "v01")) !=
              v1_v2_0_v1_successors.end());
  // Successors of v1->v00 and v1->v01 are empty
  EXPECT_TRUE(network.get_successors("v1", "v00").empty());
  EXPECT_TRUE(network.get_successors("v1", "v01").empty());
}

TEST(Functionality, NetworkVerticesByType) {
  cda_rail::Network network;
  // Add vertices of each type NoBorder (1x), TTD (2x), VSS (3x), NoBorderVSS
  // (4x)
  auto v1  = network.add_vertex("v1", cda_rail::VertexType::NoBorder);
  auto v2  = network.add_vertex("v2", cda_rail::VertexType::TTD);
  auto v3  = network.add_vertex("v3", cda_rail::VertexType::TTD);
  auto v4  = network.add_vertex("v4", cda_rail::VertexType::VSS);
  auto v5  = network.add_vertex("v5", cda_rail::VertexType::VSS);
  auto v6  = network.add_vertex("v6", cda_rail::VertexType::VSS);
  auto v7  = network.add_vertex("v7", cda_rail::VertexType::NoBorderVSS);
  auto v8  = network.add_vertex("v8", cda_rail::VertexType::NoBorderVSS);
  auto v9  = network.add_vertex("v9", cda_rail::VertexType::NoBorderVSS);
  auto v10 = network.add_vertex("v10", cda_rail::VertexType::NoBorderVSS);

  // Check if the vertices are in the correct sets
  auto no_border = network.get_vertices_by_type(cda_rail::VertexType::NoBorder);
  EXPECT_EQ(no_border.size(), 1);
  EXPECT_TRUE(std::find(no_border.begin(), no_border.end(), v1) !=
              no_border.end());

  auto ttd = network.get_vertices_by_type(cda_rail::VertexType::TTD);
  EXPECT_EQ(ttd.size(), 2);
  EXPECT_TRUE(std::find(ttd.begin(), ttd.end(), v2) != ttd.end());
  EXPECT_TRUE(std::find(ttd.begin(), ttd.end(), v3) != ttd.end());

  auto vss = network.get_vertices_by_type(cda_rail::VertexType::VSS);
  EXPECT_EQ(vss.size(), 3);
  EXPECT_TRUE(std::find(vss.begin(), vss.end(), v4) != vss.end());
  EXPECT_TRUE(std::find(vss.begin(), vss.end(), v5) != vss.end());
  EXPECT_TRUE(std::find(vss.begin(), vss.end(), v6) != vss.end());

  auto no_border_vss =
      network.get_vertices_by_type(cda_rail::VertexType::NoBorderVSS);
  EXPECT_EQ(no_border_vss.size(), 4);
  EXPECT_TRUE(std::find(no_border_vss.begin(), no_border_vss.end(), v7) !=
              no_border_vss.end());
  EXPECT_TRUE(std::find(no_border_vss.begin(), no_border_vss.end(), v8) !=
              no_border_vss.end());
  EXPECT_TRUE(std::find(no_border_vss.begin(), no_border_vss.end(), v9) !=
              no_border_vss.end());
  EXPECT_TRUE(std::find(no_border_vss.begin(), no_border_vss.end(), v10) !=
              no_border_vss.end());
}

TEST(Functionality, ReverseIndices) {
  cda_rail::Network network;
  network.add_vertex("v1", cda_rail::VertexType::TTD);
  network.add_vertex("v2", cda_rail::VertexType::TTD);
  network.add_vertex("v3", cda_rail::VertexType::TTD);
  network.add_vertex("v4", cda_rail::VertexType::TTD);

  auto e12 = network.add_edge("v1", "v2", 100, 10, false);
  auto e23 = network.add_edge("v2", "v3", 100, 10, false);
  auto e34 = network.add_edge("v3", "v4", 100, 10, false);
  auto e43 = network.add_edge("v4", "v3", 100, 10, false);
  auto e21 = network.add_edge("v2", "v1", 100, 10, false);

  // Check if the reverse indices are correct
  EXPECT_EQ(network.get_reverse_edge_index(e12), e21);
  EXPECT_EQ(network.get_reverse_edge_index(e23), -1);
  EXPECT_EQ(network.get_reverse_edge_index(e34), e43);
  EXPECT_EQ(network.get_reverse_edge_index(e43), e34);
  EXPECT_EQ(network.get_reverse_edge_index(e21), e12);

  std::vector edges          = {e12, e23, e34, e43, e21};
  auto        edges_combined = network.combine_reverse_edges(edges);
  // Expect three edge sets
  EXPECT_EQ(edges_combined.size(), 3);
  // Expect the following pairs to exist: (min(e12, e21), max(e12, e21)), (e23,
  // -1), and (min(e34, e43), max(e34, e43))
  EXPECT_TRUE(
      std::find(edges_combined.begin(), edges_combined.end(),
                std::make_pair(std::min(e12, e21), std::max(e12, e21))) !=
      edges_combined.end());
  EXPECT_TRUE(std::find(edges_combined.begin(), edges_combined.end(),
                        std::make_pair(e23, -1)) != edges_combined.end());
  EXPECT_TRUE(
      std::find(edges_combined.begin(), edges_combined.end(),
                std::make_pair(std::min(e34, e43), std::max(e34, e43))) !=
      edges_combined.end());
}

TEST(Functionality, InverseEdges) {
  cda_rail::Network network;

  network.add_vertex("v1", cda_rail::VertexType::TTD);
  network.add_vertex("v2", cda_rail::VertexType::TTD);
  network.add_vertex("v3", cda_rail::VertexType::TTD);
  network.add_vertex("v4", cda_rail::VertexType::TTD);

  const auto e12 = network.add_edge("v1", "v2", 100, 10, false);
  const auto e23 = network.add_edge("v2", "v3", 100, 10, false);
  const auto e34 = network.add_edge("v3", "v4", 100, 10, false);
  const auto e32 = network.add_edge("v3", "v2", 100, 10, false);

  // Check if the inverse edges are correct

  // inverse of e12 and e23 is e34 and e32
  const auto inv_1 = network.inverse_edges({e12, e23});
  EXPECT_EQ(inv_1.size(), 2);
  EXPECT_TRUE(std::find(inv_1.begin(), inv_1.end(), e34) != inv_1.end());
  EXPECT_TRUE(std::find(inv_1.begin(), inv_1.end(), e32) != inv_1.end());

  // inverse of e23 and e32 only considering e12, e23 and e34 is e12 and e34
  const auto inv_2 = network.inverse_edges({e23, e32}, {e12, e23, e34});
  EXPECT_EQ(inv_2.size(), 2);
  EXPECT_TRUE(std::find(inv_2.begin(), inv_2.end(), e12) != inv_2.end());
  EXPECT_TRUE(std::find(inv_2.begin(), inv_2.end(), e34) != inv_2.end());
}

TEST(Functionality, FloydWarshall) {
  // Initialize empty network
  cda_rail::Network network;

  // Add 6 vertices
  network.add_vertex("v1", cda_rail::VertexType::TTD);
  network.add_vertex("v2", cda_rail::VertexType::TTD);
  network.add_vertex("v3", cda_rail::VertexType::TTD);
  network.add_vertex("v4", cda_rail::VertexType::TTD);
  network.add_vertex("v5", cda_rail::VertexType::TTD);
  network.add_vertex("v6", cda_rail::VertexType::TTD);

  // Add the following edges
  // v1 v2 of length 100
  int v1_v2 = network.add_edge("v1", "v2", 100, 10, false);
  // v2 v3 in both directions of length 200
  int v2_v3 = network.add_edge("v2", "v3", 200, 10, false);
  int v3_v2 = network.add_edge("v3", "v2", 200, 10, false);
  // v3 v4 in both directions of length 300
  int v3_v4 = network.add_edge("v3", "v4", 300, 10, false);
  int v4_v3 = network.add_edge("v4", "v3", 300, 10, false);
  // v4 v5 in both directions of length 400
  int v4_v5 = network.add_edge("v4", "v5", 400, 10, false);
  int v5_v4 = network.add_edge("v5", "v4", 400, 10, false);
  // v4 v1 of length 500
  int v4_v1 = network.add_edge("v4", "v1", 500, 10, false);
  // v3 v5 of length 500
  int v3_v5 = network.add_edge("v3", "v5", 500, 10, false);
  // v5 v6 in both directions of length 1000
  int v5_v6 = network.add_edge("v5", "v6", 1000, 10, false);
  int v6_v5 = network.add_edge("v6", "v5", 1000, 10, false);

  // Add successor edges
  network.add_successor(v1_v2, v2_v3);
  network.add_successor(v2_v3, v3_v4);
  network.add_successor(v2_v3, v3_v5);
  network.add_successor(v3_v4, v4_v5);
  network.add_successor(v3_v4, v4_v1);
  network.add_successor(v4_v3, v3_v2);
  network.add_successor(v4_v5, v5_v6);
  network.add_successor(v5_v4, v4_v3);
  network.add_successor(v4_v1, v1_v2);
  network.add_successor(v3_v5, v5_v6);
  network.add_successor(v6_v5, v5_v4);

  const auto shortest_paths = network.all_edge_pairs_shortest_paths();

  // Check if the shortest paths are correct
  // Starting from v1_v2, we reach
  // v1_v2 in 0
  // v2_v3 in 200
  // v3_v4 in 500
  // v3_v5 in 700
  // v4_v5 in 900
  // v5_v6 in 1700
  // v4_v1 in 1000
  // all other edges are not reachable
  EXPECT_EQ(shortest_paths.at(v1_v2, v1_v2), 0);
  EXPECT_EQ(shortest_paths.at(v1_v2, v2_v3), 200);
  EXPECT_EQ(shortest_paths.at(v1_v2, v3_v4), 500);
  EXPECT_EQ(shortest_paths.at(v1_v2, v3_v5), 700);
  EXPECT_EQ(shortest_paths.at(v1_v2, v4_v5), 900);
  EXPECT_EQ(shortest_paths.at(v1_v2, v5_v6), 1700);
  EXPECT_EQ(shortest_paths.at(v1_v2, v4_v1), 1000);
  EXPECT_EQ(shortest_paths.at(v1_v2, v3_v2), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v1_v2, v4_v3), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v1_v2, v5_v4), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v1_v2, v6_v5), cda_rail::INF);

  // Starting from v2_v3, we reach
  // v2_v3 in 0
  // v3_v4 in 300
  // v3_v5 in 500
  // v4_v5 in 700
  // v5_v6 in 1500
  // v4_v1 in 800
  // v1_v2 in 900
  // all other edges are not reachable
  EXPECT_EQ(shortest_paths.at(v2_v3, v2_v3), 0);
  EXPECT_EQ(shortest_paths.at(v2_v3, v3_v4), 300);
  EXPECT_EQ(shortest_paths.at(v2_v3, v3_v5), 500);
  EXPECT_EQ(shortest_paths.at(v2_v3, v4_v5), 700);
  EXPECT_EQ(shortest_paths.at(v2_v3, v5_v6), 1500);
  EXPECT_EQ(shortest_paths.at(v2_v3, v4_v1), 800);
  EXPECT_EQ(shortest_paths.at(v2_v3, v1_v2), 900);
  EXPECT_EQ(shortest_paths.at(v2_v3, v3_v2), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v2_v3, v4_v3), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v2_v3, v5_v4), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v2_v3, v6_v5), cda_rail::INF);

  // Starting from v3_v4, we reach
  // v3_v4 in 0
  // v4_v5 in 400
  // v5_v6 in 1400
  // v4_v1 in 500
  // v1_v2 in 600
  // v2_v3 in 800
  // v3_v5 in 1300
  // all other edges are not reachable
  EXPECT_EQ(shortest_paths.at(v3_v4, v3_v4), 0);
  EXPECT_EQ(shortest_paths.at(v3_v4, v4_v5), 400);
  EXPECT_EQ(shortest_paths.at(v3_v4, v5_v6), 1400);
  EXPECT_EQ(shortest_paths.at(v3_v4, v4_v1), 500);
  EXPECT_EQ(shortest_paths.at(v3_v4, v1_v2), 600);
  EXPECT_EQ(shortest_paths.at(v3_v4, v2_v3), 800);
  EXPECT_EQ(shortest_paths.at(v3_v4, v3_v5), 1300);
  EXPECT_EQ(shortest_paths.at(v3_v4, v3_v2), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v3_v4, v4_v3), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v3_v4, v5_v4), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v3_v4, v6_v5), cda_rail::INF);

  // Starting from v3_v5, we reach
  // v3_v5 in 0
  // v5_v6 in 1000
  // all other edges are not reachable
  EXPECT_EQ(shortest_paths.at(v3_v5, v3_v5), 0);
  EXPECT_EQ(shortest_paths.at(v3_v5, v5_v6), 1000);
  EXPECT_EQ(shortest_paths.at(v3_v5, v3_v4), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v3_v5, v4_v5), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v3_v5, v4_v1), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v3_v5, v1_v2), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v3_v5, v2_v3), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v3_v5, v4_v3), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v3_v5, v5_v4), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v3_v5, v6_v5), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v3_v5, v3_v2), cda_rail::INF);

  // Starting from v4_v5, we reach
  // v4_v5 in 0
  // v5_v6 in 1000
  // all other edges are not reachable
  EXPECT_EQ(shortest_paths.at(v4_v5, v4_v5), 0);
  EXPECT_EQ(shortest_paths.at(v4_v5, v5_v6), 1000);
  EXPECT_EQ(shortest_paths.at(v4_v5, v3_v4), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v4_v5, v3_v5), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v4_v5, v4_v1), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v4_v5, v1_v2), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v4_v5, v2_v3), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v4_v5, v4_v3), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v4_v5, v5_v4), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v4_v5, v6_v5), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v4_v5, v3_v2), cda_rail::INF);

  // Starting from v5_v6, we reach
  // v5_v6 in 0
  // all other edges are not reachable
  EXPECT_EQ(shortest_paths.at(v5_v6, v5_v6), 0);
  EXPECT_EQ(shortest_paths.at(v5_v6, v3_v4), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v5_v6, v3_v5), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v5_v6, v4_v5), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v5_v6, v4_v1), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v5_v6, v1_v2), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v5_v6, v2_v3), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v5_v6, v4_v3), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v5_v6, v5_v4), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v5_v6, v6_v5), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v5_v6, v3_v2), cda_rail::INF);

  // Starting from v4_v1, we reach
  // v4_v1 in 0
  // v1_v2 in 100
  // v2_v3 in 300
  // v3_v4 in 600
  // v4_v5 in 1000
  // v3_v5 in 800
  // v5_v6 in 1800
  // all other edges are not reachable
  EXPECT_EQ(shortest_paths.at(v4_v1, v4_v1), 0);
  EXPECT_EQ(shortest_paths.at(v4_v1, v1_v2), 100);
  EXPECT_EQ(shortest_paths.at(v4_v1, v2_v3), 300);
  EXPECT_EQ(shortest_paths.at(v4_v1, v3_v4), 600);
  EXPECT_EQ(shortest_paths.at(v4_v1, v4_v5), 1000);
  EXPECT_EQ(shortest_paths.at(v4_v1, v3_v5), 800);
  EXPECT_EQ(shortest_paths.at(v4_v1, v5_v6), 1800);
  EXPECT_EQ(shortest_paths.at(v4_v1, v3_v2), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v4_v1, v4_v3), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v4_v1, v5_v4), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v4_v1, v6_v5), cda_rail::INF);

  // Starting from v6_v5, we reach
  // v6_v5 in 0
  // v5_v4 in 400
  // v4_v3 in 700
  // v3_v2 in 900
  // all other edges are not reachable
  EXPECT_EQ(shortest_paths.at(v6_v5, v6_v5), 0);
  EXPECT_EQ(shortest_paths.at(v6_v5, v5_v4), 400);
  EXPECT_EQ(shortest_paths.at(v6_v5, v4_v3), 700);
  EXPECT_EQ(shortest_paths.at(v6_v5, v3_v2), 900);
  EXPECT_EQ(shortest_paths.at(v6_v5, v3_v4), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v6_v5, v3_v5), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v6_v5, v4_v5), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v6_v5, v4_v1), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v6_v5, v1_v2), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v6_v5, v2_v3), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v6_v5, v5_v6), cda_rail::INF);

  // Starting from v5_v4, we reach
  // v5_v4 in 0
  // v4_v3 in 300
  // v3_v2 in 500
  // all other edges are not reachable
  EXPECT_EQ(shortest_paths.at(v5_v4, v5_v4), 0);
  EXPECT_EQ(shortest_paths.at(v5_v4, v4_v3), 300);
  EXPECT_EQ(shortest_paths.at(v5_v4, v3_v2), 500);
  EXPECT_EQ(shortest_paths.at(v5_v4, v3_v4), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v5_v4, v3_v5), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v5_v4, v4_v5), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v5_v4, v4_v1), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v5_v4, v1_v2), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v5_v4, v2_v3), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v5_v4, v5_v6), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v5_v4, v6_v5), cda_rail::INF);

  // Starting from v4_v3, we reach
  // v4_v3 in 0
  // v3_v2 in 200
  // all other edges are not reachable
  EXPECT_EQ(shortest_paths.at(v4_v3, v4_v3), 0);
  EXPECT_EQ(shortest_paths.at(v4_v3, v3_v2), 200);
  EXPECT_EQ(shortest_paths.at(v4_v3, v3_v4), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v4_v3, v3_v5), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v4_v3, v4_v5), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v4_v3, v4_v1), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v4_v3, v1_v2), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v4_v3, v2_v3), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v4_v3, v5_v4), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v4_v3, v5_v6), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v4_v3, v6_v5), cda_rail::INF);

  // Starting from v3_v2, we reach
  // v3_v2 in 0
  // all other edges are not reachable
  EXPECT_EQ(shortest_paths.at(v3_v2, v3_v2), 0);
  EXPECT_EQ(shortest_paths.at(v3_v2, v3_v4), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v3_v2, v3_v5), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v3_v2, v4_v3), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v3_v2, v4_v5), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v3_v2, v4_v1), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v3_v2, v1_v2), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v3_v2, v2_v3), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v3_v2, v5_v4), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v3_v2, v5_v6), cda_rail::INF);
  EXPECT_EQ(shortest_paths.at(v3_v2, v6_v5), cda_rail::INF);
}

TEST(Functionality, ReadTrains) {
  auto trains = cda_rail::TrainList::import_trains(
      "./example-networks/SimpleStation/timetable/");

  // Check if the all trains are imported
  EXPECT_EQ(trains.size(), 3);
  EXPECT_TRUE(trains.has_train("tr1"));
  EXPECT_TRUE(trains.has_train("tr2"));
  EXPECT_TRUE(trains.has_train("tr3"));

  // Check if the train tr1 is imported correctly
  auto tr1 = trains.get_train("tr1");
  EXPECT_EQ(tr1.name, "tr1");
  EXPECT_EQ(tr1.length, 100);
  EXPECT_EQ(tr1.max_speed, 83.33);
  EXPECT_EQ(tr1.acceleration, 2);
  EXPECT_EQ(tr1.deceleration, 1);

  // Check if the train tr2 is imported correctly
  auto tr2 = trains.get_train("tr2");
  EXPECT_EQ(tr2.name, "tr2");
  EXPECT_EQ(tr2.length, 100);
  EXPECT_EQ(tr2.max_speed, 27.78);
  EXPECT_EQ(tr2.acceleration, 2);
  EXPECT_EQ(tr2.deceleration, 1);

  // Check if the train tr3 is imported correctly
  auto tr3 = trains.get_train("tr3");
  EXPECT_EQ(tr3.name, "tr3");
  EXPECT_EQ(tr3.length, 250);
  EXPECT_EQ(tr3.max_speed, 20);
  EXPECT_EQ(tr3.acceleration, 2);
  EXPECT_EQ(tr3.deceleration, 1);
}

TEST(Functionality, WriteTrains) {
  // Create a train list
  auto trains    = cda_rail::TrainList();
  int  tr1_index = trains.add_train("tr1", 100, 83.33, 2, 1);
  int  tr2_index = trains.add_train("tr2", 100, 27.78, 2, 1);
  int  tr3_index = trains.add_train("tr3", 250, 20, 2, 1);

  // check the train indices
  EXPECT_EQ(trains.get_train_index("tr1"), tr1_index);
  EXPECT_EQ(trains.get_train_index("tr2"), tr2_index);
  EXPECT_EQ(trains.get_train_index("tr3"), tr3_index);

  // Write the train list to a file
  trains.export_trains("./tmp/write_trains_test");

  // Read the train list from the file
  auto trains_read =
      cda_rail::TrainList::import_trains("./tmp/write_trains_test");

  // Delete created directory and everything in it
  std::filesystem::remove_all("./tmp");

  // Check if the all trains are imported
  EXPECT_EQ(trains_read.size(), 3);
  EXPECT_TRUE(trains_read.has_train("tr1"));
  EXPECT_TRUE(trains_read.has_train("tr2"));
  EXPECT_TRUE(trains_read.has_train("tr3"));

  // Check if the train tr1 is imported correctly
  auto tr1 = trains_read.get_train("tr1");
  EXPECT_EQ(tr1.name, "tr1");
  EXPECT_EQ(tr1.length, 100);
  EXPECT_EQ(tr1.max_speed, 83.33);
  EXPECT_EQ(tr1.acceleration, 2);
  EXPECT_EQ(tr1.deceleration, 1);

  // Check if the train tr2 is imported correctly
  auto tr2 = trains_read.get_train("tr2");
  EXPECT_EQ(tr2.name, "tr2");
  EXPECT_EQ(tr2.length, 100);
  EXPECT_EQ(tr2.max_speed, 27.78);
  EXPECT_EQ(tr2.acceleration, 2);
  EXPECT_EQ(tr2.deceleration, 1);

  // Check if the train tr3 is imported correctly
  auto tr3 = trains_read.get_train("tr3");
  EXPECT_EQ(tr3.name, "tr3");
  EXPECT_EQ(tr3.length, 250);
  EXPECT_EQ(tr3.max_speed, 20);
  EXPECT_EQ(tr3.acceleration, 2);
  EXPECT_EQ(tr3.deceleration, 1);
}

TEST(Functionality, IsDirectory) {
  EXPECT_TRUE(cda_rail::is_directory_and_create("./tmp/is_directory"));
  EXPECT_TRUE(cda_rail::is_directory_and_create("./tmp/is_directory"));
  std::filesystem::remove_all("./tmp");
  EXPECT_TRUE(cda_rail::is_directory_and_create("./tmp/is_directory/"));
  EXPECT_TRUE(cda_rail::is_directory_and_create("./tmp/is_directory/"));
  std::filesystem::remove_all("./tmp");
  EXPECT_TRUE(cda_rail::is_directory_and_create("./tmp/"));
  EXPECT_TRUE(cda_rail::is_directory_and_create("./tmp/"));
  std::filesystem::remove_all("./tmp");
  EXPECT_TRUE(cda_rail::is_directory_and_create(R"(.\tmp\is_directory\)"));
  EXPECT_TRUE(cda_rail::is_directory_and_create(R"(.\tmp\is_directory\)"));
  std::filesystem::remove_all("./tmp");
  EXPECT_TRUE(cda_rail::is_directory_and_create(R"(.\tmp\is_directory)"));
  EXPECT_TRUE(cda_rail::is_directory_and_create(R"(.\tmp\is_directory)"));
  std::filesystem::remove_all("./tmp");
  EXPECT_TRUE(cda_rail::is_directory_and_create(R"(.\tmp\)"));
  EXPECT_TRUE(cda_rail::is_directory_and_create(R"(.\tmp\)"));
  std::filesystem::remove_all("./tmp");
  EXPECT_TRUE(cda_rail::is_directory_and_create(R"(.\tmp)"));
  EXPECT_TRUE(cda_rail::is_directory_and_create(R"(.\tmp)"));
  std::filesystem::remove_all("./tmp");
}

TEST(Functionality, ReadStation) {
  auto network = cda_rail::Network::import_network(
      "./example-networks/SimpleStation/network/");
  auto stations = cda_rail::StationList::import_stations(
      "./example-networks/SimpleStation/timetable/", network);

  // Check if the station is imported correctly
  EXPECT_EQ(stations.size(), 1);
  EXPECT_TRUE(stations.has_station("Central"));

  // Check if the station is imported correctly
  auto& station = stations.get_station("Central");
  EXPECT_EQ(station.name, "Central");
  EXPECT_EQ(station.tracks.size(), 4);
  std::vector<int> track_ids{network.get_edge_index("g00", "g01"),
                             network.get_edge_index("g10", "g11"),
                             network.get_edge_index("g01", "g00"),
                             network.get_edge_index("g11", "g10")};
  auto             station_tracks = station.tracks;
  std::sort(station_tracks.begin(), station_tracks.end());
  std::sort(track_ids.begin(), track_ids.end());
  EXPECT_EQ(station_tracks, track_ids);
}

TEST(Functionality, WriteStations) {
  auto network = cda_rail::Network::import_network(
      "./example-networks/SimpleStation/network/");
  cda_rail::StationList stations;

  stations.add_station("S1");
  stations.add_station("S2");

  stations.add_track_to_station("S1", "l0", "l1", network);
  stations.add_track_to_station("S2", "l0", "l1", network);
  stations.add_track_to_station("S2", "l1", "l2", network);

  stations.export_stations("./tmp/write_stations_test", network);
  auto stations_read = cda_rail::StationList::import_stations(
      "./tmp/write_stations_test", network);

  std::filesystem::remove_all("./tmp");

  EXPECT_EQ(stations_read.size(), 2);
  EXPECT_TRUE(stations_read.has_station("S1"));
  EXPECT_TRUE(stations_read.has_station("S2"));

  auto& s1 = stations_read.get_station("S1");
  EXPECT_EQ(s1.name, "S1");
  EXPECT_EQ(s1.tracks.size(), 1);
  std::vector<int> s1_tracks{network.get_edge_index("l0", "l1")};
  EXPECT_EQ(s1.tracks, s1_tracks);

  auto& s2 = stations_read.get_station("S2");
  EXPECT_EQ(s2.name, "S2");
  EXPECT_EQ(s2.tracks.size(), 2);
  std::vector<int> s2_tracks_target{network.get_edge_index("l0", "l1"),
                                    network.get_edge_index("l1", "l2")};
  auto             s2_tracks = s2.tracks;
  std::sort(s2_tracks.begin(), s2_tracks.end());
  std::sort(s2_tracks_target.begin(), s2_tracks_target.end());
  EXPECT_EQ(s2_tracks, s2_tracks_target);
}

TEST(Functionality, ReadTimetable) {
  auto network = cda_rail::Network::import_network(
      "./example-networks/SimpleStation/network/");
  auto timetable = cda_rail::Timetable::import_timetable(
      "./example-networks/SimpleStation/timetable/", network);

  // Check if the timetable has the correct stations
  auto& stations = timetable.get_station_list();
  EXPECT_EQ(stations.size(), 1);
  EXPECT_TRUE(stations.has_station("Central"));

  // Check if the station is imported correctly
  auto& station = stations.get_station("Central");
  EXPECT_EQ(station.name, "Central");
  EXPECT_EQ(station.tracks.size(), 4);
  std::vector<int> track_ids_target{network.get_edge_index("g00", "g01"),
                                    network.get_edge_index("g10", "g11"),
                                    network.get_edge_index("g01", "g00"),
                                    network.get_edge_index("g11", "g10")};
  auto             track_ids = station.tracks;
  std::sort(track_ids.begin(), track_ids.end());
  std::sort(track_ids_target.begin(), track_ids_target.end());
  EXPECT_EQ(track_ids, track_ids_target);

  // Check if the timetable has the correct trains
  auto& trains = timetable.get_train_list();
  // Check if the all trains are imported
  EXPECT_EQ(trains.size(), 3);
  EXPECT_TRUE(trains.has_train("tr1"));
  EXPECT_TRUE(trains.has_train("tr2"));
  EXPECT_TRUE(trains.has_train("tr3"));
  // Check if the train tr1 is imported correctly
  auto tr1 = trains.get_train("tr1");
  EXPECT_EQ(tr1.name, "tr1");
  EXPECT_EQ(tr1.length, 100);
  EXPECT_EQ(tr1.max_speed, 83.33);
  EXPECT_EQ(tr1.acceleration, 2);
  EXPECT_EQ(tr1.deceleration, 1);
  // Check if the train tr2 is imported correctly
  auto tr2 = trains.get_train("tr2");
  EXPECT_EQ(tr2.name, "tr2");
  EXPECT_EQ(tr2.length, 100);
  EXPECT_EQ(tr2.max_speed, 27.78);
  EXPECT_EQ(tr2.acceleration, 2);
  EXPECT_EQ(tr2.deceleration, 1);
  // Check if the train tr3 is imported correctly
  auto tr3 = trains.get_train("tr3");
  EXPECT_EQ(tr3.name, "tr3");
  EXPECT_EQ(tr3.length, 250);
  EXPECT_EQ(tr3.max_speed, 20);
  EXPECT_EQ(tr3.acceleration, 2);
  EXPECT_EQ(tr3.deceleration, 1);

  // Check the schedule of tr1
  auto& tr1_schedule = timetable.get_schedule("tr1");
  EXPECT_EQ(tr1_schedule.t_0, 120);
  EXPECT_EQ(tr1_schedule.v_0, 0);
  EXPECT_EQ(tr1_schedule.t_n, 645);
  EXPECT_EQ(tr1_schedule.v_n, 16.67);
  EXPECT_EQ(network.get_vertex(tr1_schedule.entry).name, "l0");
  EXPECT_EQ(network.get_vertex(tr1_schedule.exit).name, "r0");
  EXPECT_EQ(tr1_schedule.stops.size(), 1);
  auto& stop = tr1_schedule.stops[0];
  EXPECT_EQ(stop.begin, 240);
  EXPECT_EQ(stop.end, 300);
  EXPECT_EQ(stations.get_station(stop.station).name, "Central");

  // Check the schedule of tr2
  auto& tr2_schedule = timetable.get_schedule("tr2");
  EXPECT_EQ(tr2_schedule.t_0, 0);
  EXPECT_EQ(tr2_schedule.v_0, 0);
  EXPECT_EQ(tr2_schedule.t_n, 420);
  EXPECT_EQ(tr2_schedule.v_n, 16.67);
  EXPECT_EQ(network.get_vertex(tr2_schedule.entry).name, "l0");
  EXPECT_EQ(network.get_vertex(tr2_schedule.exit).name, "r0");
  EXPECT_EQ(tr2_schedule.stops.size(), 1);
  auto& stop2 = tr2_schedule.stops[0];
  EXPECT_EQ(stop2.begin, 120);
  EXPECT_EQ(stop2.end, 300);
  EXPECT_EQ(stations.get_station(stop2.station).name, "Central");

  // Check the schedule of tr3
  auto& tr3_schedule = timetable.get_schedule("tr3");
  EXPECT_EQ(tr3_schedule.t_0, 0);
  EXPECT_EQ(tr3_schedule.v_0, 0);
  EXPECT_EQ(tr3_schedule.t_n, 420);
  EXPECT_EQ(tr3_schedule.v_n, 16.67);
  EXPECT_EQ(network.get_vertex(tr3_schedule.entry).name, "r0");
  EXPECT_EQ(network.get_vertex(tr3_schedule.exit).name, "l0");
  EXPECT_EQ(tr3_schedule.stops.size(), 1);
  auto& stop3 = tr3_schedule.stops[0];
  EXPECT_EQ(stop3.begin, 180);
  EXPECT_EQ(stop3.end, 300);
  EXPECT_EQ(stations.get_station(stop3.station).name, "Central");

  EXPECT_EQ(timetable.max_t(), 645);

  EXPECT_TRUE(timetable.check_consistency(network));
}

TEST(Functionality, WriteTimetable) {
  auto network = cda_rail::Network::import_network(
      "./example-networks/SimpleStation/network/");
  cda_rail::Timetable timetable;

  timetable.add_train("tr1", 100, 83.33, 2, 1, 0, 0, "l0", 300, 20, "r0",
                      network);
  timetable.add_train("tr2", 100, 27.78, 2, 1, 0, 0, "r0", 300, 20, "l0",
                      network);

  std::pair<int, int> time_interval_expected{0, 300};

  EXPECT_EQ(timetable.time_interval("tr1"), time_interval_expected);
  EXPECT_EQ(timetable.time_interval("tr2"), time_interval_expected);

  timetable.add_station("Station1");
  timetable.add_station("Station2");

  timetable.add_track_to_station("Station1", "g00", "g01", network);
  timetable.add_track_to_station("Station1", "g10", "g11", network);
  timetable.add_track_to_station("Station1", "g01", "g00", network);
  timetable.add_track_to_station("Station1", "g11", "g10", network);
  timetable.add_track_to_station("Station2", "r1", "r0", network);

  timetable.add_stop("tr1", "Station1", 100, 160);
  timetable.add_stop("tr1", "Station2", 200, 260);
  timetable.add_stop("tr2", "Station1", 160, 220);

  // Check if the timetable is as expected
  // Check if the timetable has the correct stations
  auto& stations = timetable.get_station_list();
  EXPECT_EQ(stations.size(), 2);
  EXPECT_TRUE(stations.has_station("Station1"));
  EXPECT_TRUE(stations.has_station("Station2"));

  // Check if the stations are imported correctly
  auto& st1 = stations.get_station("Station1");
  EXPECT_EQ(st1.name, "Station1");
  EXPECT_EQ(st1.tracks.size(), 4);
  std::vector<int> s1_expected_tracks = {network.get_edge_index("g00", "g01"),
                                         network.get_edge_index("g10", "g11"),
                                         network.get_edge_index("g01", "g00"),
                                         network.get_edge_index("g11", "g10")};
  auto             st1_tracks         = st1.tracks;
  std::sort(st1_tracks.begin(), st1_tracks.end());
  std::sort(s1_expected_tracks.begin(), s1_expected_tracks.end());
  EXPECT_EQ(st1_tracks, s1_expected_tracks);
  auto& st2 = stations.get_station("Station2");
  EXPECT_EQ(st2.name, "Station2");
  EXPECT_EQ(st2.tracks.size(), 1);
  std::vector<int> s2_expected_tracks = {network.get_edge_index("r1", "r0")};
  EXPECT_EQ(st2.tracks, s2_expected_tracks);

  // Check if the timetable has the correct trains
  auto& trains = timetable.get_train_list();
  EXPECT_EQ(trains.size(), 2);
  EXPECT_TRUE(trains.has_train("tr1"));
  EXPECT_TRUE(trains.has_train("tr2"));

  // Check if the train tr1 is saved correctly
  auto tr1 = trains.get_train("tr1");
  EXPECT_EQ(tr1.name, "tr1");
  EXPECT_EQ(tr1.length, 100);
  EXPECT_EQ(tr1.max_speed, 83.33);
  EXPECT_EQ(tr1.acceleration, 2);
  EXPECT_EQ(tr1.deceleration, 1);
  // Check if the train tr2 is saved correctly
  auto tr2 = trains.get_train("tr2");
  EXPECT_EQ(tr2.name, "tr2");
  EXPECT_EQ(tr2.length, 100);
  EXPECT_EQ(tr2.max_speed, 27.78);
  EXPECT_EQ(tr2.acceleration, 2);
  EXPECT_EQ(tr2.deceleration, 1);

  // Check if the schedule of tr1 is saved correctly
  auto& tr1_schedule = timetable.get_schedule("tr1");
  EXPECT_EQ(tr1_schedule.t_0, 0);
  EXPECT_EQ(tr1_schedule.v_0, 0);
  EXPECT_EQ(tr1_schedule.t_n, 300);
  EXPECT_EQ(tr1_schedule.v_n, 20);
  EXPECT_EQ(network.get_vertex(tr1_schedule.entry).name, "l0");
  EXPECT_EQ(network.get_vertex(tr1_schedule.exit).name, "r0");
  EXPECT_EQ(tr1_schedule.stops.size(), 2);
  auto& stop1 = tr1_schedule.stops[0];
  EXPECT_EQ(stop1.begin, 100);
  EXPECT_EQ(stop1.end, 160);
  EXPECT_EQ(stations.get_station(stop1.station).name, "Station1");
  auto& stop2 = tr1_schedule.stops[1];
  EXPECT_EQ(stop2.begin, 200);
  EXPECT_EQ(stop2.end, 260);
  EXPECT_EQ(stations.get_station(stop2.station).name, "Station2");

  // Check if the schedule of tr2 is saved correctly
  auto& tr2_schedule = timetable.get_schedule("tr2");
  EXPECT_EQ(tr2_schedule.t_0, 0);
  EXPECT_EQ(tr2_schedule.v_0, 0);
  EXPECT_EQ(tr2_schedule.t_n, 300);
  EXPECT_EQ(tr2_schedule.v_n, 20);
  EXPECT_EQ(network.get_vertex(tr2_schedule.entry).name, "r0");
  EXPECT_EQ(network.get_vertex(tr2_schedule.exit).name, "l0");
  EXPECT_EQ(tr2_schedule.stops.size(), 1);
  auto& stop3 = tr2_schedule.stops[0];
  EXPECT_EQ(stop3.begin, 160);
  EXPECT_EQ(stop3.end, 220);
  EXPECT_EQ(stations.get_station(stop3.station).name, "Station1");

  // Write timetable to directory
  timetable.export_timetable("./tmp/test-timetable/", network);

  // Read timetable from directory
  auto timetable_read =
      cda_rail::Timetable::import_timetable("./tmp/test-timetable/", network);

  // Delete temporary files
  std::filesystem::remove_all("./tmp");

  // Check if the timetable is as expected
  // Check if the timetable has the correct stations
  auto& stations_read = timetable_read.get_station_list();
  EXPECT_EQ(stations_read.size(), 2);
  EXPECT_TRUE(stations_read.has_station("Station1"));
  EXPECT_TRUE(stations_read.has_station("Station2"));

  // Check if the stations are imported correctly
  auto& st1_read = stations_read.get_station("Station1");
  EXPECT_EQ(st1_read.name, "Station1");
  EXPECT_EQ(st1_read.tracks.size(), 4);
  auto st1_read_tracks = st1_read.tracks;
  std::sort(st1_read_tracks.begin(), st1_read_tracks.end());
  EXPECT_EQ(st1_read_tracks, s1_expected_tracks);
  auto& st2_read = stations_read.get_station("Station2");
  EXPECT_EQ(st2_read.name, "Station2");
  EXPECT_EQ(st2_read.tracks.size(), 1);
  auto st2_read_tracks = st2_read.tracks;
  std::sort(st2_read_tracks.begin(), st2_read_tracks.end());
  EXPECT_EQ(st2_read_tracks, s2_expected_tracks);

  // Check if the timetable has the correct trains
  auto& trains_read = timetable_read.get_train_list();
  EXPECT_EQ(trains_read.size(), 2);
  EXPECT_TRUE(trains_read.has_train("tr1"));
  EXPECT_TRUE(trains_read.has_train("tr2"));

  // Check if the train tr1 is saved correctly
  auto tr1_read = trains_read.get_train("tr1");
  EXPECT_EQ(tr1_read.name, "tr1");
  EXPECT_EQ(tr1_read.length, 100);
  EXPECT_EQ(tr1_read.max_speed, 83.33);
  EXPECT_EQ(tr1_read.acceleration, 2);
  EXPECT_EQ(tr1_read.deceleration, 1);
  // Check if the train tr2 is saved correctly
  auto tr2_read = trains_read.get_train("tr2");
  EXPECT_EQ(tr2_read.name, "tr2");
  EXPECT_EQ(tr2_read.length, 100);
  EXPECT_EQ(tr2_read.max_speed, 27.78);
  EXPECT_EQ(tr2_read.acceleration, 2);
  EXPECT_EQ(tr2_read.deceleration, 1);

  // Check if the schedule of tr1 is saved correctly
  auto& tr1_schedule_read = timetable_read.get_schedule("tr1");
  EXPECT_EQ(tr1_schedule_read.t_0, 0);
  EXPECT_EQ(tr1_schedule_read.v_0, 0);
  EXPECT_EQ(tr1_schedule_read.t_n, 300);
  EXPECT_EQ(tr1_schedule_read.v_n, 20);
  EXPECT_EQ(network.get_vertex(tr1_schedule_read.entry).name, "l0");
  EXPECT_EQ(network.get_vertex(tr1_schedule_read.exit).name, "r0");
  EXPECT_EQ(tr1_schedule_read.stops.size(), 2);
  auto& stop1_read = tr1_schedule_read.stops[0];
  EXPECT_EQ(stop1_read.begin, 100);
  EXPECT_EQ(stop1_read.end, 160);
  EXPECT_EQ(stations_read.get_station(stop1_read.station).name, "Station1");
  auto& stop2_read = tr1_schedule_read.stops[1];
  EXPECT_EQ(stop2_read.begin, 200);
  EXPECT_EQ(stop2_read.end, 260);
  EXPECT_EQ(stations_read.get_station(stop2_read.station).name, "Station2");

  // Check if the schedule of tr2 is saved correctly
  auto& tr2_schedule_read = timetable_read.get_schedule("tr2");
  EXPECT_EQ(tr2_schedule_read.t_0, 0);
  EXPECT_EQ(tr2_schedule_read.v_0, 0);
  EXPECT_EQ(tr2_schedule_read.t_n, 300);
  EXPECT_EQ(tr2_schedule_read.v_n, 20);
  EXPECT_EQ(network.get_vertex(tr2_schedule_read.entry).name, "r0");
  EXPECT_EQ(network.get_vertex(tr2_schedule_read.exit).name, "l0");
  EXPECT_EQ(tr2_schedule_read.stops.size(), 1);
  auto& stop3_read = tr2_schedule_read.stops[0];
  EXPECT_EQ(stop3_read.begin, 160);
  EXPECT_EQ(stop3_read.end, 220);
  EXPECT_EQ(stations_read.get_station(stop3_read.station).name, "Station1");
}

TEST(Functionality, RouteMap) {
  auto network = cda_rail::Network::import_network(
      "./example-networks/SimpleStation/network/");
  auto train_list = cda_rail::TrainList();

  train_list.add_train("tr1", 100, 83.33, 2, 1);
  train_list.add_train("tr2", 100, 27.78, 2, 1);

  auto route_map = cda_rail::RouteMap();

  EXPECT_ANY_THROW(route_map.add_empty_route("tr3", train_list));

  route_map.add_empty_route("tr1", train_list);
  route_map.push_back_edge("tr1", "l1", "l2", network);
  EXPECT_ANY_THROW(route_map.push_back_edge("tr1", "l0", "l2", network));
  EXPECT_ANY_THROW(route_map.push_back_edge("tr1", "l0", "l1", network));
  route_map.push_back_edge("tr1", "l2", "l3", network);
  EXPECT_ANY_THROW(route_map.push_front_edge("tr1", "l0", "l2", network));
  EXPECT_ANY_THROW(route_map.push_front_edge("tr1", "l3", "g00", network));
  route_map.push_front_edge("tr1", "l0", "l1", network);

  // Check if route consists of three edges passing vertices l0-l1-l2-l3 in this
  // order.
  auto& route = route_map.get_route("tr1");
  EXPECT_EQ(route.size(), 3);
  EXPECT_EQ(network.get_vertex(route.get_edge(0, network).source).name, "l0");
  EXPECT_EQ(network.get_vertex(route.get_edge(0, network).target).name, "l1");
  EXPECT_EQ(network.get_vertex(route.get_edge(1, network).source).name, "l1");
  EXPECT_EQ(network.get_vertex(route.get_edge(1, network).target).name, "l2");
  EXPECT_EQ(network.get_vertex(route.get_edge(2, network).source).name, "l2");
  EXPECT_EQ(network.get_vertex(route.get_edge(2, network).target).name, "l3");

  EXPECT_EQ(route.length(network), 1005);

  // Check if the consistency checking works as expected
  EXPECT_TRUE(route_map.check_consistency(train_list, network, false));
  EXPECT_FALSE(route_map.check_consistency(train_list, network, true));
  EXPECT_FALSE(route_map.check_consistency(train_list, network));

  route_map.add_empty_route("tr2");
  route_map.push_back_edge("tr2", "r0", "r1", network);
  route_map.push_back_edge("tr2", "r1", "r2", network);

  // Check if route consists of two edges passing vertices r0-r1-r2 in this
  // order.
  auto& route2 = route_map.get_route("tr2");
  EXPECT_EQ(route2.size(), 2);
  EXPECT_EQ(network.get_vertex(route2.get_edge(0, network).source).name, "r0");
  EXPECT_EQ(network.get_vertex(route2.get_edge(0, network).target).name, "r1");
  EXPECT_EQ(network.get_vertex(route2.get_edge(1, network).source).name, "r1");
  EXPECT_EQ(network.get_vertex(route2.get_edge(1, network).target).name, "r2");

  EXPECT_EQ(route2.length(network), 505);

  // Check route map length
  EXPECT_EQ(route_map.length("tr1", network), 1005);
  EXPECT_EQ(route_map.length("tr2", network), 505);

  // Check if the consistency checking works as expected
  EXPECT_TRUE(route_map.check_consistency(train_list, network, false));
  EXPECT_TRUE(route_map.check_consistency(train_list, network, true));
  EXPECT_TRUE(route_map.check_consistency(train_list, network));
}

TEST(Functionality, ImportRouteMap) {
  auto network = cda_rail::Network::import_network(
      "./example-networks/SimpleStation/network/");
  auto train_list = cda_rail::TrainList::import_trains(
      "./example-networks/SimpleStation/timetable/");
  auto route_map = cda_rail::RouteMap::import_routes(
      "./example-networks/SimpleStation/routes/", network);

  // Check if the route consists of thee trains with names "tr1", "tr2" and
  // "tr3"
  EXPECT_EQ(route_map.size(), 3);
  EXPECT_TRUE(route_map.has_route("tr1"));
  EXPECT_TRUE(route_map.has_route("tr2"));
  EXPECT_TRUE(route_map.has_route("tr3"));

  // Check if the route for tr1 consists of eight edges passing vertices
  // l0-l1-l2-l3-g00-g01-r2-r1-r0 in this order.
  auto& route = route_map.get_route("tr1");
  EXPECT_EQ(route.size(), 8);
  EXPECT_EQ(network.get_vertex(route.get_edge(0, network).source).name, "l0");
  EXPECT_EQ(network.get_vertex(route.get_edge(0, network).target).name, "l1");
  EXPECT_EQ(network.get_vertex(route.get_edge(1, network).source).name, "l1");
  EXPECT_EQ(network.get_vertex(route.get_edge(1, network).target).name, "l2");
  EXPECT_EQ(network.get_vertex(route.get_edge(2, network).source).name, "l2");
  EXPECT_EQ(network.get_vertex(route.get_edge(2, network).target).name, "l3");
  EXPECT_EQ(network.get_vertex(route.get_edge(3, network).source).name, "l3");
  EXPECT_EQ(network.get_vertex(route.get_edge(3, network).target).name, "g00");
  EXPECT_EQ(network.get_vertex(route.get_edge(4, network).source).name, "g00");
  EXPECT_EQ(network.get_vertex(route.get_edge(4, network).target).name, "g01");
  EXPECT_EQ(network.get_vertex(route.get_edge(5, network).source).name, "g01");
  EXPECT_EQ(network.get_vertex(route.get_edge(5, network).target).name, "r2");
  EXPECT_EQ(network.get_vertex(route.get_edge(6, network).source).name, "r2");
  EXPECT_EQ(network.get_vertex(route.get_edge(6, network).target).name, "r1");
  EXPECT_EQ(network.get_vertex(route.get_edge(7, network).source).name, "r1");
  EXPECT_EQ(network.get_vertex(route.get_edge(7, network).target).name, "r0");

  // Check if the route for tr2 consists of eight edges passing vertices
  // l0-l1-l2-l3-g00-g01-r2-r1-r0 in this order.
  auto& route2 = route_map.get_route("tr2");
  EXPECT_EQ(route2.size(), 8);
  EXPECT_EQ(network.get_vertex(route2.get_edge(0, network).source).name, "l0");
  EXPECT_EQ(network.get_vertex(route2.get_edge(0, network).target).name, "l1");
  EXPECT_EQ(network.get_vertex(route2.get_edge(1, network).source).name, "l1");
  EXPECT_EQ(network.get_vertex(route2.get_edge(1, network).target).name, "l2");
  EXPECT_EQ(network.get_vertex(route2.get_edge(2, network).source).name, "l2");
  EXPECT_EQ(network.get_vertex(route2.get_edge(2, network).target).name, "l3");
  EXPECT_EQ(network.get_vertex(route2.get_edge(3, network).source).name, "l3");
  EXPECT_EQ(network.get_vertex(route2.get_edge(3, network).target).name, "g00");
  EXPECT_EQ(network.get_vertex(route2.get_edge(4, network).source).name, "g00");
  EXPECT_EQ(network.get_vertex(route2.get_edge(4, network).target).name, "g01");
  EXPECT_EQ(network.get_vertex(route2.get_edge(5, network).source).name, "g01");
  EXPECT_EQ(network.get_vertex(route2.get_edge(5, network).target).name, "r2");
  EXPECT_EQ(network.get_vertex(route2.get_edge(6, network).source).name, "r2");
  EXPECT_EQ(network.get_vertex(route2.get_edge(6, network).target).name, "r1");
  EXPECT_EQ(network.get_vertex(route2.get_edge(7, network).source).name, "r1");
  EXPECT_EQ(network.get_vertex(route2.get_edge(7, network).target).name, "r0");

  // Check if the route for tr3 consists of eight edges passing vertices
  // r0-r1-r2-g11-g10-l3-l2-l1 in this order.
  auto& route3 = route_map.get_route("tr3");
  EXPECT_EQ(route3.size(), 8);
  EXPECT_EQ(network.get_vertex(route3.get_edge(0, network).source).name, "r0");
  EXPECT_EQ(network.get_vertex(route3.get_edge(0, network).target).name, "r1");
  EXPECT_EQ(network.get_vertex(route3.get_edge(1, network).source).name, "r1");
  EXPECT_EQ(network.get_vertex(route3.get_edge(1, network).target).name, "r2");
  EXPECT_EQ(network.get_vertex(route3.get_edge(2, network).source).name, "r2");
  EXPECT_EQ(network.get_vertex(route3.get_edge(2, network).target).name, "g11");
  EXPECT_EQ(network.get_vertex(route3.get_edge(3, network).source).name, "g11");
  EXPECT_EQ(network.get_vertex(route3.get_edge(3, network).target).name, "g10");
  EXPECT_EQ(network.get_vertex(route3.get_edge(4, network).source).name, "g10");
  EXPECT_EQ(network.get_vertex(route3.get_edge(4, network).target).name, "l3");
  EXPECT_EQ(network.get_vertex(route3.get_edge(5, network).source).name, "l3");
  EXPECT_EQ(network.get_vertex(route3.get_edge(5, network).target).name, "l2");
  EXPECT_EQ(network.get_vertex(route3.get_edge(6, network).source).name, "l2");
  EXPECT_EQ(network.get_vertex(route3.get_edge(6, network).target).name, "l1");
  EXPECT_EQ(network.get_vertex(route3.get_edge(7, network).source).name, "l1");
  EXPECT_EQ(network.get_vertex(route3.get_edge(7, network).target).name, "l0");

  // Check imported consistency
  EXPECT_TRUE(route_map.check_consistency(train_list, network, false));
  EXPECT_TRUE(route_map.check_consistency(train_list, network, true));
  EXPECT_TRUE(route_map.check_consistency(train_list, network));
}

TEST(Functionality, ExportRouteMap) {
  auto network = cda_rail::Network::import_network(
      "./example-networks/SimpleStation/network/");
  auto train_list = cda_rail::TrainList();
  train_list.add_train("tr1", 100, 83.33, 2, 1);
  train_list.add_train("tr2", 100, 27.78, 2, 1);
  auto route_map = cda_rail::RouteMap();
  route_map.add_empty_route("tr1", train_list);
  route_map.push_back_edge("tr1", "l1", "l2", network);
  route_map.push_back_edge("tr1", "l2", "l3", network);
  route_map.push_front_edge("tr1", "l0", "l1", network);
  route_map.add_empty_route("tr2");
  route_map.push_back_edge("tr2", "r0", "r1", network);
  route_map.push_back_edge("tr2", "r1", "r2", network);

  // Export and import route map
  route_map.export_routes("./tmp/write_route_map_test", network);
  auto route_map_read =
      cda_rail::RouteMap::import_routes("./tmp/write_route_map_test", network);
  std::filesystem::remove_all("./tmp");

  // Check if the route map is the same as the original one
  // Check if the route map contains two routes for tr1 and tr2
  EXPECT_EQ(route_map_read.size(), 2);
  EXPECT_TRUE(route_map_read.has_route("tr1"));
  EXPECT_TRUE(route_map_read.has_route("tr2"));

  // Check if the route for tr1 consists of three edges passing vertices
  // l0-l1-l2-l3 in this order.
  auto& route1 = route_map_read.get_route("tr1");
  EXPECT_EQ(route1.size(), 3);
  EXPECT_EQ(network.get_vertex(route1.get_edge(0, network).source).name, "l0");
  EXPECT_EQ(network.get_vertex(route1.get_edge(0, network).target).name, "l1");
  EXPECT_EQ(network.get_vertex(route1.get_edge(1, network).source).name, "l1");
  EXPECT_EQ(network.get_vertex(route1.get_edge(1, network).target).name, "l2");
  EXPECT_EQ(network.get_vertex(route1.get_edge(2, network).source).name, "l2");
  EXPECT_EQ(network.get_vertex(route1.get_edge(2, network).target).name, "l3");

  // Check if the route for tr2 consists of two edges passing vertices r0-r1-r2
  // in this order.
  auto& route2 = route_map_read.get_route("tr2");
  EXPECT_EQ(route2.size(), 2);
  EXPECT_EQ(network.get_vertex(route2.get_edge(0, network).source).name, "r0");
  EXPECT_EQ(network.get_vertex(route2.get_edge(0, network).target).name, "r1");
  EXPECT_EQ(network.get_vertex(route2.get_edge(1, network).source).name, "r1");
  EXPECT_EQ(network.get_vertex(route2.get_edge(1, network).target).name, "r2");

  // Check imported consistency
  EXPECT_TRUE(route_map_read.check_consistency(train_list, network, false));
  EXPECT_TRUE(route_map_read.check_consistency(train_list, network, true));
  EXPECT_TRUE(route_map_read.check_consistency(train_list, network));
}

TEST(Functionality, RouteMapHelper) {
  cda_rail::Network network;
  network.add_vertex("v0", cda_rail::VertexType::TTD);
  int v1 = network.add_vertex("v1", cda_rail::VertexType::TTD);
  int v2 = network.add_vertex("v2", cda_rail::VertexType::TTD);
  network.add_vertex("v3", cda_rail::VertexType::TTD);

  network.add_edge("v0", "v1", 10, 5, false);
  int v1_v2 = network.add_edge("v1", "v2", 20, 5, false);
  int v2_v3 = network.add_edge("v2", "v3", 30, 5, false);
  int v3_v2 = network.add_edge("v3", "v2", 30, 5, false);
  int v2_v1 = network.add_edge("v2", "v1", 20, 5, false);
  network.add_edge("v1", "v0", 10, 5, false);

  network.add_successor({"v0", "v1"}, {"v1", "v2"});
  network.add_successor({"v1", "v2"}, {"v2", "v3"});

  cda_rail::RouteMap route_map;
  route_map.add_empty_route("tr1");
  route_map.push_back_edge("tr1", "v0", "v1", network);
  route_map.push_back_edge("tr1", "v1", "v2", network);
  route_map.push_back_edge("tr1", "v2", "v3", network);

  const auto&               tr1_map    = route_map.get_route("tr1");
  const auto&               tr1_e1_pos = tr1_map.edge_pos("v0", "v1", network);
  std::pair<double, double> expected_tr1_e1_pos = {0, 10};
  EXPECT_EQ(tr1_e1_pos, expected_tr1_e1_pos);
  const auto&               tr1_e2_pos = tr1_map.edge_pos(v1, v2, network);
  std::pair<double, double> expected_tr1_e2_pos = {10, 30};
  EXPECT_EQ(tr1_e2_pos, expected_tr1_e2_pos);
  const auto&               tr1_e3_pos = tr1_map.edge_pos(v2_v3, network);
  std::pair<double, double> expected_tr1_e3_pos = {30, 60};
  EXPECT_EQ(tr1_e3_pos, expected_tr1_e3_pos);

  const auto& station_pos =
      tr1_map.edge_pos({v1_v2, v2_v1, v2_v3, v3_v2}, network);
  std::pair<double, double> expected_station_pos = {10, 60};
  EXPECT_EQ(station_pos, expected_station_pos);

  EXPECT_EQ(tr1_map.length(network), 60);
}

TEST(Functionality, Iterators) {
  // Create a train list
  auto trains = cda_rail::TrainList();
  trains.add_train("tr1", 100, 83.33, 2, 1);
  trains.add_train("tr2", 100, 27.78, 2, 1);
  trains.add_train("tr3", 250, 20, 2, 1);

  // Check range based for loop
  int i = 0;
  for (const auto& train : trains) {
    EXPECT_EQ(&train, &trains.get_train(i));
    i++;
  }

  // Create route map
  auto route_map = cda_rail::RouteMap();

  route_map.add_empty_route("tr1");
  route_map.add_empty_route("tr2");

  // Check range based for loop
  for (const auto& [name, route] : route_map) {
    EXPECT_EQ(&route, &route_map.get_route(name));
  }

  // Create stations
  cda_rail::StationList stations;
  stations.add_station("S1");
  stations.add_station("S2");

  // Check range based for loop
  for (const auto& [name, station] : stations) {
    EXPECT_EQ(&station, &stations.get_station(name));
  }

  // Create timetable
  auto network = cda_rail::Network::import_network(
      "./example-networks/SimpleStation/network/");
  cda_rail::Timetable timetable;

  timetable.add_train("tr1", 100, 83.33, 2, 1, 0, 0, "l0", 300, 20, "r0",
                      network);
  timetable.add_train("tr2", 100, 27.78, 2, 1, 0, 0, "r0", 300, 20, "l0",
                      network);

  // Check range based for loop
  // for (const auto& [name, schedule] : timetable) {
  //    EXPECT_EQ(&schedule, &timetable.get_schedule(name));
  //}
}
