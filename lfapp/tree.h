/*
 * $Id$
 * Copyright (C) 2009 Lucid Fusion Labs

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __LFL_LFAPP_TREE_H__
#define __LFL_LFAPP_TREE_H__
namespace LFL {

struct RedBlackTreeZipper {
    RedBlackTreeZipper(bool update=0) {} 
    string DebugString() const { return ""; }
};

template <class K, class V, class Zipper = RedBlackTreeZipper> struct RedBlackTreeNode {
    enum { Left, Right };
    K key;
    unsigned val:31, left, right, parent, color:1;
    RedBlackTreeNode(K k, unsigned v, unsigned P, bool C=0) : key(k), val(v), left(0), right(0), parent(P), color(C) {}

    void SwapKV(RedBlackTreeNode *n) { swap(key, n->key); unsigned v=val; val=n->val; n->val=v; }
    K GetKey(Zipper*) const { return key; }
    int WalkLeft   (Zipper *z) const { return left; }
    int WalkRight  (Zipper *z) const { return right; }
    int UnwalkRight(Zipper *z) const { return parent; }
    bool LessThan(const K &k, int, Zipper*) const { return key < k; }
    bool MoreThan(const K &k, int, Zipper*) const { return k < key; }
    void ComputeStateFromChildren(const RedBlackTreeNode*, const RedBlackTreeNode*) {}
};

template <class K, class V, class Zipper = RedBlackTreeZipper, class Node = RedBlackTreeNode<K,V,Zipper>,
          template <class T> class Storage = FreeListVector>
struct RedBlackTree {
    enum Color { Red, Black };
    struct Iterator { 
        RedBlackTree *tree; int ind; K key; V *val; Zipper zipper;
        Iterator(RedBlackTree *T=0, int I=0)        : tree(T), ind(I), key(), val(0) {                  if (ind) LoadKV(); }
        Iterator(RedBlackTree *T, int I, Zipper &z) : tree(T), ind(I), key(), val(0) { swap(z, zipper); if (ind) LoadKV(); }
        Iterator& operator--() { if (!ind) *this = tree->RBegin(); else if ((ind = tree->DecrementNode(ind, &zipper))) LoadKV(); else val = 0; return *this; }
        Iterator& operator++() { CHECK(ind);                            if ((ind = tree->IncrementNode(ind, &zipper))) LoadKV(); else val = 0; return *this; }
        bool operator!=(const Iterator &i) { return tree != i.tree || ind != i.ind; }
        void LoadKV() { if (const Node *n = &tree->node[ind-1]) { key = n->GetKey(&zipper); val = &tree->val[n->val]; } }
    };
    struct ConstIterator { 
        const RedBlackTree *tree; int ind; K key; const V *val; Zipper zipper;
        ConstIterator(const RedBlackTree *T=0, int I=0)        : tree(T), ind(I), key(), val(0) {                  if (ind) LoadKV(); }
        ConstIterator(const RedBlackTree *T, int I, Zipper &z) : tree(T), ind(I), key(), val(0) { swap(z, zipper); if (ind) LoadKV(); }
        ConstIterator(const Iterator &i) : tree(i.tree), ind(i.ind), key(i.key), val(i.val), zipper(i.zipper) {}
        ConstIterator& operator-- () { if (!ind) *this = tree->RBegin(); else if ((ind = tree->DecrementNode(ind, &zipper))) LoadKV(); else val = 0; return *this; }
        ConstIterator& operator++ () { CHECK(ind);                            if ((ind = tree->IncrementNode(ind, &zipper))) LoadKV(); else val = 0; return *this; }
        bool operator!=(const ConstIterator &i) { return tree != i.tree || ind != i.ind; }
        void LoadKV() { if (const Node *n = &tree->node[ind-1]) { key = n->GetKey(&zipper); val = &tree->val[n->val]; } }
    };
    struct Query {
        const K key; const V *val; Zipper z;
        Query(const K &k, const V *v=0, bool update=0) : key(k), val(v), z(update) {}
    };

    Storage<Node> node;
    Storage<V>    val;
    int head=0, count=0;

    int size() const { return count; }
    void Clear() { head=count=0; node.Clear(); val.Clear(); }
    /**/ Iterator  Begin()       { Zipper z; int n = head ? GetMinNode(head)     : 0; return      Iterator(this, n, z); }
    ConstIterator  Begin() const { Zipper z; int n = head ? GetMinNode(head)     : 0; return ConstIterator(this, n, z); }
    /**/ Iterator RBegin()       { Zipper z; int n = head ? GetMaxNode(head, &z) : 0; return      Iterator(this, n, z); }
    ConstIterator RBegin() const { Zipper z; int n = head ? GetMaxNode(head, &z) : 0; return ConstIterator(this, n, z); }

    bool          Erase (const K &k)             { Query q(k, 0,  1); return EraseNode(&q); }
    /**/ Iterator Insert(const K &k, const V &v) { Query q(k, &v, 1); int n=InsertNode    (&q); return Iterator     (this, n, q.z); }
    ConstIterator Find       (const K &k) const  { Query q(k);        int n=FindNode      (&q); return ConstIterator(this, n, q.z); }
    /**/ Iterator Find       (const K &k)        { Query q(k);        int n=FindNode      (&q); return Iterator     (this, n, q.z); }
    ConstIterator LowerBound (const K &k) const  { Query q(k);        int n=LowerBoundNode(&q); return ConstIterator(this, n, q.z); }
    /**/ Iterator LowerBound (const K &k)        { Query q(k);        int n=LowerBoundNode(&q); return Iterator     (this, n, q.z); }
    ConstIterator LesserBound(const K &k) const  { Query q(k);        int n=LowerBoundNode(&q); ConstIterator i(this, n, q.z); if (i.val && k < i.key) --i; return i; }
    /**/ Iterator LesserBound(const K &k)        { Query q(k);        int n=LowerBoundNode(&q); Iterator      i(this, n, q.z); if (i.val && k < i.key) --i; return i; }

    virtual K GetCreateNodeKey(const Query *q) const { return q->key; }
    virtual void ComputeStateFromChildrenOnPath(Query *q) {}
    virtual int ResolveInsertCollision(int ind, Query *q) { 
        Node *n = &node[ind-1];
        if (bool update_on_dup_insert = 1) val[n->val] = *q->val;
        return ind;
    }

    int FindNode(Query *q) const {
        const Node *n;
        for (int ind=head; ind;) {
            n = &node[ind-1];
            if      (n->MoreThan(q->key, ind, &q->z)) ind = n->WalkLeft (&q->z);
            else if (n->LessThan(q->key, ind, &q->z)) ind = n->WalkRight(&q->z);
            else return ind;
        } return 0;
    }
    int LowerBoundNode(Query *q) const {
        const Node *n = 0;
        int ind = head;
        while (ind) {
            n = &node[ind-1];
            if      (n->MoreThan(q->key, ind, &q->z)) { if (!n->left) break; ind = n->WalkLeft(&q->z); }
            else if (n->LessThan(q->key, ind, &q->z)) {
                if (!n->right) { ind = IncrementNode(ind, &q->z); break; }
                ind = n->WalkRight(&q->z);
            } else break;
        }
        return ind;
    }
    int IncrementNode(int ind, Zipper *z=0) const {
        const Node *n = &node[ind-1], *p;
        if (n->right) for (n = &node[(ind = n->WalkRight(z))-1]; n->left; n = &node[(ind = n->left)-1]) {}
        else {
            int p_ind = GetParent(ind);
            while (p_ind && ind == (p = &node[p_ind-1])->right) { ind=p_ind; p_ind=p->UnwalkRight(z); }
            ind = p_ind;
        }
        return ind;
    }
    int DecrementNode(int ind, Zipper *z=0) const {
        const Node *n = &node[ind-1], *p;
        if (n->left) for (n = &node[(ind = n->left)-1]; n->right; n = &node[(ind = n->WalkRight(z))-1]) {}
        else {
            int p_ind = GetParent(ind);
            while (p_ind && ind == (p = &node[p_ind-1])->left) { ind=p_ind; p_ind=p->parent; }
            if ((ind = p_ind)) p->UnwalkRight(z); 
        }
        return ind;
    }
 
    int InsertNode(Query *q) {
        Node *new_node;
        int new_ind = node.Insert(Node(GetCreateNodeKey(q), 0, 0))+1;
        if (int ind = head) {
            for (;;) {
                Node *n = &node[ind-1];
                if      (n->MoreThan(q->key, ind, &q->z)) { if (!n->left)  { n->left  = new_ind; break; } ind = n->WalkLeft (&q->z); }
                else if (n->LessThan(q->key, ind, &q->z)) { if (!n->right) { n->right = new_ind; break; } ind = n->WalkRight(&q->z); }
                else { node.Erase(new_ind-1); return ResolveInsertCollision(ind, q); }
            }
            (new_node = &node[new_ind-1])->parent = ind;
        } else new_node = &node[(head = new_ind)-1];
        new_node->val = val.Insert(*q->val);
        ComputeStateFromChildrenOnPath(q);
        InsertBalance(new_ind);
        count++;
        return new_ind;
    }
    void InsertBalance(int ind) {
        int p_ind, gp_ind, u_ind;
        Node *n = &node[ind-1], *p, *gp, *u;
        if (!(p_ind = n->parent)) { n->color = Black; return; }
        if ((p = &node[p_ind-1])->color == Black || !(gp_ind = p->parent)) return;
        gp = &node[gp_ind-1];
        if ((u_ind = GetSibling(gp, p_ind)) && (u = &node[u_ind-1])->color == Red) {
            p->color = u->color = Black;
            gp->color = Red;
            return InsertBalance(gp_ind);
        }
        do {
            if      (ind == p->right && p_ind == gp->left)  { RotateLeft (p_ind); ind = node[ind-1].left; }
            else if (ind == p->left  && p_ind == gp->right) { RotateRight(p_ind); ind = node[ind-1].right; }
            else break;
            p  = &node[(p_ind  = GetParent(ind))-1];
            gp = &node[(gp_ind = p->parent)     -1];
        } while (0);
        p->color = Black;
        gp->color = Red;
        if (ind == p->left && p_ind == gp->left) RotateRight(gp_ind);
        else                                     RotateLeft (gp_ind);
    }

    bool EraseNode(Query *q) {
        Node *n;
        int ind = head;
        while (ind) {
            n = &node[ind-1];
            if      (n->MoreThan(q->key, ind, &q->z)) ind = n->WalkLeft (&q->z);
            else if (n->LessThan(q->key, ind, &q->z)) ind = n->WalkRight(&q->z);
            else break;
        }
        if (!ind) return false;
        int orig_ind = ind, max_path = 0;
        val.Erase(n->val);
        if (n->left && n->right) {
            n->SwapKV(&node[(ind = GetMaxNode((max_path = n->left)))-1]);
            n = &node[ind-1];
        }
        bool balance = n->color == Black;
        int child = X_or_Y(n->left, n->right), parent = n->parent;
        ReplaceNode(ind, child);
        node.Erase(ind-1);
        if (child == head && head) node[head-1].color = Black;
        if (max_path) ComputeStateFromChildrenOnMaxPath(max_path);
        if (max_path) ComputeStateFromChildren(&node[orig_ind-1]);
        ComputeStateFromChildrenOnPath(q);
        if (balance) EraseBalance(child, parent);
        count--;
        return true;
    }
    void EraseBalance(int ind, int p_ind) {
        if (!p_ind) return;
        Node *p = &node[p_ind-1];
        int s_ind = GetSibling(p, ind);
        if (!s_ind) return;
        Node *s = &node[s_ind-1];
        if (s->color == Red) {
            p->color = Red;
            s->color = Black;
            if (ind == p->left) RotateLeft (p_ind);
            else                RotateRight(p_ind);
            s = &node[(s_ind = GetSibling(p, ind))-1];
        }
        if (s->color == Black) {
            bool slr = GetColor(s->left)  == Red, slb = !slr, lc = ind == p->left;
            bool srr = GetColor(s->right) == Red, srb = !srr, rc = ind == p->right;
            if (p->color == Black && slb && srb) { s->color = Red; return EraseBalance(p_ind, GetParent(p_ind)); }
            if (p->color == Red   && slb && srb) { s->color = Red; p->color = Black; return; }
            if      (lc && slr && srb) { s->color = Red; SetColor(s->left , Black); RotateRight(s_ind); }
            else if (rc && slb && srr) { s->color = Red; SetColor(s->right, Black); RotateLeft (s_ind); }
            s = &node[(s_ind = GetSibling(p, ind))-1];
        }
        s->color = p->color;
        p->color = Black;
        if (ind == p->left) { SetColor(s->right, Black); RotateLeft (p_ind); }
        else                { SetColor(s->left,  Black); RotateRight(p_ind); }
    }

    void ReplaceNode(int ind, int new_ind) {
        Node *n = &node[ind-1];
        if (!n->parent) head = new_ind;
        else if (Node *parent = &node[n->parent-1]) {
            if (ind == parent->left) parent->left  = new_ind;
            else                     parent->right = new_ind;
        }
        if (new_ind) node[new_ind-1].parent = n->parent;
    }
    void RotateLeft(int ind) {
        int right_ind;
        Node *n = &node[ind-1], *o = &node[(right_ind = n->right)-1];
        ReplaceNode(ind, right_ind);
        n->right = o->left;
        if (o->left) node[o->left-1].parent = ind;
        o->left = ind;
        n->parent = right_ind;
        ComputeStateFromChildren(n);
        ComputeStateFromChildren(o);
    }
    void RotateRight(int ind) {
        int left_ind;
        Node *n = &node[ind-1], *o = &node[(left_ind = n->left)-1];
        ReplaceNode(ind, left_ind);
        n->left = o->right;
        if (o->right) node[o->right-1].parent = ind;
        o->right = ind;
        n->parent = left_ind;
        ComputeStateFromChildren(n);
        ComputeStateFromChildren(o);
    }
    void ComputeStateFromChildren(Node *n) {
        n->ComputeStateFromChildren(n->left ? &node[n->left -1] : 0, n->right ? &node[n->right-1] : 0);
    }
    void ComputeStateFromChildrenOnMaxPath(int ind) {
        Node *n = &node[ind-1];
        if (n->right) ComputeStateFromChildrenOnMaxPath(n->right);
        ComputeStateFromChildren(n);
    }
    void SetColor(int ind, int color) { node[ind-1].color = color; }
    int GetColor  (int ind) const { return ind ? node[ind-1].color : Black; }
    int GetParent (int ind) const { return node[ind-1].parent; }
    int GetSibling(int ind) const { return GetSibling(&node[GetParent(ind)-1], ind); }
    int GetMinNode(int ind) const {
        const Node *n = &node[ind-1];
        return n->left ? GetMinNode(n->left) : ind;
    }
    int GetMaxNode(int ind, Zipper *z=0) const {
        const Node *n = &node[ind-1];
        return n->right ? GetMaxNode(n->WalkRight(z), z) : ind;
    }

    struct LoadFromSortedArraysQuery { const K *k; const V *v; int max_height; };
    virtual void LoadFromSortedArrays(const K *k, const V *v, int n) {
        CHECK_EQ(0, node.size());
        LoadFromSortedArraysQuery q = { k, v, WhichLog2(NextPowerOfTwo(n, true)) };
        count = n;
        head = BuildTreeFromSortedArrays(&q, 0, n-1, 1);
    }
    virtual int BuildTreeFromSortedArrays(LoadFromSortedArraysQuery *q, int beg_ind, int end_ind, int h) {
        if (end_ind < beg_ind) return 0;
        CHECK_LE(h, q->max_height);
        int mid_ind = (beg_ind + end_ind) / 2, color = ((h>1 && h == q->max_height) ? Red : Black);
        int node_ind = node.Insert(Node(q->k[mid_ind], val.Insert(q->v[mid_ind]), 0, color))+1;
        int left_ind  = BuildTreeFromSortedArrays(q, beg_ind,   mid_ind-1, h+1);
        int right_ind = BuildTreeFromSortedArrays(q, mid_ind+1, end_ind,   h+1);
        Node *n = &node[node_ind-1];
        n->left  = left_ind;
        n->right = right_ind;
        if (left_ind)  node[ left_ind-1].parent = node_ind; 
        if (right_ind) node[right_ind-1].parent = node_ind; 
        ComputeStateFromChildren(&node[node_ind-1]);
        return node_ind;
    }

    void CheckProperties() const {
        CHECK_EQ(Black, GetColor(head));
        CheckNoAdjacentRedNodes(head);
        int same_black_length = -1;
        CheckEveryPathToLeafHasSameBlackLength(head, 0, &same_black_length);
    }
    void CheckNoAdjacentRedNodes(int ind) const {
        if (!ind) return;
        const Node *n = &node[ind-1];
        CheckNoAdjacentRedNodes(n->left);
        CheckNoAdjacentRedNodes(n->right);
        if (n->color == Black) return;
        CHECK_EQ(Black, GetColor(n->parent));
        CHECK_EQ(Black, GetColor(n->parent));
        CHECK_EQ(Black, GetColor(n->parent));
    }
    void CheckEveryPathToLeafHasSameBlackLength(int ind, int black_length, int *same_black_length) const {
        if (!ind) {
            if (*same_black_length < 0) *same_black_length = black_length;
            CHECK_EQ(*same_black_length, black_length);
            return;
        }
        const Node *n = &node[ind-1];
        if (n->color == Black) black_length++;
        CheckEveryPathToLeafHasSameBlackLength(n->left,  black_length, same_black_length);
        CheckEveryPathToLeafHasSameBlackLength(n->right, black_length, same_black_length);
    }

    virtual string DebugString(const string &name=string()) const {
        string ret = GraphVizFile::DigraphHeader(StrCat("RedBlackTree", name.size()?"_":"", name));
        ret += GraphVizFile::NodeColor("black"); PrintNodes(head, Black, &ret);
        ret += GraphVizFile::NodeColor("red");   PrintNodes(head, Red,   &ret);
        PrintEdges(head, &ret);
        return ret + GraphVizFile::Footer();
    }
    virtual void PrintNodes(int ind, int color, string *out) const {
        if (!ind) return;
        const Node *n = &node[ind-1];
        PrintNodes(n->left, color, out);
        if (n->color == color) GraphVizFile::AppendNode(out, StrCat(n->key));
        PrintNodes(n->right, color, out);
    }
    virtual void PrintEdges(int ind, string *out) const {
        if (!ind) return;
        const Node *n = &node[ind-1], *l=n->left?&node[n->left-1]:0, *r=n->right?&node[n->right-1]:0;
        if (l) { PrintEdges(n->left, out);  GraphVizFile::AppendEdge(out, StrCat(n->key), StrCat(l->key), "left" ); }
        if (r) { PrintEdges(n->right, out); GraphVizFile::AppendEdge(out, StrCat(n->key), StrCat(r->key), "right"); }
    }

    static int GetSibling(const Node *parent, int ind) { return ind == parent->left ? parent->right : parent->left; }
};

