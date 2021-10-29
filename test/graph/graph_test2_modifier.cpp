#include "gdwg/graph.hpp"

#include <catch2/catch.hpp>
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

TEST_CASE("Insert node") {
	auto g = gdwg::graph<int, std::string>{1};
	CHECK(g.insert_node(7));
	CHECK(g.insert_node(-1));

	CHECK(g.is_node(1));
	CHECK(g.is_node(-1));
	CHECK(g.is_node(7));

	SECTION("Insert existing node") {
		CHECK_FALSE(g.insert_node(7));
		CHECK_FALSE(g.insert_node(1));
	}
}

TEST_CASE("Insert edge") {
	auto g = gdwg::graph<std::string, int>{"Yoona", "Taeyeon", "Tzuyu"};

	SECTION("Insert legal edges") {
		CHECK(g.insert_edge("Yoona", "Tzuyu", 530));
		CHECK(g.insert_edge("Yoona", "Yoona", 530)); // Reflexive edge

		CHECK(g.find("Yoona", "Tzuyu", 530) != g.end());
		CHECK(g.find("Yoona", "Yoona", 530) != g.end());

		// Edge with the same src and dst but different weight
		CHECK(g.insert_edge("Yoona", "Tzuyu", 1314));
		CHECK(g.insert_edge("Yoona", "Yoona", 1314)); // Reflexive edge

		CHECK(g.find("Yoona", "Tzuyu", 1314) != g.end());
		CHECK(g.find("Yoona", "Yoona", 1314) != g.end());
	}

	SECTION("Insert existing node") {
		CHECK(g.insert_edge("Yoona", "Tzuyu", 530));
		CHECK(g.insert_edge("Yoona", "Yoona", 530)); // Reflexive edge

		CHECK_FALSE(g.insert_edge("Yoona", "Tzuyu", 530));
		CHECK_FALSE(g.insert_edge("Yoona", "Yoona", 530)); // Reflexive edge
	}

	SECTION("Exception: Either src or dst not exist") {
		// dst not exist
		CHECK_THROWS_MATCHES(g.insert_edge("Yoona", "Mina", 1314),
		                     std::runtime_error,
		                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::insert_edge "
		                                              "when "
		                                              "either src or dst node does not exist"));

		// src not exist
		CHECK_THROWS_MATCHES(g.insert_edge("Nayeon", "Taeyeon", 520),
		                     std::runtime_error,
		                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::insert_edge "
		                                              "when "
		                                              "either src or dst node does not exist"));

		// both not exist
		CHECK_THROWS_MATCHES(g.insert_edge("Nayeon", "Mina", 520),
		                     std::runtime_error,
		                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::insert_edge "
		                                              "when "
		                                              "either src or dst node does not exist"));
	}
}

TEST_CASE("Replace Node") {
	auto g = gdwg::graph<std::string, int>{"Yoona", "Taeyeon", "Tzuyu"};

	SECTION("Simple case: If node are replaced") {
		CHECK(g.replace_node("Yoona", "Rose"));

		CHECK(g.is_node("Rose"));
		CHECK_FALSE(g.is_node("Yoona"));
	}

	SECTION("Hard case: If edges are also replaced") {
		CHECK(g.insert_edge("Yoona", "Taeyeon", 818));
		CHECK(g.insert_edge("Taeyeon", "Yoona", 1314));
		CHECK(g.insert_edge("Yoona", "Yoona", 530));
		CHECK(g.insert_edge("Yoona", "Yoona", 1314));

		CHECK(g.replace_node("Yoona", "Rose"));

		// Check that the edges no longer exist
		CHECK(g.find("Yoona", "Taeyeon", 818) == g.end());
		CHECK(g.find("Taeyeon", "Yoona", 1314) == g.end());
		CHECK(g.find("Yoona", "Yoona", 530) == g.end());
		CHECK(g.find("Yoona", "Yoona", 1314) == g.end());

		// Check that the old node doesn't exist, and new node exist
		CHECK(g.is_node("Rose"));
		CHECK_FALSE(g.is_node("Yoona"));

		// Check we have these edges
		CHECK(g.find("Rose", "Taeyeon", 818) != g.end());
		CHECK(g.find("Taeyeon", "Rose", 1314) != g.end());
		CHECK(g.find("Rose", "Rose", 530) != g.end());
		CHECK(g.find("Rose", "Rose", 1314) != g.end());
	}

	SECTION("new_data already exist") {
		CHECK_FALSE(g.replace_node("Yoona", "Taeyeon"));

		CHECK(g.is_node("Yoona"));
		CHECK(g.is_node("Taeyeon"));
	}

	SECTION("Exception: is_node(old_data) == false") {
		// src, dst not exist
		CHECK_THROWS_MATCHES(g.replace_node("Yeonwoo", "Mina"),
		                     std::runtime_error,
		                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::replace_node "
		                                              "on a node that doesn't exist"));

		// src not exist, dst exist
		CHECK_THROWS_MATCHES(g.replace_node("Yeonwoo", "Taeyeon"),
		                     std::runtime_error,
		                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::replace_node "
		                                              "on a node that doesn't exist"));
	}
}

