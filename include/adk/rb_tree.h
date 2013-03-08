/* /ADK/include/adk/rb_tree.h
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2013, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file rb_tree.h
 * Generic red-black trees algorithms implementation which can be re-used
 * by custom tree structure implementations.
 */

#ifndef RB_TREE_H_
#define RB_TREE_H_

namespace adk {

/** All RB-trees algorithms are implemented inside this class. Since they are
 * parameterized and provided completely in this header file, the client code
 * should instantiate the implementation in one source file.
 *
 * Node type must be provided by the custom tree implementation. It should have
 * the following methods:
 *
 * Return @a true if the node is red, @a false if it is black one.
 * @code
 * bool
 * IsRed() const;
 * @endcode
 *
 * Return @a true if the node is currently inserted in the tree, @a false if it
 * is detached.
 * @code
 * bool
 * IsWired() const;
 * @endcode
 *
 * Set node color.
 * @code
 * void
 * SetColor(bool isRed);
 * @endcode
 *
 * Set wired status for the node.
 * @code
 * void
 * SetWired(bool isWired);
 * @endcode
 *
 * Get child node of the specified direction.
 * @code
 * NodePtr
 * GetChild(Dir dir) const;
 * @endcode
 *
 * Set child node for the specified direction.
 * @code
 * void
 * SetChild(Dir dir, NodePtr node);
 * @endcode
 *
 * Get parent node.
 * @code
 * NodePtr
 * GetParent() const;
 * @endcode
 *
 * Set parent node.
 * @code
 * void
 * SetParent(NodePtr node);
 * @endcode
 *
 * Compare with other node.
 * Return ositive value if this node greater than @a node, negative value if it
 * is less than @a node, zero if it is equal to @a node.
 * @code
 * int
 * Compare(const NodePtr node) const;
 * @endcode
 *
 * Node are always referenced using pointer semantic (@a NodePtr parameterized
 * type). User code can create any class which satisfies pointer semantic
 * requirements (or just use plain pointer to node class):
 * @li Type cast to @a bool type should indicate pointer NULL/NON-NULL value.
 * @li Comparison (== and !=) operators should be implemented.
 * @li Construction and assignment by copy and moving should be implemented.
 * @li -> operator should be implemented to access node class.
 */
template <class NodePtr>
class RBTree {
private:


public:

    /** Tree node direction relatively to its parent node. */
    enum Dir {
        /** Left node. */
        DIR_LEFT,
        /** Right node. */
        DIR_RIGHT
    };

    /** Delete node from the tree.
     * @param root Root node.
     * @param node Node to delete.
     */
    static void
    DeleteNode(NodePtr &root, NodePtr node)
    {

    }

    /** Insert node to the tree.
     * @param root Root node.
     * @param node Node to insert.
     */
    static void
    InsertNode(NodePtr &root, NodePtr node)
    {

    }

};

} /* namespace adk */

#endif /* RB_TREE_H_ */