// RedBlackIntervalTree 

template <class K> struct RedBlackIntervalTreeZipper {
    pair<K, K> query;
    vector<pair<int, int> > path;
    RedBlackIntervalTreeZipper(bool update=0) {}
    RedBlackIntervalTreeZipper(const K &q, int head) : query(pair<K,K>(q,q)) { path.reserve(64); path.emplace_back(head, 0); }
    string DebugString() const { return ""; }
};

template <class K, class V, class Zipper = RedBlackIntervalTreeZipper<K> > struct RedBlackIntervalTreeNode {
    enum { Left, Right };
    pair<K,K> key;
    K left_max=0, right_max=0, left_min=0, right_min=0;
    unsigned val:31, left, right, parent, color:1;
    RedBlackIntervalTreeNode(pair<K,K> k, unsigned v, unsigned P, bool C=0)
        : key(k), val(v), left(0), right(0), parent(P), color(C) {}

    void SwapKV(RedBlackIntervalTreeNode *n) { swap(key, n->key); unsigned v=val; val=n->val; n->val=v; }
    pair<K,K> GetKey(Zipper*) const { return key; }
    int WalkLeft    (Zipper*) const { return left; }
    int WalkRight   (Zipper*) const { return right; }
    int UnwalkRight (Zipper*) const { return parent; }
    bool LessThan(const pair<K,K> &k, int, Zipper*) const { return key.first < k.first; }
    bool MoreThan(const pair<K,K> &k, int, Zipper*) const { return k.first < key.first; }
    void ComputeStateFromChildren(const RedBlackIntervalTreeNode *lc, const RedBlackIntervalTreeNode *rc) {
        left_max  = lc ? max(lc->key.second, max(lc->left_max, lc->right_max)) : 0;
        right_max = rc ? max(rc->key.second, max(rc->left_max, rc->right_max)) : 0;
        left_min  = lc ? ComputeMinFromChild(lc) : 0;
        right_min = lc ? ComputeMinFromChild(rc) : 0;
    }
    static K ComputeMinFromChild(const RedBlackIntervalTreeNode *n) {
        K ret = n->key.first;
        if (n->left_min)  ret = min(ret, n->left_min);
        if (n->right_min) ret = min(ret, n->right_min);
        return ret;
    }
};

