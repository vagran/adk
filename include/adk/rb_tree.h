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
 * Node are always referenced using pointer semantic (@a NodePtr parameterized
 * type). User code can create any class which satisfies pointer semantic
 * requirements (or just use plain pointer to node class):
 * @li Type cast to @a bool type should indicate pointer NULL/NON-NULL value.
 * @li Comparison (== and !=) operators should be implemented.
 * @li Construction and assignment by copy and moving should be implemented.
 * @li -> operator should be implemented to access node class.
 *
 * @a NodeCmp class is a comparator for nodes. Function call semantic is used
 * its invocation, using the following prototype:
 * @code
 * int
 * Compare(const NodePtr node1, const NodePtr node2) [const];
 * @endcode
 * It should return positive value if this @a node1 is greater than @a node2,
 * negative value if @a node1 is less than @a node2, zero if both nodes are
 * equal.
 */
template <class NodePtr, class NodeCmp>
class RBTree {
public:
    /** Tree node direction relatively to its parent node. */
    enum DirValue {
        /** Left node. */
        DIR_LEFT,
        /** Right node. */
        DIR_RIGHT
    };

    /** Helper class for direction value manipulations. */
    class Dir {
    public:
        DirValue value;

        Dir(): value(DIR_LEFT) {}

        Dir(DirValue value): value(value) {}

        operator DirValue()
        {
            return value;
        }

        /** Return opposite direction. */
        DirValue
        operator !()
        {
            return static_cast<DirValue>(!value);
        }

        Dir &
        operator =(DirValue value)
        {
            this->value = value;
            return *this;
        }
    };

private:
    /** Rotate subtree around the specified node in specified direction.
     *
     * @param root Root node.
     * @param node Node to rotate around.
     * @param dir Rotation direction. If @ref DIR_LEFT then right child of the
     *      node will become its parent, left if @ref DIR_RIGHT.
     */
    static void
    _Rotate(NodePtr &root, NodePtr node, Dir dir)
    {
        ASSERT(dir == DIR_LEFT || dir == DIR_RIGHT);
        NodePtr x = node->GetChild(dir);
        ASSERT(x);
        x->SetParent(node->GetParent());
        if (node->GetParent()) {
            if (node->GetParent()->GetChild(DIR_LEFT) == node) {
                node->GetParent()->SetChild(DIR_LEFT, x);
            } else {
                ASSERT(node->GetParent()->GetChild(DIR_RIGHT) == node);
                node->GetParent()->SetChild(DIR_RIGHT, x);
            }
        } else {
            ASSERT(root == node);
            root = x;
        }

        node->SetChild(dir, x->GetChild(!dir));
        if (node->GetChild(dir)) {
            node->GetChild(dir)->SetParent(node);
        }

        x->SetChild(!dir, node);
        node->SetParent(x);
    }

    /** Re-balance the tree after insertion. This function can be called
     * recursively. It must be called only if there is RB balancing rules
     * violations. In particular only one violation must be present upon this
     * function calling - provided node is red and its parent is also red.
     *
     * @param root Root node.
     * @param node Inserted or lastly balanced node. Its parent should not be
     *      root and should be red.
     */
    static void
    _RebalanceInsertion(NodePtr &root, NodePtr node)
    {
        /* Validate entrance conditions. */
        ASSERT(node->IsRed());
        ASSERT(node->GetParent());
        ASSERT(node->GetParent()->IsRed());
        ASSERT(node->GetParent()->GetParent());

        /* We have two symmetric scenarios - new node is either in left or right
         * subtree of its grandparent.
         */
        ASSERT(node->GetParent()->GetChild(DIR_LEFT) == node ||
               node->GetParent()->GetChild(DIR_RIGHT) == node);

        ASSERT(node->GetParent()->GetParent()->GetChild(DIR_LEFT) == node->GetParent() ||
               node->GetParent()->GetParent()->GetChild(DIR_RIGHT) == node->GetParent());

        Dir dir =
            node->GetParent()->GetParent()->GetChild(DIR_RIGHT) == node->GetParent() ?
                DIR_RIGHT : DIR_LEFT;

        /* Node uncle. */
        NodePtr y = node->GetParent()->GetParent()->GetChild(!dir);
        if (y && y->IsRed()) {
            /* Case 1: uncle is red - just re-color nodes. */
            y->SetColor(false);
            node->GetParent()->SetColor(false);
            node->GetParent()->GetParent()->SetColor(true);
            /* Grandparent became red so re-balancing could be required again. */
            _CheckRebalanceInsertion(root, node->GetParent()->GetParent());
        } else {
            Dir nodeDir = node->GetParent()->GetChild(DIR_RIGHT) == node ?
                DIR_RIGHT : DIR_LEFT;
            if (nodeDir == dir) {
                y = node->GetParent();
            } else {
                /* Case 3 - rotate parent and transform to case 2. */
                _Rotate(root, node->GetParent(), !dir);
                y = node;
            }
            /* Case 2 - perform grandparent rotation. */
            NodePtr x = y->GetParent();

            x->SetColor(true);
            y->SetColor(false);

            _Rotate(root, x, dir);
        }
    }

