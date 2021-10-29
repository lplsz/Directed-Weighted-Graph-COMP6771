#include "gdwg/graph.hpp"

#include <catch2/catch.hpp>
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

TEST_CASE("Test for a bunch of accessor functions") {
	auto g = gdwg::graph<std::string, int>{"Yoona", "Taeyeon", "Tzuyu"};
	g.insert_edge("Yoona", "Taeyeon", 818);
	g.insert_edge("Yoona", "Yoona", 530);
	g.insert_edge("Tzuyu", "Taeyeon", 1314);

	auto const g_copy_const = g;

	SECTION("Non-const is_node()") {
		CHECK(g.is_node("Yoona"));
		CHECK(g.is_node("Taeyeon"));
		CHECK(g.is_node("Tzuyu"));

		CHECK_FALSE(g.is_node("Yeonwoo"));
	}

	SECTION("Const is_node()") {
		CHECK(g_copy_const.is_node("Yoona"));
		CHECK(g_copy_const.is_node("Taeyeon"));
		CHECK(g_copy_const.is_node("Tzuyu"));

		CHECK_FALSE(g_copy_const.is_node("Yeonwoo"));
	}

	SECTION("empty()") {
		CHECK(gdwg::graph<std::string, int>{}.empty());
		CHECK_FALSE(g.empty());
		CHECK_FALSE(g_copy_const.empty());
	}

	SECTION("is_connected()") {
		SECTION("Non-const: Test has connection and no connection") {
			CHECK(g.is_connected("Yoona", "Taeyeon"));
			CHECK(g.is_connected("Yoona", "Yoona"));
			CHECK(g.is_connected("Tzuyu", "Taeyeon"));

			CHECK_FALSE(g.is_connected("Taeyeon", "Taeyeon"));
			CHECK_FALSE(g.is_connected("Taeyeon", "Tzuyu"));
		}

		SECTION("Const: Test has connection and no connection") {
			CHECK(g_copy_const.is_connected("Yoona", "Taeyeon"));
			CHECK(g_copy_const.is_connected("Yoona", "Yoona"));
			CHECK(g_copy_const.is_connected("Tzuyu", "Taeyeon"));

			CHECK_FALSE(g_copy_const.is_connected("Taeyeon", "Taeyeon"));
			CHECK_FALSE(g_copy_const.is_connected("Taeyeon", "Tzuyu"));
		}

		SECTION("Non-const Exception: either of is_node(src) or is_node(dst) are false") {
			// src not exist
			CHECK_THROWS_MATCHES(g.is_connected("Yeonwoo", "Tzuyu"),
			                     std::runtime_error,
			                     Catch::Matchers::Message("Cannot call gdwg::graph<N, "
			                                              "E>::is_connected if src or dst node don't "
			                                              "exist in the graph"));

			// dst not exist
			CHECK_THROWS_MATCHES(g.is_connected("Taeyeon", "Mina"),
			                     std::runtime_error,
			                     Catch::Matchers::Message("Cannot call gdwg::graph<N, "
			                                              "E>::is_connected if src or dst node don't "
			                                              "exist in the graph"));

			// src, dst not exist
			CHECK_THROWS_MATCHES(g.is_connected("Yeonwoo", "Mina"),
			                     std::runtime_error,
			                     Catch::Matchers::Message("Cannot call gdwg::graph<N, "
			                                              "E>::is_connected if src or dst node don't "
			                                              "exist in the graph"));
		}

		SECTION("Const Exception: either of is_node(src) or is_node(dst) are false") {
			// src not exist
			CHECK_THROWS_MATCHES(g_copy_const.is_connected("Yeonwoo", "Tzuyu"),
			                     std::runtime_error,
			                     Catch::Matchers::Message("Cannot call gdwg::graph<N, "
			                                              "E>::is_connected if src or dst node don't "
			                                              "exist in the graph"));

			// dst not exist
			CHECK_THROWS_MATCHES(g_copy_const.is_connected("Taeyeon", "Mina"),
			                     std::runtime_error,
			                     Catch::Matchers::Message("Cannot call gdwg::graph<N, "
			                                              "E>::is_connected if src or dst node don't "
			                                              "exist in the graph"));

			// src, dst not exist
			CHECK_THROWS_MATCHES(g_copy_const.is_connected("Yeonwoo", "Mina"),
			                     std::runtime_error,
			                     Catch::Matchers::Message("Cannot call gdwg::graph<N, "
			                                              "E>::is_connected if src or dst node don't "
			                                              "exist in the graph"));
		}
	}

	SECTION("nodes") {
		CHECK(gdwg::graph<std::string, int>{}.nodes().empty());
		CHECK(g.nodes() == std::vector<std::string>{"Taeyeon", "Tzuyu", "Yoona"});
		CHECK(g_copy_const.nodes() == std::vector<std::string>{"Taeyeon", "Tzuyu", "Yoona"});
	}

	SECTION("weights") {
		SECTION("Check nodes with no connection") {
			CHECK(g.weights("Taeyeon", "Tzuyu").empty());
			CHECK(g.weights("Taeyeon", "Yoona").empty());

			CHECK(g_copy_const.weights("Taeyeon", "Tzuyu").empty());
			CHECK(g_copy_const.weights("Taeyeon", "Yoona").empty());
		}

		SECTION("Check if the weights are in the right order") {
			g.insert_edge("Yoona", "Yoona", 250);
			g.insert_edge("Yoona", "Taeyeon", 1000);
			g.insert_edge("Yoona", "Taeyeon", 530);

			CHECK(g.weights("Yoona", "Yoona") == std::vector<int>{250, 530});
			CHECK(g.weights("Yoona", "Taeyeon") == std::vector<int>{530, 818, 1000});

			// Check const
			auto const g_const = g;

			CHECK(g_const.weights("Yoona", "Yoona") == std::vector<int>{250, 530});
			CHECK(g_const.weights("Yoona", "Taeyeon") == std::vector<int>{530, 818, 1000});
		}

		SECTION("Non-const Exception: either of is_node(src) or is_node(dst) are false") {
			// src not exist
			CHECK_THROWS_MATCHES(g.weights("Yeonwoo", "Tzuyu"),
			                     std::runtime_error,
			                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::weights if "
			                                              "src or dst node don't exist in the graph"));

			// dst not exist
			CHECK_THROWS_MATCHES(g.weights("Taeyeon", "Mina"),
			                     std::runtime_error,
			                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::weights if "
			                                              "src or dst node don't exist in the graph"));

			// src, dst not exist
			CHECK_THROWS_MATCHES(g.weights("Yeonwoo", "Mina"),
			                     std::runtime_error,
			                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::weights if "
			                                              "src or dst node don't exist in the graph"));
		}

		SECTION("Const Exception: either of is_node(src) or is_node(dst) are false") {
			// src not exist
			CHECK_THROWS_MATCHES(g_copy_const.weights("Yeonwoo", "Tzuyu"),
			                     std::runtime_error,
			                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::weights if "
			                                              "src or dst node don't exist in the graph"));

			// dst not exist
			CHECK_THROWS_MATCHES(g_copy_const.weights("Taeyeon", "Mina"),
			                     std::runtime_error,
			                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::weights if "
			                                              "src or dst node don't exist in the graph"));

			// src, dst not exist
			CHECK_THROWS_MATCHES(g_copy_const.weights("Yeonwoo", "Mina"),
			                     std::runtime_error,
			                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::weights if "
			                                              "src or dst node don't exist in the graph"));
		}
	}

	SECTION("find()") {
		SECTION("Non-const: Edge not exist") {
			CHECK(g.find("Yoona", "Tzuyu", 100) == g.end());
			CHECK(g.find("Yoona", "Yoona", 520) == g.end());
			CHECK(g.find("Yeonwoo", "Tzuyu", 100) == g.end());
			CHECK(g.find("Yeonwoo", "Mina", 100) == g.end());
		}

		SECTION("Const: Edge not exist") {
			CHECK(g_copy_const.find("Yoona", "Tzuyu", 100) == g_copy_const.end());
			CHECK(g_copy_const.find("Yoona", "Yoona", 520) == g_copy_const.end());
			CHECK(g_copy_const.find("Yeonwoo", "Tzuyu", 100) == g_copy_const.end());
			CHECK(g_copy_const.find("Yeonwoo", "Mina", 100) == g_copy_const.end());
		}

		SECTION("Non-const: Return correct iterator") {
			auto it = g.find("Yoona", "Taeyeon", 818);

			CHECK((*it).from == "Yoona");
			CHECK((*it).to == "Taeyeon");
			CHECK((*it).weight == 818);

			it = g.find("Yoona", "Yoona", 530);

			CHECK((*it).from == "Yoona");
			CHECK((*it).to == "Yoona");
			CHECK((*it).weight == 530);
		}

		SECTION("Const: Return correct iterator") {
			auto it = g_copy_const.find("Yoona", "Taeyeon", 818);

			CHECK((*it).from == "Yoona");
			CHECK((*it).to == "Taeyeon");
			CHECK((*it).weight == 818);

			it = g_copy_const.find("Yoona", "Yoona", 530);

			CHECK((*it).from == "Yoona");
			CHECK((*it).to == "Yoona");
			CHECK((*it).weight == 530);
		}
	}

	SECTION("connections()") {
		SECTION("Empty connections") {
			CHECK(g.connections("Taeyeon").empty());
			CHECK(g_copy_const.connections("Taeyeon").empty());
		}

		SECTION("Non-const: Check if nodes return are in order and complete") {
			g.insert_edge("Yoona", "Tzuyu", 818);
			g.insert_edge("Yoona", "Yoona", 425);
			g.insert_edge("Yoona", "Taeyeon", 309);

			CHECK(g.connections("Yoona")
			      == std::vector<std::string>{"Taeyeon", "Taeyeon", "Tzuyu", "Yoona", "Yoona"});
		}

		SECTION("Const: Check if nodes return are in order and complete") {
			g.insert_edge("Yoona", "Tzuyu", 818);
			g.insert_edge("Yoona", "Yoona", 425);
			g.insert_edge("Yoona", "Taeyeon", 309);

			auto const g_const = g;

			CHECK(g_const.connections("Yoona")
			      == std::vector<std::string>{"Taeyeon", "Taeyeon", "Tzuyu", "Yoona", "Yoona"});
		}

		SECTION("Non-const Exception: if is_node(src) is false") {
			// src not exist
			CHECK_THROWS_MATCHES(g.connections("Yeonwoo"),
			                     std::runtime_error,
			                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::connections "
			                                              "if src doesn't exist in the graph"));
		}

		SECTION("Const Exception: if is_node(src) is false") {
			// src not exist
			CHECK_THROWS_MATCHES(g_copy_const.connections("Yeonwoo"),
			                     std::runtime_error,
			                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::connections "
			                                              "if src doesn't exist in the graph"));
		}
	}
}