template <class K, class V, class Zipper = RedBlackIntervalTreeZipper<K>,
          class Node = RedBlackIntervalTreeNode<K, V, Zipper> >
struct RedBlackIntervalTree : public RedBlackTree<pair<K, K>, V, Zipper, Node> {
    typedef RedBlackTree<pair<K, K>, V, Zipper, Node> Parent;

    const V *IntersectOne(const K &k) const { int n=IntersectOneNode(k); return n ? &Parent::val[Parent::data[n-1].val] : 0; }
          V *IntersectOne(const K &k)       { int n=IntersectOneNode(k); return n ? &Parent::val[Parent::data[n-1].val] : 0; }

    typename Parent::ConstIterator IntersectAll(const K &k) const { Zipper z(k, Parent::head); int n=IntersectAllNode(&z); return typename Parent::ConstIterator(this, n, z); }
    typename Parent::Iterator      IntersectAll(const K &k)       { Zipper z(k, Parent::head); int n=IntersectAllNode(&z); return typename Parent::Iterator     (this, n, z); }
    void IntersectAllNext(typename Parent::ConstIterator *i) const { if ((i->ind = IntersectAllNode(&i->zipper))) i->LoadKV(); else i->val=0; }
    void IntersectAllNext(typename Parent::Iterator      *i)       { if ((i->ind = IntersectAllNode(&i->zipper))) i->LoadKV(); else i->val=0; }