    /** Check if re-balancing after insertion is required for the provided node.
     * Call @ref _RebalanceInsertion method if required.
     *
     * @param root Root node.
     * @param node Node to check. Node should be red.
     */
    static void
    _CheckRebalanceInsertion(NodePtr &root, NodePtr node)
    {
        ASSERT(node->IsRed());
        if (node->GetParent() && node->GetParent()->IsRed() &&
            node->GetParent()->GetParent()) {

            _RebalanceInsertion(root, node);
        }
    }

    /** Re-balance the tree after deletion and detach replacement entry.
     *
     * @param root Root node.
     * @param node Replacement node which must be detached.
     */
    static void
    _RebalanceDeletion(NodePtr &root, NodePtr node)
    {
        NodePtr replNode = node, tmpNode;

        if (!node->GetParent()) {
            ASSERT(node == root);
            root = nullptr;
            return;
        }

        do {
            /* Current is red leaf. */
            if (node->IsRed() && !node->GetChild(DIR_LEFT) && !node->GetChild(DIR_RIGHT)) {
                /* Done */
                break;
            }

            Dir nodeDir;
            if (node->GetParent()->GetChild(DIR_LEFT) == node) {
                nodeDir = DIR_LEFT;
            } else {
                ASSERT(node->GetParent()->GetChild(DIR_RIGHT) == node);
                nodeDir = DIR_RIGHT;
            }

            /* Current is black with one red child. Detach current node and make its
             * child node black to keep the tree balanced.
             */
            if (!node->IsRed() &&
                (((tmpNode = node->GetChild(DIR_LEFT)) &&
                  tmpNode->IsRed() && !node->GetChild(DIR_RIGHT)) ||

                 ((tmpNode = node->GetChild(DIR_RIGHT)) &&
                  tmpNode->IsRed() && !node->GetChild(DIR_LEFT)))) {

                tmpNode->SetParent(node->GetParent());
                node->GetParent()->SetChild(nodeDir, tmpNode);
                tmpNode->SetColor(false);
                /* Node detached, all done. */
                return;
            }

            do {
                NodePtr siblNode = node->GetParent()->GetChild(!nodeDir);
                /* Current sibling is red. */
                if (siblNode->IsRed()) {
                    /* Exchange colors of parent and sibling nodes. */
                    node->GetParent()->SetColor(true);
                    siblNode->SetColor(false);

                    /* Rotate around the parent. */
                    _Rotate(root, node->GetParent(), !nodeDir);
                    continue;
                }

                /* Current sibling is black with two black children */
                if ((!siblNode->GetChild(DIR_LEFT) ||
                     !siblNode->GetChild(DIR_LEFT)->IsRed()) &&

                    (!siblNode->GetChild(DIR_RIGHT) ||
                     !siblNode->GetChild(DIR_RIGHT)->IsRed())) {

                    /* Make sibling red. */
                    siblNode->SetColor(true);
                    /* Make parent new current node. */
                    node = node->GetParent();
                    if (!node->IsRed() && node->GetParent()) {
                        if (node->GetParent()->GetChild(DIR_LEFT) == node) {
                            nodeDir = DIR_LEFT;
                        } else {
                            ASSERT(node->GetParent()->GetChild(DIR_RIGHT) == node);
                            nodeDir = DIR_RIGHT;
                        }
                        continue;
                    }
                    /* Current is red - make it black and we are done. */
                    node->SetColor(false);
                    break;
                }

                /* Current sibling is black with one or two red children (can be
                 * one red and one black).
                 */
                NodePtr farNephewNode = siblNode->GetChild(!nodeDir);

                if (!farNephewNode || !farNephewNode->IsRed()) {
                    /* Far nephew is black (null leaf counted as black also),
                     * rotate around the sibling.
                     */
                    _Rotate(root, siblNode, nodeDir);
                    siblNode = node->GetParent()->GetChild(!nodeDir);
                    farNephewNode = siblNode->GetChild(!nodeDir);
                }

                /* Color the far nephew black, make the sibling color the same
                 * as the color of its parent, color the parent black.
                 */
                farNephewNode->SetColor(false);
                siblNode->SetColor(node->GetParent()->IsRed());
                node->GetParent()->SetColor(false);

                /* Rotate around the parent. */
                _Rotate(root, node->parent, !nodeDir);
                break;
            } while (node);
        } while(false);

        /* Detach replacement node. */
        if (replNode->GetParent()->GetChild(DIR_LEFT) == replNode) {
            replNode->GetParent()->SetChild(DIR_LEFT, nullptr);
        } else {
            ASSERT(replNode->GetParent()->GetChild(DIR_RIGHT) == replNode);
            replNode->GetParent()->SetChild(DIR_RIGHT, nullptr);
        }
    }


public:

