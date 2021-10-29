#ifndef GDWG_GRAPH_HPP
#define GDWG_GRAPH_HPP

#include <algorithm>
#include <cassert>
#include <experimental/iterator>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <set>
#include <sstream>
#include <tuple>
#include <utility>
#include <vector>

namespace gdwg {
	template<typename N, typename E>
	class graph {
	public:
		struct value_type {
			N from;
			N to;
			E weight;
		};

		struct edge {
			N* src;
			N* dst;
			E weight;
		};

		class iterator;

		// Constructors
		graph() noexcept = default;

		graph(std::initializer_list<N> il)
		: graph(il.begin(), il.end()) {}

		template<typename InputIt>
		graph(InputIt first, InputIt last)
		: graph() {
			std::transform(first, last, std::inserter(nodes_, nodes_.end()), [](auto const& n) {
				return std::make_shared<N>(n);
			});
		}

		// Copy Constructor
		graph(graph const& other)
		: graph() {
			std::transform(other.nodes_.begin(),
			               other.nodes_.end(),
			               std::inserter(nodes_, nodes_.end()),
			               // Make a copy
			               [&](auto const& n_ptr) { return std::make_shared<N>(*n_ptr); });

			std::transform(other.edges_.begin(),
			               other.edges_.end(),
			               std::inserter(edges_, edges_.end()),
			               // Find the ptr of the nodes created, avoid duplication
			               [&](auto const& e_ptr) {
				               struct edge new_edge = edge{(*(nodes_.find(*(e_ptr->src)))).get(),
				                                           (*(nodes_.find(*(e_ptr->dst)))).get(),
				                                           e_ptr->weight};
				               return std::make_shared<edge>(new_edge);
			               });
		}

		// Move Constructor
		graph(graph&& other) noexcept
		: nodes_{std::exchange(other.nodes_, std::set<std::shared_ptr<N>, node_comparator>())}
		, edges_{std::exchange(other.edges_, std::set<std::shared_ptr<edge>, edge_comparator>())} {}

		// Copy Assignment
		auto operator=(graph const& other) -> graph& {
			auto copy = graph(other);
			swap(*this, copy);
			return *this;
		}

		// Move Assignment
		auto operator=(graph&& other) noexcept -> graph& {
			// Check for self assignment
			if (this == std::addressof(other)) {
				return *this;
			}

			swap(*this, other);

			// Clear the moved from graph
			other.nodes_.clear();
			other.edges_.clear();

			return *this;
		}

		// Modifiers
		auto insert_node(N const& value) -> bool {
			return nodes_.emplace(std::make_shared<N>(value)).second;
		}

		auto insert_edge(N const& src, N const& dst, E const& weight) -> bool {
			if (not is_node(src) or not is_node(dst)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::insert_edge when either src "
				                         "or dst node does not exist");
			}

			struct edge new_edge = {(*(nodes_.find(src))).get(), (*(nodes_.find(dst))).get(), weight};
			return edges_.emplace(std::make_shared<edge>(new_edge)).second;
		}

		auto replace_node(N const& old_data, N const& new_data) -> bool {
			auto old_it = nodes_.find(old_data);
			if (old_it == std::end(nodes_)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::replace_node on a node that "
				                         "doesn't exist");
			}

			if (is_node(new_data)) {
				return false;
			}

			// Insert the new node
			auto new_node = std::make_shared<N>(new_data);
			nodes_.emplace(new_node);

			// Find all relevant edges
			auto edge_ptrs = std::vector<std::shared_ptr<edge>>();
			std::copy_if(edges_.begin(),
			             edges_.end(),
			             std::back_inserter(edge_ptrs),
			             [&](auto const& e_ptr) {
				             return *(e_ptr->src) == old_data or *(e_ptr->dst) == old_data;
			             });

			// Replace the nodes
			for (auto const& e_ptr : edge_ptrs) {
				auto new_src_ptr = *(e_ptr->src) == old_data ? new_node.get() : e_ptr->src;
				auto new_dst_ptr = *(e_ptr->dst) == old_data ? new_node.get() : e_ptr->dst;

				// Insert new and remove old
				edges_.emplace(std::make_shared<edge>(edge{new_src_ptr, new_dst_ptr, e_ptr->weight}));
				edges_.erase(e_ptr);
			}

			// Erase the old node
			nodes_.erase(old_it);

