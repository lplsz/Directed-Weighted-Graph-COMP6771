> Reading the following markdown document in Typora gives the best reading experience.

## Constructor

> **Rational**: The purpose of constructor is to construct the correct object with the right nodes and edges inside. So we basically construct graph using different constructor and then use accessor functions to check if the graph constructed is correct (Assuming some accessor functions are correctly implemented).
>
> In addition, since the spec requires that graph has a own copy of the nodes, we also test this property by inserting nodes stored in variable and check if the nodes is still there after the variable goes out of scope.
>
> For assignment operators, we need to ensure the two relevant objects are in the state as specified (e.g. moved from object is in empty state). We check this also using accessors functions.
>
> Below are an outline of the test cases for each constructor and assignment operator.

**All Constructor**

* The nodes are correctly initialise, check using `is_node`
* The nodes are copied constructed and have its own memory space
* Assume the correctness of
    * `is_node`
    * `is_empty`

**Copy and Move Constructor/ Assignment**

* Nodes and edges are correctly copied
* Copy from and copy to are independent
* Moved from is empty
* Assume the correctness of
    * `insert_edge`
    * `begin`
    * `replace_node`

## Modifier

> **Rational**: The purpose of modifiers are to change the state of the graph. Thus, they are not const qualified and need only to be tested on non-const objects.
>
> Since the state is change, to see if the changes are made successfully, we use accessors function to check if the correct state is achieved. See if the graph object satisfy the postcondition when provided with the precondition.
>
> In addition, we need to check if appropriate exceptions is thrown according to the spec.
>
> If all these are correct, then we can say that the function have been implemented correctly.
>
> Tests are mainly in the fashion of making changes and then use accessors to check if the changes have been made successfully.
>
> Below are an outline of all the test cases for all modifier functions.

Assume the correctness of constructors for all the test.

**insert_node**

Make sure the following behaviors are correct

* Valid nodes are inserted
* Existing nodes are rejected

Assume the correctness of

* `is_node`

**Insert Edge**

Make sure the following behaviors are correct

* Throw exception when src or dst not exist
* Allow reflexive edges
* Allow edges with the same src and dst but different weight
* Reject existing edges

* **Edges do not make duplicate of the node**

Assume the correctness of

* find

**replace_node**

Make sure the following behaviors are correct

* Throw exception when src not exist
* Replace the node: old node disappear, new node exist
* Replace the edges: All relevant edges have been replaced
* Reject if new_data exist

Assume the correctness of

* find

**merge_replace_node**

Make sure the following behaviors are correct

* Throw exception when src or dst not exist
* Replace the node: old node disappear, new node exist
* Replace the edges: All relevant edges have been replaced
* Edges are merged correctly

Assume the correctness of

* overload `<<`
* `find`

**erase_node**

Make sure the following behaviors are correct

* The node no long exist
* Removal of non-exist node returns false
* Relevant edges are removed correctly

Assume the correctness of

* `find`

**erase_edge(src, dst, weight)**

Make sure the following behaviors are correct

* Exception is thrown when either src and dst not exist
* Removal of non-exist edge returns false
* Only the matching edge is removed

**erase_edge(i)**

Make sure the following behaviors are correct

* Only the specified edge is removed
* The returned iterator is correct
    * Removing the only edge get end()
    * Remove from many edge get the iterator to the next element

**erase_edge(i, s)**

Make sure the following behaviors are correct

* Only the edge between [i, s) is removed
* The returned iterator is correct
    * Removing the only edge get end()
    * Remove from many edge get the iterator to s

Assume the correctness of

* `find`
* `begin`, `end`

**clear()**

Make sure the following behaviors are correct

* `empty()` return true after clear

Assume the correctness of

* `empty()`
* `insert_edge()`

## Accessor

> **Rational:** Accessors reflect the state of the graph. So we just simply construct the graph and make some changes and then see if the accessor give us the expected state according to the spec (Assuming the correctness of constructor and modifiers). In addition, we check if the right exception is thrown.
>
> Since accessors are only check the state, they should be able to be invoked on const object. This should be tested.
>
> Testing the above criteria should be suffiicient for accessor functions.
>
> Below are an outline of the tests written.

**is_node**: `true` if a node equivalent to `value` exists in the graph, and `false` otherwise.

**empty**: `true` if there are no nodes in the graph, and `false` otherwise.

**is_connect**: `true` if an edge `src` → `dst` exists in the graph, and `false` otherwise.

**nodes**: Check if all nodes are returned and in the correst order even if they are not in order when inserted. Check if empty list for empty graph

**weights**: Check if the weights are in the right order and empty list is return for nodes with no connection.

**find**: Check if `end()` is returned for non exist edge. Check if the content of the iterator is correct.

**connection**: Check if all connected nodes are returned in the correct order and empty list for nodes with no outgoing edge.

## Other

> **Rational**: These two functions are relatively simple in their behavior.
>
> For ==, simply check if the correct result is returned on different paris of graphs. And then negate the tests to test !=.
>
> For <<, simply check if the correct string is outputted.
>
> We can use modifiers to make changes to the graph and see if the behavior of these two operators are still correct.
>
> In addtion, since they don't change the state of the object, we should test them on both const and non-const object.

**Comparison**:

* Check that two graph with same nodes and same edges return true. Empty graph returns true, and the rest return false.
* != should be implicit in c++20, but should also be tested simply by negating the test for ==

**Extractor <<**: Check if the format of the output is correct and in the right order. The sample test on the spec. is used.

## Iterator

> **Rational**:
>
> * Iterator provide a linear abstraction so it is important that they traverse the graph in the right ordering, so we test if ++ -- get us to the edge we expect
> * Iterator acts as pointer to elements, so testing if dereferencing them give the right result is important.
> * Iterator can be compared, so we check if iterator pointing to the same edge are equal.
> * Iterator getter function like `begin` and `end` are tested to ensure they points to the logically correct edge.
>
> Tested the above criteria ensures the corretness of iterator's behavior.
>
> Since the returned `value_type` is just a copy but not the actual data stored in the graph object, so all the above functions should work on const object and hence on non-const object implicitly. 'constness' correctness also need to be tested.

**iterator()**

* Value initialised iterators are equal

**Dereference ***:

* Check if the returned value_type correspond to the current edge.
* const iterator should also be able to dereference

**Traveral ++ --**

* For pre-increment, check if the resultant iterator points to the expected edge
* For post-increment, check if the returned iterator points to the edge before incrementing, and the incremented iterator points to the next edge (Either the one before or after).

**Comparison**:

* Check if iterator obtained from `find` is equals to the result of incrementing the iterator returned by `begin()` when they are theoretically pointing to the same edge.
* Check if it returns false when pointing to different edge.
* Check if correct on const objects

**begin & end**

* Check if begin return an iterator to the first edge
* Check in end of a graph is equals to incrementing a begin iterator the number of times equals to the number of nodes in the graph
* Check if correct on const objects