    /** Delete node from the tree.
     * @param root Root node.
     * @param node Node to delete.
     */
    static void
    DeleteNode(NodePtr &root, NodePtr node)
    {
        ASSERT(node->IsWired());
        /* Firstly find successor or predecessor of the provided. It will be
         * detached from the tree instead of the provided node and after that it
         * will replace the target node.
         */
        NodePtr targetNode = node;
        NodePtr replNode = node; /* Replacement node. */
        Dir dir; /* Initial direction. */
        if (replNode->GetChild(DIR_LEFT) || replNode->GetChild(DIR_RIGHT)) {
            /* If there are children select direction. If there is a right item
             * which is red or there are no left item, then find predecessor.
             * Otherwise find successor.
             */
            if ((replNode->GetChild(DIR_LEFT) &&
                 replNode->GetChild(DIR_LEFT)->IsRed()) ||
                !replNode->GetChild(DIR_RIGHT)) {

                dir = DIR_LEFT;
            } else {
                dir = DIR_RIGHT;
            }
            replNode = replNode->GetChild(dir);
            while (replNode->GetChild(!dir)) {
                replNode = replNode->GetChild(!dir);
            }
        }

        /* Re-balance the tree and detach replacement entry. */
        _RebalanceDeletion(root, replNode);

        /* Replace target entry with detached replacement entry. */
        if (replNode == targetNode) {
            /* The replacement entry which is also target entry was already
             * detached by the previous call, so we are done.
             */
            targetNode->SetWired(false);
            return;
        }
        /* Move all links and color from target entry to the replacement one. */
        replNode->SetColor(targetNode->IsRed());
        targetNode->SetWired(false);
        replNode->SetParent(targetNode->GetParent());
        if (targetNode->GetParent()) {
            if (targetNode->GetParent()->GetChild(DIR_LEFT) == targetNode) {
                targetNode->GetParent()->SetChild(DIR_LEFT, replNode);
            } else {
                ASSERT(targetNode->GetParent()->GetChild(DIR_RIGHT) == targetNode);
                targetNode->GetParent()->SetChild(DIR_RIGHT, replNode);
            }
        } else {
            ASSERT(targetNode == root);
            root = replNode;
            root->SetColor(false);
        }
        replNode->SetChild(DIR_LEFT, targetNode->GetChild(DIR_LEFT));
        if (replNode->GetChild(DIR_LEFT)) {
            ASSERT(replNode->GetChild(DIR_LEFT)->GetParent() == targetNode);
            replNode->GetChild(DIR_LEFT)->SetParent(replNode);
        }
        replNode->SetChild(DIR_RIGHT, targetNode->GetChild(DIR_RIGHT));
        if (replNode->GetChild(DIR_RIGHT)) {
            ASSERT(replNode->GetChild(DIR_RIGHT)->GetParent() == targetNode);
            replNode->GetChild(DIR_RIGHT)->SetParent(replNode);
        }
    }