    int IntersectOneNode(const K &q) const {
        const Node *n;
        for (int ind = Parent::head; ind; /**/) {
            n = &Parent::node[ind-1];
            if (n->key.first <= q && q < n->key.second) return ind;
            ind = (n->left && q <= n->left_max) ? n->left : n->right;
        } return 0;
    }
    int IntersectAllNode(Zipper *z) const {
        const Node *n;
        while (z->path.size()) {
            auto &i = z->path.back();
            n = &Parent::node[i.first-1];
            if (i.second == 0 && ++i.second) if (Intersect(z->query, n->key)) return i.first;
            if (i.second == 1 && ++i.second && n->left  && z->query.first  <= n->left_max)  { z->path.emplace_back(n->left,  0); continue; }
            if (i.second == 2 && ++i.second && n->right && z->query.second >= n->right_min) { z->path.emplace_back(n->right, 0); continue; }
            z->path.pop_back();
        } return 0;
    }
    static bool Intersect(const pair<K,K> &q, const pair<K,K> &p) {
        return q.first <= p.second && p.first <= q.second;
    }
};

// Prefix-sum-keyed Red Black Tree, a Finger Tree variant

template <class K> struct PrefixSumZipper {
    K sum=0;
    bool update;
    vector<pair<int, bool> > path;
    PrefixSumZipper(bool U=0) : update(U) { if (update) path.reserve(64); }
    string DebugString() const { 
        string p;
        for (auto i : path) StrAppend(&p, p.size()?", ":"", i.first);
        return StrCat("PrefixSumZipper: sum=", sum, ", update=", update, ", path={", p, "}");
    }
};

