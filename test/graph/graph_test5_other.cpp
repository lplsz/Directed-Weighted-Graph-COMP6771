#include "gdwg/graph.hpp"

#include <catch2/catch.hpp>
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

TEST_CASE("Comparison ==") {
	SECTION("Empty Graph") {
		CHECK(gdwg::graph<std::string, int>{} == gdwg::graph<std::string, int>({}));
	}

	SECTION("Just nodes") {
		// Construct two new graph, one const, one non-const
		auto const g_const = gdwg::graph<std::string, int>{"Yoona", "Mina", "Taeyeon"};
		auto g = gdwg::graph<std::string, int>{};

		CHECK_FALSE(g == g_const);

		g.insert_node("Yoona");
		g.insert_node("Mina");
		g.insert_node("Taeyeon");

		CHECK(g == g_const);
	}

	SECTION("Nodes and edges") {
		auto g = gdwg::graph<std::string, int>{"Yoona", "Tzuyu", "Taeyeon"};
		g.insert_edge("Yoona", "Taeyeon", 818);
		g.insert_edge("Yoona", "Yoona", 530);
		g.insert_edge("Tzuyu", "Taeyeon", 1314);

		auto v = std::vector<std::string>{"Taeyeon", "Tzuyu", "Yoona"};
		auto g_same = gdwg::graph<std::string, int>{v.begin(), v.end()};

		g_same.insert_edge("Yoona", "Taeyeon", 818);
		g_same.insert_edge("Yoona", "Yoona", 530);
		g_same.insert_edge("Tzuyu", "Taeyeon", 1314);

		SECTION("Check comparison with const") {
			auto const g_const = g_same;
			(void)g_const;

			CHECK(g_const == g_same);
			CHECK(g == g);
		}

		CHECK(g == g_same);

		g.erase_edge("Yoona", "Yoona", 530);
		CHECK_FALSE(g == g_same);

		g_same.erase_edge("Yoona", "Yoona", 530);
		CHECK(g == g_same);
	}
}

TEST_CASE("Comparison !=") {
	SECTION("Empty Graph") {
		CHECK_FALSE(gdwg::graph<std::string, int>{} != gdwg::graph<std::string, int>({}));
	}

	SECTION("Just nodes") {
		// Construct two new graph, one const, one non-const
		auto const g_const = gdwg::graph<std::string, int>{"Yoona", "Mina", "Taeyeon"};
		auto g = gdwg::graph<std::string, int>{};

		CHECK(g_const != g);
		CHECK(g != g_const);

		g.insert_node("Yoona");
		g.insert_node("Mina");
		g.insert_node("Taeyeon");

		CHECK_FALSE(g_const != g);
		CHECK_FALSE(g != g_const);
	}

	SECTION("Nodes and edges") {
		auto g = gdwg::graph<std::string, int>{"Yoona", "Tzuyu", "Taeyeon"};
		g.insert_edge("Yoona", "Taeyeon", 818);
		g.insert_edge("Yoona", "Yoona", 530);
		g.insert_edge("Tzuyu", "Taeyeon", 1314);

		auto v = std::vector<std::string>{"Taeyeon", "Tzuyu", "Yoona"};
		auto g_same = gdwg::graph<std::string, int>{v.begin(), v.end()};

		g_same.insert_edge("Yoona", "Taeyeon", 818);
		g_same.insert_edge("Yoona", "Yoona", 530);
		g_same.insert_edge("Tzuyu", "Taeyeon", 1314);

		SECTION("Check comparison with const") {
			auto const g_const = g_same;
			(void)g_const;

			CHECK_FALSE(g_const != g_same);
			CHECK_FALSE(g != g);
		}

		CHECK_FALSE(g != g_same);

		g.erase_edge("Yoona", "Yoona", 530);
		CHECK(g != g_same);

		g_same.erase_edge("Yoona", "Yoona", 530);
		CHECK_FALSE(g != g_same);
	}
}

TEST_CASE("Extractor <<") {
	SECTION("Empty graph") {
		auto const g = gdwg::graph<int, int>{};

		auto out = std::ostringstream{};
		out << g;
		CHECK(out.str().empty());
	}

	SECTION("Just nodes") {
		auto g = gdwg::graph<std::string, int>{"Yoona", "Tzuyu", "Taeyeon"};

		auto out = std::ostringstream{};
		out << g;
		auto const expected_output = std::string_view(R"(Taeyeon (
)
Tzuyu (
)
Yoona (
)
)");

		CHECK(out.str() == expected_output);

		auto const g_const = g;
		(void)g_const;
		out.str("");
		out << g_const;
		CHECK(out.str() == expected_output);
	}

	SECTION("Sample test with edge insertion order changed") {
		auto const v = std::vector<gdwg::graph<int, int>::value_type>{
		   {4, 1, -4},
		   {3, 6, -8},
		   {3, 2, 2},
		   {2, 4, 2},
		   {2, 1, 1},
		   {6, 3, 10},
		   {6, 2, 5},
		   {1, 5, -1},
		   {4, 5, 3},
		   {5, 2, 7},
		};

		auto g = gdwg::graph<int, int>{1, 2, 3, 4, 5, 6, 64};
		for (const auto& x : v) {
			g.insert_edge(x.from, x.to, x.weight);
		};

		g.insert_node(64);
		auto out = std::ostringstream{};
		out << g;
		auto const expected_output = std::string_view(R"(1 (
  5 | -1
)
2 (
  1 | 1
  4 | 2
)
3 (
  2 | 2
  6 | -8
)
4 (
  1 | -4
  5 | 3
)
5 (
  2 | 7
)
6 (
  2 | 5
  3 | 10
)
64 (
)
)");
		CHECK(out.str() == expected_output);

		// Also check for const
		auto const g_const = g;
		out.str("");
		out << g_const;
		CHECK(out.str() == expected_output);
	}
}