TEST_CASE("Merge and replace node") {
	auto g = gdwg::graph<std::string, int>{"Yoona", "Taeyeon", "Tzuyu"};

	SECTION("Simple case: No duplicate edge after replacement") {
		g.insert_edge("Yoona", "Taeyeon", 309);
		g.insert_edge("Yoona", "Yoona", 530);

		g.merge_replace_node("Yoona", "Tzuyu");

		// Check old edge are removed
		CHECK(g.find("Yoona", "Taeyeon", 309) == g.end());
		CHECK(g.find("Yoona", "Yoona", 530) == g.end());

		// Check the existence of the new edges
		CHECK(g.find("Tzuyu", "Tzuyu", 530) != g.end());
		CHECK(g.find("Tzuyu", "Taeyeon", 309) != g.end());

		// Check that the old node is remove
		CHECK_FALSE(g.is_node("Yoona"));
	}

	SECTION("Hard case: Edges need to be merged") {
		g.insert_edge("Yoona", "Tzuyu", 309);
		g.insert_edge("Tzuyu", "Tzuyu", 309);
		g.insert_edge("Yoona", "Taeyeon", 520);
		g.insert_edge("Tzuyu", "Taeyeon", 520);

		g.merge_replace_node("Yoona", "Tzuyu");

		// Check that the old node is remove
		CHECK_FALSE(g.is_node("Yoona"));

		// Check that edges have been merged, by checking the printed graph
		auto oss = std::ostringstream();
		oss << g;
		auto const expected_oss = std::string_view(R"(Taeyeon (
)
Tzuyu (
  Taeyeon | 520
  Tzuyu | 309
)
)");
		CHECK(oss.str() == expected_oss);
	}

	SECTION("Exception: either is_node(old_data) or is_node(new_data) are false") {
		// src not exist, dst exist
		CHECK_THROWS_MATCHES(g.merge_replace_node("Yeonwoo", "Taeyeon"),
		                     std::runtime_error,
		                     Catch::Matchers::Message("Cannot call gdwg::graph<N, "
		                                              "E>::merge_replace_node on old or new data if "
		                                              "they don't exist in the graph"));

		// dst not exist, src exist
		CHECK_THROWS_MATCHES(g.merge_replace_node("Taeyeon", "Yeonwoo"),
		                     std::runtime_error,
		                     Catch::Matchers::Message("Cannot call gdwg::graph<N, "
		                                              "E>::merge_replace_node on old or new data if "
		                                              "they don't exist in the graph"));

		// Both not exist
		CHECK_THROWS_MATCHES(g.merge_replace_node("Yeonwoo", "Mina"),
		                     std::runtime_error,
		                     Catch::Matchers::Message("Cannot call gdwg::graph<N, "
		                                              "E>::merge_replace_node on old or new data if "
		                                              "they don't exist in the graph"));
	}
}

TEST_CASE("Erase node") {
	auto g = gdwg::graph<std::string, int>{"Yoona", "Taeyeon", "Tzuyu"};

	SECTION("Erase not exist node") {
		CHECK_FALSE(g.erase_node("Twice"));
	}

	SECTION("No edge situtation") {
		CHECK(g.erase_node("Yoona"));

		CHECK(g.is_node("Yoona") == false);
	}

	SECTION("With edge: Relevant edges should be removed") {
		g.insert_edge("Yoona", "Yoona", 530);
		g.insert_edge("Yoona", "Yoona", 520);
		g.insert_edge("Yoona", "Tzuyu", 309);
		g.insert_edge("Taeyeon", "Tzuyu", 309);

		CHECK(g.erase_node("Yoona"));

		// Non-relevant is not removed
		CHECK(g.find("Taeyeon", "Tzuyu", 309) != g.end());

		// Relevant ones are removed
		CHECK(g.find("Yoona", "Yoona", 530) == g.end());
		CHECK(g.find("Yoona", "Yoona", 520) == g.end());
		CHECK(g.find("Yoona", "Tzuyu", 309) == g.end());
	}
}

TEST_CASE("Erase edge: (src, dst, weight)") {
	auto g = gdwg::graph<std::string, int>{"Yoona", "Taeyeon", "Tzuyu"};

	SECTION("Remove not exist edge") {
		CHECK_FALSE(g.erase_edge("Yoona", "Taeyeon", 520));
	}

	SECTION("Remove an edge successfully") {
		g.insert_edge("Tzuyu", "Taeyeon", 2);
		g.insert_edge("Tzuyu", "Taeyeon", 4);
		g.insert_edge("Yoona", "Taeyeon", 666);

		CHECK(g.erase_edge("Tzuyu", "Taeyeon", 4));

		// Check the remaining edges are correct
		CHECK(g.find("Tzuyu", "Taeyeon", 4) == g.end());

		CHECK(g.find("Tzuyu", "Taeyeon", 2) != g.end());
		CHECK(g.find("Yoona", "Taeyeon", 666) != g.end());
	}

	SECTION("Exception: either is_node(src) or is_node(dst) is false.") {
		// dst not exist
		CHECK_THROWS_MATCHES(g.erase_edge("Taeyeon", "Mina", 1314),
		                     std::runtime_error,
		                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::erase_edge on "
		                                              "src or dst if they don't exist in the graph"));

		// // src not exist
		CHECK_THROWS_MATCHES(g.erase_edge("Yeonwoo", "Taeyeon", 530),
		                     std::runtime_error,
		                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::erase_edge on "
		                                              "src or dst if they don't exist in the graph"));

		// // Both not exist
		CHECK_THROWS_MATCHES(g.erase_edge("Yeonwoo", "Mina", 520),
		                     std::runtime_error,
		                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::erase_edge on "
		                                              "src or dst if they don't exist in the graph"));
	}
}

