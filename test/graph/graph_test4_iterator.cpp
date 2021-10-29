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

TEST_CASE("Iterator") {
	SECTION("Empty Graph") {
		auto const g_empty = gdwg::graph<std::string, int>{};
		CHECK(g_empty.begin() == g_empty.end());
	}

	auto g = gdwg::graph<std::string, int>{"Yoona", "Taeyeon", "Tzuyu"};
	g.insert_edge("Yoona", "Taeyeon", 818);
	g.insert_edge("Yoona", "Yoona", 530);
	g.insert_edge("Tzuyu", "Taeyeon", 1314);

	auto const g_const = g;

	SECTION("Begin and dereference *") {
		auto const it = g_const.begin();

		CHECK((*it).from == "Tzuyu");
		CHECK((*it).to == "Taeyeon");
		CHECK((*it).weight == 1314);
	}

	SECTION("Traveral ++()") {
		// Expected output
		auto const exp = std::vector<gdwg::graph<std::string, int>::value_type>{
		   {"Tzuyu", "Taeyeon", 1314},
		   {"Yoona", "Taeyeon", 818},
		   {"Yoona", "Yoona", 530},
		};

		// Iterate through the graph
		auto count = std::size_t{0};
		for (auto it = g_const.begin(); it != g_const.end(); ++it, ++count) {
			CHECK((*it).from == exp[count].from);
			CHECK((*it).to == exp[count].to);
			CHECK((*it).weight == exp[count].weight);
		}
	}

	SECTION("Traveral ++(int)") {
		auto it = g_const.begin();

		// Post increment and check if the iterator returned is correct
		auto prev_it = it++;
		CHECK((*prev_it).from == "Tzuyu");
		CHECK((*prev_it).to == "Taeyeon");
		CHECK((*prev_it).weight == 1314);

		// Check if increment is success
		CHECK((*it).from == "Yoona");
		CHECK((*it).to == "Taeyeon");
		CHECK((*it).weight == 818);
	}

	SECTION("Traveral --()") {
		// Expected output
		auto const exp = std::vector<gdwg::graph<std::string, int>::value_type>{
		   {"Tzuyu", "Taeyeon", 1314},
		   {"Yoona", "Taeyeon", 818},
		   {"Yoona", "Yoona", 530},
		};

		// Increment to point the last element
		auto last_element_it = g_const.begin();
		++last_element_it;
		++last_element_it;

		// Iterate the graph backwardly
		auto count = std::size_t{2};
		auto it = last_element_it;
		for (; it != g_const.begin(); --it, --count) {
			CHECK((*it).from == exp[count].from);
			CHECK((*it).to == exp[count].to);
			CHECK((*it).weight == exp[count].weight);
		}

		CHECK((*it).from == exp[count].from);
		CHECK((*it).to == exp[count].to);
		CHECK((*it).weight == exp[count].weight);
	}

	SECTION("Traveral --(int)") {
		// Let the iterator points to the last element
		auto it = g_const.begin();
		++it;
		++it;

		// Post decrement and check if the iterator returned is correct
		auto last_element_it = it--;
		CHECK((*last_element_it).from == "Yoona");
		CHECK((*last_element_it).to == "Yoona");
		CHECK((*last_element_it).weight == 530);

		// Check if decrement is success
		CHECK((*it).from == "Yoona");
		CHECK((*it).to == "Taeyeon");
		CHECK((*it).weight == 818);
	}

	SECTION("Comparision") {
		SECTION("Value constructed iterator are equal") {
			CHECK(gdwg::graph<std::string, int>::iterator() == gdwg::graph<std::string, int>::iterator());
		}

		SECTION("Test if true on equal iterator") {
			auto it = g_const.begin();
			CHECK(g_const.begin() == g_const.find("Tzuyu", "Taeyeon", 1314));

			++it;
			CHECK(it == g_const.find("Yoona", "Taeyeon", 818));

			++it;
			CHECK(it == g_const.find("Yoona", "Yoona", 530));

			++it;
			CHECK(it == g_const.end());
		}

		SECTION("Test if false on not equal iterator") {
			auto it = g_const.begin();
			CHECK_FALSE(it == g_const.end());
			CHECK_FALSE(++it == g_const.end());
			CHECK_FALSE(++it == g_const.end());
			CHECK_FALSE(--it == g_const.end());
			CHECK_FALSE(--it == g_const.end());
		}
	}
}