template <class K, class V, class Zipper = PrefixSumZipper<K> >
struct PrefixSumKeyedRedBlackTreeNode {
    enum { Left, Right };
    K key, left_sum=0, right_sum=0;
    unsigned val:31, left, right, parent, color:1;
    PrefixSumKeyedRedBlackTreeNode(K k, unsigned v, unsigned P, bool C=0) : key(k), val(v), left(0), right(0), parent(P), color(C) {}

    void SwapKV(PrefixSumKeyedRedBlackTreeNode *n) { swap(key, n->key); unsigned v=val; val=n->val; n->val=v; }
    K   GetKey     (Zipper *z) const { return z->sum + left_sum; }
    int WalkLeft   (Zipper *z) const { return left; }
    int WalkRight  (Zipper *z) const { if (z) z->sum += (left_sum + key); return right; }
    int UnwalkRight(Zipper *z) const { if (z) z->sum -= (left_sum + key); return parent; }
    bool LessThan(const K &k, int ind, Zipper *z) const {
        if (!(GetKey(z) < k)) return 0;
        if (z->update) z->path.emplace_back(ind, Left);
        return 1;
    }
    bool MoreThan(const K &k, int ind, Zipper *z) const {
        if (!(k < GetKey(z))) return 0;
        if (z->update) z->path.emplace_back(ind, Right);
        return 1;
    }
    void ComputeStateFromChildren(const PrefixSumKeyedRedBlackTreeNode *lc, const PrefixSumKeyedRedBlackTreeNode *rc) {
        left_sum  = lc ? (lc->left_sum + lc->right_sum + lc->key) : 0;
        right_sum = rc ? (rc->left_sum + rc->right_sum + rc->key) : 0;
    }
};