TEST_CASE("Erase edge: (iterator i)") {
	auto g = gdwg::graph<std::string, int>{"Yoona", "Taeyeon", "Tzuyu"};

	SECTION("Remove not exist edge") {
		CHECK(g.erase_edge(g.begin()) == g.end());
		CHECK(g.erase_edge(g.end()) == g.end());
		CHECK(g.erase_edge(gdwg::graph<std::string, int>::iterator()) == g.end());
	}

	SECTION("Remove from a graph with single edge") {
		g.insert_edge("Tzuyu", "Taeyeon", 2);

		CHECK(g.erase_edge(g.begin()) == g.end());
		CHECK(g.find("Tzuyu", "Taeyeon", 2) == g.end());
	}

	SECTION("Remove from a graph with multiple edges") {
		g.insert_edge("Tzuyu", "Taeyeon", 2);
		g.insert_edge("Yoona", "Taeyeon", 666);
		g.insert_edge("Tzuyu", "Taeyeon", 4);

		SECTION("Test 1") {
			CHECK(g.erase_edge(g.find("Tzuyu", "Taeyeon", 2)) == g.find("Tzuyu", "Taeyeon", 4));

			// Only the particular edge is removed
			CHECK(g.find("Tzuyu", "Taeyeon", 2) == g.end());
			CHECK(g.find("Yoona", "Taeyeon", 666) != g.end());
			CHECK(g.find("Tzuyu", "Taeyeon", 4) != g.end());
		}

		SECTION("Test 2") {
			CHECK(g.erase_edge(g.find("Yoona", "Taeyeon", 666)) == g.end());
		}

		SECTION("Test 3") {
			CHECK(g.erase_edge(g.find("Tzuyu", "Taeyeon", 4)) == g.find("Yoona", "Taeyeon", 666));
		}

		SECTION("Test 4") {
			CHECK(g.erase_edge(g.find("Tzuyu", "Taeyeon", 4)) == g.find("Yoona", "Taeyeon", 666));
			CHECK(g.erase_edge(g.find("Tzuyu", "Taeyeon", 2)) == g.find("Yoona", "Taeyeon", 666));
			CHECK(g.erase_edge(g.find("Yoona", "Taeyeon", 666)) == g.end());
		}
	}
}

TEST_CASE("Erase edge: (iterator i, iterator s)") {
	auto g = gdwg::graph<std::string, int>{"Yoona", "Taeyeon", "Tzuyu"};

	SECTION("Remove from a graph with single edge") {
		g.insert_edge("Tzuyu", "Taeyeon", 2);

		CHECK(g.erase_edge(g.begin(), g.end()) == g.end());
		CHECK(g.find("Tzuyu", "Taeyeon", 2) == g.end());
	}

	SECTION("Remove from a graph with multiple edges") {
		g.insert_edge("Tzuyu", "Taeyeon", 2);
		g.insert_edge("Yoona", "Taeyeon", 666);
		g.insert_edge("Tzuyu", "Taeyeon", 4);

		SECTION("Test 1") {
			CHECK(g.erase_edge(g.find("Tzuyu", "Taeyeon", 2), g.find("Tzuyu", "Taeyeon", 4))
			      == g.find("Tzuyu", "Taeyeon", 4));

			// Only the particular edge is removed
			CHECK(g.find("Tzuyu", "Taeyeon", 2) == g.end());
			CHECK(g.find("Yoona", "Taeyeon", 666) != g.end());
			CHECK(g.find("Tzuyu", "Taeyeon", 4) != g.end());
		}

		SECTION("Test 2") {
			CHECK(g.erase_edge(g.find("Tzuyu", "Taeyeon", 4), g.end()) == g.end());

			CHECK(g.find("Tzuyu", "Taeyeon", 2) != g.end());
		}
	}
}

TEST_CASE("Clear") {
	SECTION("Default constructed emtpy graph") {
		auto g = gdwg::graph<std::string, int>{};
		g.clear();
		CHECK(g.empty());
	}

	SECTION("Graph with nodes") {
		auto g = gdwg::graph<std::string, int>{"one", "two"};
		g.clear();
		CHECK(g.empty());
	}

	SECTION("Graph with nodes and edges") {
		auto g = gdwg::graph<std::string, int>{"one", "two"};
		g.insert_edge("one", "two", 2);
		g.clear();
		CHECK(g.empty());
	}
}