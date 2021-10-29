#include "gdwg/graph.hpp"

#include <catch2/catch.hpp>
#include <iterator>
#include <set>
#include <string>

TEST_CASE("Constructor: Default") {
	auto g = gdwg::graph<int, std::string>{};

	CHECK(g.empty());
}

TEST_CASE("Constructor: Initializer list") {
	SECTION("int") {
		auto g = gdwg::graph<int, std::string>{1, 5, 0, -9, -4};

		CHECK(g.is_node(1));
		CHECK(g.is_node(5));
		CHECK(g.is_node(0));
		CHECK(g.is_node(-9));
		CHECK(g.is_node(-4));
	}

	SECTION("string") {
		auto g = gdwg::graph<std::string, std::string>{"one", "two", "three"};

		CHECK(g.is_node("one"));
		CHECK(g.is_node("two"));
		CHECK(g.is_node("three"));
	}

	SECTION("std::vector") {
		auto v1 = std::vector<int>{1, 3, 5, 7};
		auto v2 = std::vector<int>{-2, -4, -6, -8};
		auto v3 = std::vector<int>{1, 3, 5, 7};
		auto g = gdwg::graph<std::vector<int>, std::string>{v1, v2, v3};

		CHECK(g.is_node(std::vector<int>(v1)));
		CHECK(g.is_node(std::vector<int>(v2)));
		CHECK(g.is_node(std::vector<int>(v3)));
	}
}

TEST_CASE("Constructor: InputIt") {
	SECTION("int vector") {
		auto v = std::vector<int>{-2, -4, -6, -8};
		auto g = gdwg::graph<int, std::string>(v.begin(), v.end());

		CHECK(g.is_node(-2));
		CHECK(g.is_node(-4));
		CHECK(g.is_node(-6));
		CHECK(g.is_node(-8));
	}

	SECTION("string set") {
		auto s = std::set<std::string>{"Taeyeon", "Yoona", "Mina"};
		auto g = gdwg::graph<std::string, int>(s.begin(), s.end());

		CHECK(g.is_node("Taeyeon"));
		CHECK(g.is_node("Yoona"));
		CHECK(g.is_node("Mina"));
	}
}

TEST_CASE("Copy Constructor") {
	SECTION("Just nodes") {
		auto const g = gdwg::graph<std::string, int>{"Taeyeon", "Yoona", "Mina"};
		auto g_copy = g;

		// Check that both graph has the exactly same nodes
		CHECK(g.is_node("Taeyeon"));
		CHECK(g.is_node("Yoona"));
		CHECK(g.is_node("Mina"));

		CHECK(g_copy.is_node("Taeyeon"));
		CHECK(g_copy.is_node("Yoona"));
		CHECK(g_copy.is_node("Mina"));

		// Check that modifying one will not affect the other
		// Assumes the correctness of replace_node
		CHECK(g_copy.replace_node("Mina", "Nayeon"));

		CHECK(g.is_node("Mina"));
		CHECK(g_copy.is_node("Nayeon"));

		CHECK_FALSE(g.is_node("Nayeon"));
		CHECK_FALSE(g_copy.is_node("Mina"));
	}

	// Assumes insert_edge(), begin() works
	SECTION("Edges") {
		auto g = gdwg::graph<std::string, int>{"Taeyeon", "Yoona", "Mina"};
		g.insert_edge("Taeyeon", "Yoona", 1314);
		g.insert_edge("Taeyeon", "Yoona", 520);

		auto g_copy = g;

		// Check edges by iterator
		auto g_copy_it = g_copy.begin();
		CHECK((*g_copy_it).from == "Taeyeon");
		CHECK((*g_copy_it).to == "Yoona");
		CHECK((*g_copy_it).weight == 520);

		++g_copy_it;
		CHECK((*g_copy_it).from == "Taeyeon");
		CHECK((*g_copy_it).to == "Yoona");
		CHECK((*g_copy_it).weight == 1314);
	}
}