template <class K, class V, class Zipper = PrefixSumZipper<K>, class Node = PrefixSumKeyedRedBlackTreeNode<K,V,Zipper> >
struct PrefixSumKeyedRedBlackTree : public RedBlackTree<K, V, Zipper, Node> {
    typedef RedBlackTree<K, V, Zipper, Node> Parent;
    function<K     (const V*)> node_value_cb;
    function<string(const V*)> node_print_cb;
    PrefixSumKeyedRedBlackTree() : 
        node_value_cb([](const V *v){ return 1;          }), 
        node_print_cb([](const V *v){ return StrCat(*v); }) {}

    virtual K GetCreateNodeKey(const typename Parent::Query *q) const { return node_value_cb(q->val); }
    virtual int ResolveInsertCollision(int ind, typename Parent::Query *q) { 
        int val_ind = Parent::val.Insert(*q->val), p_ind = ind;
        int new_ind = Parent::node.Insert(Node(GetCreateNodeKey(q), val_ind, 0))+1;
        Node *n = &Parent::node[ind-1], *nn = &Parent::node[new_ind-1];
        if (!n->left) n->left = new_ind;
        else Parent::node[(p_ind = Parent::GetMaxNode(n->left))-1].right = new_ind;
        nn->parent = p_ind;
        Parent::ComputeStateFromChildrenOnMaxPath(n->left);
        Parent::ComputeStateFromChildren(n);
        ComputeStateFromChildrenOnPath(q);
        Parent::InsertBalance(new_ind);
        Parent::count++;
        return new_ind;
    }
    virtual void ComputeStateFromChildrenOnPath(typename Parent::Query *q) {
        for (auto i = q->z.path.rbegin(), e = q->z.path.rend(); i != e; ++i) 
            Parent::ComputeStateFromChildren(&Parent::node[i->first-1]);
    }