    /** Insert node to the tree.
     * @param root Root node.
     * @param node Node to insert.
     * @param comparator Nodes comparator.
     * @return Either @a node if it was inserted or existing node with the
     *      same key (@a node is not inserted in the tree in such case).
     */
    static NodePtr
    InsertNode(NodePtr &root, NodePtr node, const NodeCmp &comparator)
    {
        ASSERT(!node->IsWired());

        node->SetChild(DIR_LEFT, nullptr);
        node->SetChild(DIR_RIGHT, nullptr);

        /* Special case - empty tree, insert root. */
        if (UNLIKELY(!root)) {
            root = node;
            node->SetParent(nullptr);
            node->SetColor(false);
            node->SetWired(true);
            return node;
        }

        /* Firstly search for insertion point and insert the node. */
        NodePtr parent = root;
        while (true) {
            int cmp = comparator(node, parent);
            if (!cmp) {
                /* The same node found, do not insert the new one. */
                return parent;
            }
            Dir dir = cmp > 0 ? DIR_RIGHT : DIR_LEFT;
            if (parent->GetChild(dir)) {
                parent = parent->GetChild(dir);
            } else {
                parent->SetChild(dir, node);
                node->SetParent(parent);
                node->SetColor(true);
                node->SetWired(true);
                break;
            }
        }

        /* Re-balance the tree if necessary. */
        if (node->GetParent()->IsRed()) {
            _RebalanceInsertion(root, node);
        }

        /* Set root black if it was re-colored during re-balancing. */
        root->SetColor(false);
        return node;
    }

    /** Get next tree node during the tree traversal.
     *
     * @param root Root node.
     * @param node Previously visited node. Can be NULL to get the first node.
     * @return Next node. NULL if all nodes traversed.
     */
    static NodePtr
    GetNextNode(const NodePtr &root, NodePtr node = nullptr)
    {
        if (!node) {
            return root;
        }
        if (node->GetChild(DIR_LEFT)) {
            return node->GetChild(DIR_LEFT);
        }
        if (node->GetChild(DIR_RIGHT)) {
            return node->GetChild(DIR_RIGHT);
        }

        while (node->GetParent()) {
            if (node->GetParent()->GetChild(DIR_LEFT) == node &&
                node->GetParent()->GetChild(DIR_RIGHT)) {

                return node->GetParent()->GetChild(DIR_RIGHT);
            }
            node = node->GetParent();
        }
        return nullptr;
    }

    /** Validate the tree. This method is intended for tree implementation
     * troubleshooting and normally is not required to be used.
     *
     * @param root Root node.
     * @param comparator Nodes comparator.
     * @return @a true if the tree is valid red-black tree, @a false if there
     *      are some rules violations or dis-integrity.
     */
    static bool
    Validate(const NodePtr &root, const NodeCmp &comparator)
    {
        /* Iterate all nodes and check balancing rules validity for each node. */
        NodePtr node = nullptr;
        int numBlackNodes = -1; /* Black nodes amount in a simple path. */
        while ((node = GetNextNode(node))) {

            /* Verify link with parent. */
            if (node->GetParent()) {
                if (node->GetParent()->GetChild(DIR_LEFT) != node &&
                    node->GetParent()->GetChild(DIR_RIGHT) != node) {

                    /* Parent link broken. */
                    return false;
                }
            }

            /* Validate children. */
            if (node->GetChild(DIR_LEFT) &&
                comparator(node->GetChild(DIR_LEFT), node) >= 0) {

                return false;
            }
            if (node->GetChild(DIR_RIGHT) &&
                comparator(node->GetChild(DIR_RIGHT), node) <= 0) {

                return false;
            }

            /* Red node never can have red children. */
            if (node->IsRed() && node->GetParent() && node->GetParent()->IsRed()) {
                return false;
            }

            /* If this is a leaf node, check black nodes amount in ascendant path. */
            if (!node->GetChild(DIR_LEFT) || !node->GetChild(DIR_RIGHT)) {
                int n = 0;
                NodePtr next = node;
                do {
                    if (!next->IsRed()) {
                        n++;
                    }
                    next = next->GetParent();
                } while (next);
                if (numBlackNodes != -1) {
                    if (numBlackNodes != n) {
                        return false;
                    }
                } else {
                    numBlackNodes = n;
                }
            }
        }
        return true;
    }
};

} /* namespace adk */

#endif /* RB_TREE_H_ */