TEST_CASE("Copy Assignment") {
	SECTION("Just nodes") {
		auto const g = gdwg::graph<std::string, int>{"Taeyeon", "Yoona", "Mina"};
		auto g_copy = gdwg::graph<std::string, int>{"How", "Are", "You"};

		CHECK(g.is_node("Taeyeon"));
		CHECK(g.is_node("Yoona"));
		CHECK(g.is_node("Mina"));

		CHECK(g_copy.is_node("How"));
		CHECK(g_copy.is_node("Are"));
		CHECK(g_copy.is_node("You"));

		// Perform the copy assignment
		g_copy = g;

		CHECK(g.is_node("Taeyeon"));
		CHECK(g.is_node("Yoona"));
		CHECK(g.is_node("Mina"));

		CHECK(g_copy.is_node("Taeyeon"));
		CHECK(g_copy.is_node("Yoona"));
		CHECK(g_copy.is_node("Mina"));

		// Modify one and check if the other is changes
		// Assumes the correctness of replace_node
		CHECK(g_copy.replace_node("Mina", "Nayeon"));

		CHECK(g.is_node("Mina"));
		CHECK(g_copy.is_node("Nayeon"));

		CHECK_FALSE(g.is_node("Nayeon"));
		CHECK_FALSE(g_copy.is_node("Mina"));
	}

	// Assumes insert_edge(), begin() works
	SECTION("Edges") {
		auto g = gdwg::graph<std::string, int>{"Taeyeon", "Yoona", "Mina"};
		g.insert_edge("Taeyeon", "Yoona", 1314);
		g.insert_edge("Taeyeon", "Yoona", 520);

		auto g_copy = gdwg::graph<std::string, int>();
		g_copy = g;

		// Check edges by iterator
		auto g_copy_it = g_copy.begin();
		CHECK((*g_copy_it).from == "Taeyeon");
		CHECK((*g_copy_it).to == "Yoona");
		CHECK((*g_copy_it).weight == 520);

		++g_copy_it;
		CHECK((*g_copy_it).from == "Taeyeon");
		CHECK((*g_copy_it).to == "Yoona");
		CHECK((*g_copy_it).weight == 1314);
	}
}

// Move constructor: Iterator of other are not invalidated
TEST_CASE("Move Constructor") {
	SECTION("Just nodes") {
		auto moved_from = gdwg::graph<std::string, int>{"Taeyeon", "Yoona", "Mina"};

		SECTION("Check moved to contents && if the moved from is emtpy") {
			auto moved_to = gdwg::graph<std::string, int>(std::move(moved_from));

			// Check that both graph has the exactly same nodes
			CHECK(moved_to.is_node("Taeyeon"));
			CHECK(moved_to.is_node("Yoona"));
			CHECK(moved_to.is_node("Mina"));

			CHECK(moved_from.empty());
		}

		// Assumes insert_edge(), begin() works
		SECTION("Check that moved_from iterator works and edge is correctly moved") {
			moved_from.insert_edge("Taeyeon", "Yoona", 1314);
			auto moved_from_it = moved_from.begin();

			auto moved_to = gdwg::graph<std::string, int>(std::move(moved_from));
			CHECK((*moved_from_it).from == "Taeyeon");
			CHECK((*moved_from_it).to == "Yoona");
			CHECK((*moved_from_it).weight == 1314);
		}
	}
}

TEST_CASE("Move Assignment") {
	SECTION("Just nodes") {
		auto moved_from = gdwg::graph<std::string, int>{"Taeyeon", "Yoona", "Mina"};
		auto moved_to = gdwg::graph<std::string, int>();

		SECTION("Check moved to contents && if the moved from is emtpy") {
			moved_to = std::move(moved_from);

			// Check that both graph has the exactly same nodes
			CHECK(moved_to.is_node("Taeyeon"));
			CHECK(moved_to.is_node("Yoona"));
			CHECK(moved_to.is_node("Mina"));

			CHECK(moved_from.empty());
		}

		// Assumes insert_edge(), begin() works
		SECTION("Check that moved_from iterator works and edge is correctly moved") {
			moved_from.insert_edge("Taeyeon", "Yoona", 1314);
			auto moved_from_it = moved_from.begin();

			moved_to = std::move(moved_from);
			CHECK((*moved_from_it).from == "Taeyeon");
			CHECK((*moved_from_it).to == "Yoona");
			CHECK((*moved_from_it).weight == 1314);
		}
	}
}

TEST_CASE("Check if resources are owned") {
	SECTION("Test 1: Node") {
		auto g = gdwg::graph<std::string, int>();
		{
			auto s = std::string("Test");
			g.insert_node(s);
		}

		CHECK(g.is_node("Test"));
	}

	SECTION("Test 2: Node and Edge") {
		auto g = gdwg::graph<std::vector<int>, std::string>();
		{
			auto s = std::string("Test");
			auto v1 = std::vector<int>{1, 3, 5};
			auto v2 = std::vector<int>{2, 4, 6};

			g.insert_node(v1);
			g.insert_node(v2);
			g.insert_edge(v1, v2, s);
		}

		CHECK(g.is_node(std::vector<int>{1, 3, 5}));
		CHECK(g.is_node(std::vector<int>{2, 4, 6}));
		CHECK((*(g.begin())).weight == "Test");
	}
}