    virtual void PrintNodes(int ind, int color, string *out) const {
        if (!ind) return;
        const Node *n = &Parent::node[ind-1];
        string v = node_print_cb(&Parent::val[n->val]);
        PrintNodes(n->left, color, out);
        if (n->color == color) StrAppend(out, "node [label = \"", v, " v:", n->key, "\nlsum:", n->left_sum,
                                         " rsum:", n->right_sum, "\"];\r\n\"", v, "\";\r\n");
        PrintNodes(n->right, color, out);
    }
    virtual void PrintEdges(int ind, string *out) const {
        if (!ind) return;
        const Node *n = &Parent::node[ind-1], *l=n->left?&Parent::node[n->left-1]:0, *r=n->right?&Parent::node[n->right-1]:0;
        string v = node_print_cb(&Parent::val[n->val]), lv = l?node_print_cb(&Parent::val[l->val]):"", rv = r?node_print_cb(&Parent::val[r->val]):"";
        if (l) { PrintEdges(n->left,  out); StrAppend(out, "\"", v, "\" -> \"", lv, "\" [ label = \"left\"  ];\r\n"); }
        if (r) { PrintEdges(n->right, out); StrAppend(out, "\"", v, "\" -> \"", rv, "\" [ label = \"right\" ];\r\n"); }
    }