			return true;
		}

		auto merge_replace_node(N const& old_data, N const& new_data) -> void {
			auto old_it = nodes_.find(old_data);
			auto new_it = nodes_.find(new_data);
			if (old_it == nodes_.end() or new_it == nodes_.end()) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::merge_replace_node on old or "
				                         "new data if they don't exist in the graph");
			}

			// Find all relevant nodes
			auto edge_ptrs = std::vector<std::shared_ptr<edge>>();
			std::copy_if(edges_.begin(),
			             edges_.end(),
			             std::back_inserter(edge_ptrs),
			             [&](auto const& e_ptr) {
				             return *(e_ptr->src) == old_data or *(e_ptr->dst) == old_data;
			             });

			// Merge the nodes
			for (auto const& e_ptr : edge_ptrs) {
				auto new_src_ptr = *(e_ptr->src) == old_data ? (*new_it).get() : e_ptr->src;
				auto new_dst_ptr = *(e_ptr->dst) == old_data ? (*new_it).get() : e_ptr->dst;

				struct edge new_edge = edge{new_src_ptr, new_dst_ptr, e_ptr->weight};
				edges_.erase(e_ptr);

				// If the new edge not exist, insert it
				if (edges_.find(new_edge) == edges_.end()) {
					edges_.emplace(std::make_shared<edge>(new_edge));
				}
			}

			// Remove the old node
			nodes_.erase(old_it);
		}

		/* Remove a node and all relevant edges */
		auto erase_node(N const& value) -> bool {
			if (not is_node(value)) {
				return false;
			}

			// Remove all relevant edges
			std::erase_if(edges_,
			              [&](auto const& e) { return *(e->src) == value or *(e->dst) == value; });
			nodes_.erase(nodes_.find(value));

			return true;
		}

		/* Erase node: log(n) + e */
		auto erase_edge(N const& src, N const& dst, E const& weight) -> bool {
			if (not is_node(src) or not is_node(dst)) { // 2log(n)
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::erase_edge on src or dst if "
				                         "they don't exist in the graph");
			}

			auto count = std::erase_if(edges_, [&](auto const& e_ptr) { // e
				return *(e_ptr->src) == src and *(e_ptr->dst) == dst and e_ptr->weight == weight;
			});

			return count > 0;
		}

		/* Remove an edge pointed by i, return iterator of element after i. Constant */
		auto erase_edge(iterator i) -> iterator {
			// Check if exist
			if (i == end() or i == iterator{}) {
				return end();
			}

			return iterator{edges_.erase(i.it_)};
		}

		/* Erase [i, s) */
		auto erase_edge(iterator i, iterator s) -> iterator {
			return iterator{edges_.erase(i.it_, s.it_)};
		}

		/* Erase all nodes */
		auto clear() noexcept -> void {
			nodes_.clear();
			edges_.clear();
		}

		// Accessors
		[[nodiscard]] auto is_node(N const& value) const -> bool {
			return nodes_.find(value) != nodes_.end();
		}

		[[nodiscard]] auto empty() const -> bool {
			return nodes_.empty();
		}

		[[nodiscard]] auto is_connected(N const& src, N const& dst) const -> bool {
			if (not is_node(src) or not is_node(dst)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::is_connected if src or dst "
				                         "node don't exist in the graph");
			}

			return std::any_of(edges_.begin(), edges_.end(), [&](auto const& e_ptr) {
				return *(e_ptr->src) == src and *(e_ptr->dst) == dst;
			});
		}

		[[nodiscard]] auto nodes() const -> std::vector<N> {
			auto nodes = std::vector<N>();

			std::transform(nodes_.begin(),
			               nodes_.end(),
			               std::back_inserter(nodes),
			               [](auto const& n_ptr) { return *n_ptr; });

			return nodes;
		}

		[[nodiscard]] auto weights(N const& src, N const& dst) const -> std::vector<E> {
			if (not is_node(src) or not is_node(dst)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::weights if src or dst node "
				                         "don't exist in the graph");
			}

			// Get all weights
			auto weights = std::vector<E>();
			for (auto const& e_ptr : edges_) {
				if (*(e_ptr->src) == src and *(e_ptr->dst) == dst) {
					weights.push_back(e_ptr->weight);
				}
			}

			return weights;
		}

		// log (n) + log (e)
		[[nodiscard]] auto find(N const& src, N const& dst, E const& weight) const -> iterator {
			return iterator{edges_.find(value_type{src, dst, weight})};
		}

		// log (n) + e
		[[nodiscard]] auto connections(N const& src) const -> std::vector<N> {
			if (not is_node(src)) { // log(n)
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::connections if src doesn't "
				                         "exist in the graph");
			}

			auto dsts = std::vector<N>();
			for (auto const& e_ptr : edges_) { // e
				if (*(e_ptr->src) == src) {
					dsts.push_back(*(e_ptr->dst));
				}
			}

			return dsts;
		}

		// Iterator
		[[nodiscard]] auto begin() const -> iterator {
			return iterator{edges_.begin()};
		}

		[[nodiscard]] auto end() const -> iterator {
			return iterator{edges_.end()};
		}

		// Comparision
		[[nodiscard]] auto operator==(graph const& other) const -> bool {
			return std::equal(nodes_.begin(),
			                  nodes_.end(),
			                  other.nodes_.begin(),
			                  other.nodes_.end(),
			                  [](auto const& lhs, auto const& rhs) { return *lhs == *rhs; })
			       and std::equal(edges_.begin(),
			                      edges_.end(),
			                      other.edges_.begin(),
			                      other.edges_.end(),
			                      [](auto const& lhs, auto const& rhs) {
				                      return *(lhs->src) == *(rhs->src) and *(lhs->dst) == *(rhs->dst)
				                             and lhs->weight == rhs->weight;
			                      });
		}

	private:
		struct node_comparator {
			using is_transparent = std::true_type;

			auto operator()(std::shared_ptr<N> const& lhs, std::shared_ptr<N> const& rhs) const -> bool {
				return *lhs < *rhs;
			}

			auto operator()(std::shared_ptr<N> const& lhs, N const& rhs) const -> bool {
				return *lhs < rhs;
			}

			auto operator()(N const& lhs, std::shared_ptr<N> const& rhs) const -> bool {
				return lhs < *rhs;
			}
		};

		struct edge_comparator {
			using is_transparent = std::true_type;

			auto operator()(std::shared_ptr<edge> const& lhs, std::shared_ptr<edge> const& rhs) const
			   -> bool {
				return std::tie(*(lhs->src), *(lhs->dst), lhs->weight)
				       < std::tie(*(rhs->src), *(rhs->dst), rhs->weight);
			}

			// Compare edge ptr to edge struct
			auto operator()(std::shared_ptr<edge> const& lhs, struct edge const& rhs) const -> bool {
				return std::tie(*(lhs->src), *(lhs->dst), lhs->weight)
				       < std::tie(*(rhs.src), *(rhs.dst), rhs.weight);
			}

			auto operator()(struct edge const& lhs, std::shared_ptr<edge> const& rhs) const -> bool {
				return std::tie(*(lhs.src), *(lhs.dst), lhs.weight)
				       < std::tie(*(rhs->src), *(rhs->dst), rhs->weight);
			}

			// Compare edge ptr to value type
			auto operator()(std::shared_ptr<edge> const& lhs, struct value_type const& rhs) const
			   -> bool {
				return std::tie(*(lhs->src), *(lhs->dst), lhs->weight)
				       < std::tie(rhs.from, rhs.to, rhs.weight);
			}

			auto operator()(struct value_type const& lhs, std::shared_ptr<edge> const& rhs) const
			   -> bool {
				return std::tie(lhs.from, lhs.to, lhs.weight)
				       < std::tie(*(rhs->src), *(rhs->dst), rhs->weight);
			}
		};

		std::set<std::shared_ptr<N>, node_comparator> nodes_;
		std::set<std::shared_ptr<edge>, edge_comparator> edges_;

		/* Swap two graph */
		static auto swap(graph<N, E>& first, graph<N, E>& second) noexcept {
			std::swap(first.nodes_, second.nodes_);
			std::swap(first.edges_, second.edges_);
		}

		// Hidden Friend: Extractor
		friend auto operator<<(std::ostream& os, graph const& g) -> std::ostream& {
			auto oss = std::ostringstream{};

			std::for_each(g.nodes_.begin(), g.nodes_.end(), [&](auto const& n_ptr) {
				oss << *n_ptr << " (\n";

				for (auto const& e_ptr : g.edges_) {
					if (*(e_ptr->src) == *n_ptr) {
						oss << "  " << *(e_ptr->dst) << " | " << e_ptr->weight << "\n";
					}
				}

				oss << ")\n";
			});

			os << oss.str();

			return os;
		}

	public:
		class iterator {
		public:
			using value_type = graph<N, E>::value_type;
			using reference = value_type;
			using pointer = void;
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::bidirectional_iterator_tag;

			// Iterator constructor
			iterator() = default;

			// Iterator source
			auto operator*() const -> reference {
				return value_type{*((*it_)->src), *((*it_)->dst), (*it_)->weight};
			}

			// Iterator traversal
			auto operator++() -> iterator& {
				++it_;
				return *this;
			}

			auto operator++(int) -> iterator {
				auto old = *this;
				++(*this);
				return old;
			}

			auto operator--() -> iterator& {
				--it_;
				return *this;
			}

			auto operator--(int) -> iterator {
				auto old = *this;
				--(*this);
				return old;
			}

			// Iterator comparison
			auto operator==(iterator const& other) const -> bool {
				return it_ == other.it_;
			}

		private:
			using edges_iterator = typename std::set<std::shared_ptr<edge>, edge_comparator>::iterator;

			edges_iterator it_;

			explicit iterator(edges_iterator begin)
			: it_{begin} {}

			friend class graph<N, E>;
		};
	};
} // namespace gdwg

#endif // GDWG_GRAPH_HPP