    void LoadFromSortedVal() {
        int n = Parent::val.size();
        CHECK_EQ(0, Parent::node.size());
        Parent::count = Parent::val.size();
        Parent::head = BuildTreeFromSortedVal(0, n-1, 1, WhichLog2(NextPowerOfTwo(n, true)));
    }
    int BuildTreeFromSortedVal(int beg_val_ind, int end_val_ind, int h, int max_h) {
        if (end_val_ind < beg_val_ind) return 0;
        CHECK_LE(h, max_h);
        int mid_val_ind = (beg_val_ind + end_val_ind) / 2, color = ((h>1 && h == max_h) ? Parent::Red : Parent::Black);
        int ind = Parent::node.Insert(Node(node_value_cb(&Parent::val[mid_val_ind]), mid_val_ind, 0, color))+1;
        int left_ind  = BuildTreeFromSortedVal(beg_val_ind,   mid_val_ind-1, h+1, max_h);
        int right_ind = BuildTreeFromSortedVal(mid_val_ind+1, end_val_ind,   h+1, max_h);
        Node *n = &Parent::node[ind-1];
        n->left  = left_ind;
        n->right = right_ind;
        if (left_ind)  Parent::node[ left_ind-1].parent = ind;
        if (right_ind) Parent::node[right_ind-1].parent = ind;
        Parent::ComputeStateFromChildren(&Parent::node[ind-1]);
        return ind;
    }

    V *Update(const K &k, const V &v) {
        typename Parent::Query q(k, 0, 1);
        Node *n;
        for (int ind = Parent::head; ind; ) {
            n = &Parent::node[ind-1];
            if      (n->MoreThan(q.key, ind, &q.z)) ind = n->WalkLeft (&q.z);
            else if (n->LessThan(q.key, ind, &q.z)) ind = n->WalkRight(&q.z);
            else {
                n->key = node_value_cb(&v);
                ComputeStateFromChildrenOnPath(&q);
                return &Parent::val[n->val];
            }
        } return 0;
    }
};

}; // namespace LFL
#endif // __LFL_LFAPP_TREE_H__
