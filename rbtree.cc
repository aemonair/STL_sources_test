#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <queue>

using namespace std;

typedef bool __rb_tree_color_type;
const __rb_tree_color_type __rb_tree_red = false; // 红色为0
const __rb_tree_color_type __rb_tree_black = true; // 黑色为1

struct __rb_tree_node_base
{
    typedef __rb_tree_color_type color_type;
    typedef __rb_tree_node_base* base_ptr;

    color_type color; // 节点颜色，非红即黑
    base_ptr parent;  // 父节点 RB树的许多操作，必须知道父节点
    base_ptr left;    // 左节点
    base_ptr right;   // 右节点

    static base_ptr minimum(base_ptr x)
    {
        // 最小值一直查找左子树
        while (x->left != 0) x = x->left;
        return x;
    }

    static base_ptr maximum(base_ptr x)
    {
        // 最大值一直查找右子树
        while (x->right != 0) x = x->right;
        return x;
    }
};

template <class Value>
struct __rb_tree_node : public __rb_tree_node_base
{
    typedef __rb_tree_node<Value>* link_type;
    Value value_field; // 节点值
    typedef std::allocator<Value> allocator_type;

    // 构造函数，接受一个allocator_type参数
    explicit __rb_tree_node(const allocator_type& a) : value_field(a) {}
};

// 基层迭代器
struct __rb_tree_base_iterator
{
    typedef __rb_tree_node_base::base_ptr base_ptr;
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef ptrdiff_t difference_type;
    base_ptr node;// 与容器产生连接关系
                  //它用来与容器之间产生一个连结关系(make a reference)

                  // 只有operator++调用
    void increment()
    {
        // 有右子树，找右子树的最左节点
        if (node->right != 0) {  // 如果有右子节点。状况(1)
            node = node->right; // 就向右走
            while (node->left != 0)// 然后一直往左子树走到底
                node = node->left;// 即是解答
        }
        // 没有右子树，找不是右节点的父节点
        else {                               // 没有右子节点。状况(2)
            base_ptr y = node->parent;// 找出父节点
            while (node == y->right) { // 如果现行节点本身是个右子节点
                node = y;                   // 就一直上溯，直到 “不为右子节点” 止
                y = y->parent;
            }
            // 根节点无右子树的情况
            if (node->right != y)       // 若此时的右子节点不等于此时的父节点
                node = y;                    // 状况(3)此时的父节点即为解答
                                             // 否则此时的node为解答.状況(4)
        }
        // 注意，以上判断 “若此时的右子节点不等于此时的父节点” ，是为了应付一种
        // 特殊情况:我们欲寻找根节点的下一节点，而恰巧根节点无右子节点
        // 当然，以上特殊做法必须配合RB-tree 根节点与特殊之header 之间的
        // 特殊关系
    }
    // 只有opeartor--调用
    void decrement()
    {
        // 为红色节点且父节点的父节点等于自己
        // 这种情况发生在node为header时（亦即node为end()时）
        // header的左子节点即leftright，指向整个树的min节点
        // header的右子节点即mostright，指向整个树的max节点
        if (node->color == __rb_tree_red &&    // 如果是红节点，且
                node->parent->parent == node) // 父节点的父节点等于自己
            node = node->right;// 状况 (1)       // 右子节点即为解答
                               //以上情况发生于node为header时(亦即node为end()时)
                               // 注意，header 之右子节点即mostright，指向整棵树的max节点
                               // 有左子节点，找左子节点的最右子节点
        else if (node->left != 0) {             // 如果有左子节点。状况(2)
            base_ptr y = node->left;           // 令y指向左子节点
            while (y->right != 0)                  // 当y有右子节点时
                y = y->right;                      // 一直往右子节点走到底
            node = y;                              // 最后即为答案
        }
        // 不是根节点也没有左子节点，找不是左节点的父节点
        else {                                        // 既非根节点，亦无左子节点
            base_ptr y = node->parent;         // 状况(3)找出父节点
            while (node == y->left) {           // 当现行节点身为左子节点
                node = y;                            // 一直交替往上走，直到现行节点
                y = y->parent;                     // 不为左子节点
            }
            node = y;                              // 此时之父节点即为答案
        }
    }
};
inline bool operator==(const __rb_tree_base_iterator& __x,
        const __rb_tree_base_iterator& __y) {
    return __x.node == __y.node;
}

inline bool operator!=(const __rb_tree_base_iterator& __x,
        const __rb_tree_base_iterator& __y) {
    return __x.node != __y.node;
}

// RB-tree 的正规迭代器
template <class Value, class Ref, class Ptr>
struct __rb_tree_iterator : public __rb_tree_base_iterator
{
    typedef Value value_type;
    typedef Ref reference;
    typedef Ptr pointer;
    typedef __rb_tree_iterator<Value, Value&, Value*>
        iterator;
    typedef __rb_tree_iterator<Value, const Value&, const Value*>
        const_iterator;
    typedef __rb_tree_iterator<Value, Ref, Ptr>
        self;
    typedef __rb_tree_node<Value>* link_type;

    __rb_tree_iterator() {}
    __rb_tree_iterator(link_type __x) { node = __x; }
    __rb_tree_iterator(const iterator& __it) { node = __it.node; }

    reference operator*() const { return link_type(node)->value_field; }
#ifndef __SGI_STL_NO_ARROW_OPERATOR
    pointer operator->() const { return &(operator*()); }
#endif /* __SGI_STL_NO_ARROW_OPERATOR */

    self& operator++() { increment(); return *this; }
    self operator++(int) {
        self __tmp = *this;
        increment();
        return __tmp;
    }

    self& operator--() { decrement(); return *this; }
    self operator--(int) {
        self __tmp = *this;
        decrement();
        return __tmp;
    }
};

template <class Key, class Value, class KeyOfValue, class Compare = std::less<Key>,
         class Alloc = std::allocator<Key> >
class rb_tree {
    protected:

        typedef void* void_pointer;
        typedef __rb_tree_node_base* base_ptr;
        typedef __rb_tree_node<Value> rb_tree_node;
        //typedef simple_alloc<rb_tree_node, Alloc> rb_tree_node_allocator;
        // 使用std::allocator来管理rb_tree_node的内存
        typedef std::allocator<Value> allocator_type;

        typedef typename allocator_type::template rebind<rb_tree_node>::other rb_tree_node_allocator;
        rb_tree_node_allocator rb_tree_node_alloc;

        typedef __rb_tree_color_type color_type;
        //typedef __rb_tree_node_base* base_ptr;
        //typedef __rb_tree_node<Value> __rb_tree_node;
        //typedef __rb_treecolor_type color_type;
    public:
        typedef Key key_type;
        typedef Value value_type;
        typedef value_type* pointer;
        typedef const value_type* const_pointer;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef rb_tree_node* link_type;
        typedef size_t size_type;
        typedef std::ptrdiff_t difference_type;

        template <typename K, typename V, typename KOV, typename C, typename A>
            friend int printTreeGrid(rb_tree<K, V, KOV, C, A>&, bool);
        //template<typename T>
        //friend int printTreeGrid(rb_tree& tree, bool printline);
        //friend int printTreeGrid(typename rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::link_type ptr);

        //typedef typename base::allocator_type allocator_type;
        // allocator_type get_allocator() const { return rb_tree_node_allocator; }
        //typedef std::allocator<Value> allocator_type;


    protected:
        link_type get_node () {// return rb_tree_node_allocator::allocate();
            return rb_tree_node_alloc.allocate(1); // 注意，allocate通常需要一个数量参数
        }
        void put_node (link_type p) {// rb_tree_node_allocator::deallocate (p);
            rb_tree_node_alloc.deallocate(p, 1); // 同样，deallocate通常需要一个数量参数
        }
        //#ifdef __STL_USE_NAMESPACES
        //using base::get_node;
        //using base::put_node;
        //using base::header;
        //#endif /* __STL_USE_NAMESPACES */

        link_type create_node(const value_type& x)
        {
            link_type tmp = get_node(); // 配置空间
                                        //__STL_TRY {
                                        //construct(&tmp->value_field, x); // 构造
            rb_tree_node_alloc.construct(&tmp->value_field, x); // 构造
                                                                //}
                                                                //__STL_UNWIND(put_node(tmp));
            return tmp;
        }

        // 复制节点（颜色与值）
        link_type clone_node(link_type x)
        {
            link_type tmp = create_node(x->value_field);
            tmp->color = x->color;
            tmp->left = 0;
            tmp->right = 0;
            return tmp;
        }

        void destroy_node(link_type p)
        {
            destroy(&p->value_field); // 析构
            put_node(p);              // 释放内存
        }

    protected:
        size_type node_count; // keeps track of size of tree
                              // 追踪记录树的大小(节点数量)
        link_type header;
        Compare key_compare; // 比较大小函数对象

        // 取得header的成员
        // 以下三个函数用来方便取得 header 的成员
        link_type& root() const
        { return (link_type&) header->parent; }
        link_type& leftmost() const
        { return (link_type&) header->left; }
        link_type& rightmost() const
        { return (link_type&) header->right; }

        // 取得x的成员
        // 以下六个函数用来方便取得节点x的成员
        static link_type& left(link_type x)
        { return (link_type&)(x->left); }
        static link_type& right(link_type x)
        { return (link_type&)(x->right); }
        static link_type& parent(link_type x)
        { return (link_type&)(x->parent); }
        static reference value(link_type x)
        { return x->value_field; }
        static const Key& key(link_type x)
        { return KeyOfValue()(value(x)); }
        static color_type& color(link_type x)
        { return (color_type&)(x->color); }

        // 通过指针取得节点x的成员
        // 以下六个函数用来方便取得节点× 的成员
        static link_type& left(base_ptr x)
        { return (link_type&)(x->left); }
        static link_type& right(base_ptr x)
        { return (link_type&)(x->right); }
        static link_type& parent(base_ptr x)
        { return (link_type&)(x->parent); }
        static reference value(base_ptr x)
        { return ((link_type)x)->value_field; }
        //static const Key& key(base_ptr x) {
        //auto tempKey = KeyOfValue()(value(link_type(x)));
        //return std::move(tempKey);
        //}
        static const Key& key(base_ptr x)
        { return KeyOfValue()(value(link_type(x)));}
        //static const Key& key(base_ptr x) {
        //return std::move(
        //KeyOfValue()(value(link_type(x)))
        //);
        //}
        static color_type& color(base_ptr x)
        { return (color_type&)(link_type(x)->color); }

        // 求极值
        // 求取极大值和极小值. node class 有实现此功能，交给它们完成即可
        static link_type minimum(link_type x)
        { return (link_type)  __rb_tree_node_base::minimum(x); }

        static link_type maximum(link_type x)
        { return (link_type) __rb_tree_node_base::maximum(x); }

    public:
        typedef __rb_tree_iterator<value_type, reference, pointer> iterator;

        typedef __rb_tree_iterator<value_type, const_reference, const_pointer>
            const_iterator;

    private:
        iterator __insert(base_ptr x, base_ptr y, const value_type& v);
        link_type __copy(link_type x, link_type p);
        void __erase(link_type x);

        void init() {
            header=get_node();//产生一个节点空间，令header指向它
            color(header)=__rb_tree_red; // 令header 为红色，用来区分header
                                         // 和root，在iterator.operator++ 中
            root () = 0;
            leftmost() = header; //令header的左子节点为自己
            rightmost () = header;// 令header 的右子节点为自己
        }

    private:
        void empty_initialize() {
            // header为红色，用来区分header与root
            color(header) = __rb_tree_red; // used to distinguish header from
                                           // __root, in iterator.operator++
                                           // 令header 为红色，用来区分header
                                           // 和root，在iterator.operator++ 中
            root() = 0;
            leftmost() = header;  //令header的左子节点为自己
            rightmost() = header; //令header的右子节点为自己
        }

    public:
        // allocation/deallocation
        //rb_tree()
        //  : base(allocator_type()), node_count(0), key_compare()
        //  { empty_initialize(); }
        rb_tree(const Compare& comp = Compare() )
            :node_count(0), key_compare(comp) { init(); }
        ~rb_tree() {
            //clear();
            put_node(header);
        }

        rb_tree<Key,Value,KeyOfValue,Compare,Alloc>&
            operator=(const rb_tree<Key,Value,KeyOfValue,Compare,Alloc>& x);

    public:
        // accessors:
        Compare key_comp() const { return key_compare; }
        // RB树的起头为最左/最小节点
        iterator begin() { return leftmost(); } // RB树的起头为最左(最小)节点处
        const_iterator begin() const { return leftmost(); } // RB树的终点力header所指处
                                                            // RB树的终点为header所指处？这里实现不一样，书上是指向header了
        iterator end() { return header; }
        const_iterator end() const { return header; }
        //std::reverse_iterator rbegin() { return reverse_iterator(end()); }
        //std::const_reverse_iterator rbegin() const {
        //return const_reverse_iterator(end());
        //}
        //std::reverse_iterator rend() { return reverse_iterator(begin()); }
        //std::const_reverse_iterator rend() const {
        //return const_reverse_iterator(begin());
        //}
        bool empty() const { return node_count == 0; }
        size_type size() const { return node_count; }
        size_type max_size() const { return size_type(-1); }

        // insert/erase
        pair<iterator,bool> insert_unique(const value_type& x);
        iterator insert_equal(const value_type& x);

        // 不允许重复
        // 将×插入到RB-tree中(保持节点值独一无二)
        iterator insert_unique(iterator __position, const value_type& x);
        // 允许重复
        // 将×插入到RB-tree中(允许节点值重复)。
        iterator insert_equal(iterator __position, const value_type& x);
};

// 插人新值;节点键值允许重复
//注意，返回值是一个RB-tree迭代器，指向新增节点
// 以上，x 为新值插人点

template <class Key, class Value, class KeyOfValue,
         class Compare, class Alloc>
         typename rb_tree<Key,Value,KeyOfValue,Compare,Alloc>::iterator
    rb_tree<Key,Value,KeyOfValue,Compare,Alloc>
::insert_equal(const Value& v)
{
    link_type y = header;
    link_type x = root(); // 从根节点开始
    while (x != 0) {          // 往下寻找适当的插入点
        y = x;
        // 当前值比v大则往左，否则小于或等于则往右
        x = key_compare(KeyOfValue()(v), key(x)) ?
            left(x) : right(x);
    }
    // x为新值插入点， y为插入点之父节点， v为新值
    return __insert(x, y, v);
}

// 插入新值，不允许重复，重复则插入无效
// 返回值为pair，迭代器和插入是否成功
// 第一元素是个 RB-tree 迭代器，指向新增节点，
// 第二叉素表示插入成功与否
template <class Key, class Value, class KeyOfValue,
         class Compare, class Alloc>
         pair<typename rb_tree<Key,Value,KeyOfValue,Compare,Alloc>::iterator,
         bool>
    rb_tree<Key,Value,KeyOfValue,Compare,Alloc>
::insert_unique(const Value& v)
{
    std::cout << "insert_unique:" << v << std::endl;
    link_type y = header;
    link_type x = root(); // 从根节点开始寻找插入点
    bool comp = true;
    while (x != 0) {          // 从根节点开始，往下寻找适当的插人点
        y = x;
        // v是否小于当前节点键值?
        comp = key_compare(KeyOfValue()(v), key(x));
        // 遇大往左，否则小于等于往右
        x = comp ? left(x) : right(x);
    }
    // 结束while循环后，y为插入点的父节点 (此时的它必为叶节点)
    typedef __rb_tree_iterator<value_type, reference, pointer> iterator;
    iterator j = iterator(y);   // 迭代器j指向插入点之父节点y的迭代器
    if (comp) {// 如果离开while循环时comp为真(表示遇“大”)表示y的键值大于插入键值，则插入于左侧
        if (j == begin()) {   // 如果插入点的父节点为最左节点
            return pair<iterator,bool>(__insert(x, y, v), true);
            // 以上，× 为插人点，y 为插人点之父节点，v 为新值
        } else {// 否则(插人点之父节点不为最左节点)调整j
            --j; //调整，回头准备测试.
        }
    }
    // y的键值小于新插入键值(遇小)，则插入于右侧
    if (key_compare(key(j.node), KeyOfValue()(v)))
        return pair<iterator,bool>(__insert(x, y, v), true);
    // 以上，x 为新值插人点，y 为插人点之父节点，v 为新值

    // 进行至此，表示新值 一定与树中键值重复，那么就不该插人新值;返回插入失败
    return pair<iterator,bool>(j, false);
}

// 真正的插入函数
template <class Key, class Value, class KeyOfValue,
         class Compare, class Alloc>
         typename rb_tree<Key,Value,KeyOfValue,Compare,Alloc>::iterator
    rb_tree<Key,Value,KeyOfValue,Compare,Alloc>
::__insert(base_ptr x_, base_ptr y_, const Value& v)
{
    // 参数x为插入点，参数y为插入点之父节点，参数v为新值
    link_type x = (link_type) x_;
    link_type y = (link_type) y_;
    link_type z;

    // compare为函数对象,
    // key _compare 是键值大小比较准则。应该会是个function object
    if (y == header || x != 0 ||
            key_compare(KeyOfValue()(v), key(y))) {
        z = create_node(v);        // 产生新节点
                                   // 也使得当y为header时，leftmost() = z
        left(y) = z;               // also makes leftmost() = z
                                   //    when y == header
        if (y == header) {
            root() = z;
            rightmost() = z;
        }
        // 如果y为最左节点，调整leftmost
        else if (y == leftmost())
            leftmost() = z;   // maintain leftmost() pointing to min node
                              // 维护1eftmost()，使它永远指向最左节点
    }
    // 右侧与左侧相同，必要时调整rightmost
    else {
        z = create_node(v);         // 产生一个新节点
        right(y) = z;               // 令新节点成为插人点之父节点y的右子节点
        if (y == rightmost())
            rightmost() = z;  // maintain rightmost() pointing to max node
                              // 维护 rightmost()，使它永远指向最右节点
    }
    parent(z) = y;                // 设置新节点的父节点
    left(z) = 0;                    // 设置新节点的左子节点
    right(z) = 0;                   // 设置新节点的右子节点
                                    // 新节点的颜色将在_rb_tree_rebalance()设定(并调整)
    __rb_tree_rebalance(z, header->parent); // 新增节点与root
                                            // 参数一为新增节点，参数二为root
    ++node_count;                     // 节点数累加
    return iterator(z); // 返回指向新增节点的迭代器
}
// 全局函数 左旋函数
// 插入违反红黑树规则时进行旋转调整
// 新节点必为红节点。如果插入处之父节点亦为红节点，就违反红黑树规则，此时必须
// 做树形旋转(及颜色改变，在程序它处)
    inline void
__rb_tree_rotate_left(__rb_tree_node_base* x, __rb_tree_node_base*& root)
{
    std::cout << "  _rb_tree_rotate_left"
        << "  x:" << (static_cast<__rb_tree_node<int> *>(x))->value_field
        << ", root:" << static_cast<__rb_tree_node<int> *>(root)->value_field << std::endl;
    // x为旋转点，y为旋转点的右子节点
    __rb_tree_node_base* y = x->right;
    x->right = y->left;
    if (y->left !=0)
        y->left->parent = x;//别忘了回马枪设定父节点
    y->parent = x->parent;

    //  根节点情况
    // 令y完全顶替×的地位(必须将×对其父节点的关系完全接收过来)
    if (x == root)                       // ×为根节点
        root = y;
    else if (x == x->parent->left) // x为其父节点的左子节点
        x->parent->left = y;
    else                                     // x为其父节点的右子节点
        x->parent->right = y;
    y->left = x;
    x->parent = y;
}

// 全局函数，右旋函数
// 插入违反红黑树规则时进行旋转调整
// 新节点必为红节点。如果插入处之父节点亦为红节点，就违反红黑树规则，此时必须
// 做树形旋转(及颜色改变，在程序其它处)
    inline void
__rb_tree_rotate_right(__rb_tree_node_base* x, __rb_tree_node_base*& root)
{
    std::cout << "  _rb_tree_rotate_right"
        << "  x:" << (static_cast<__rb_tree_node<int> *>(x))->value_field
        << ", root:" << static_cast<__rb_tree_node<int> *>(root)->value_field << std::endl;
    // × 为旋转点
    __rb_tree_node_base* y = x->left;  // y 为旋转点的左子节点
    x->left = y->right;
    if (y->right != 0)
        y->right->parent = x; //别忘了回马枪设定父节点
    y->parent = x->parent;

    // 令y完全顶替×的地位(必须将×对其父节点的关系完全接收过来)
    if (x == root)                         // x为根节点
        root = y;
    else if (x == x->parent->right)  // x为其父节点的右子节点
        x->parent->right = y;
    else                                       // x为其父节点的左子节点
        x->parent->left = y;
    y->right = x;
    x->parent = y;
}

// 调整RB-tree，旋转及变色
// 全局函数
// 重新令树形平衡(改变颜色及旋转树形)
// 参数一为新增节点，参数二为root
    inline void
__rb_tree_rebalance(__rb_tree_node_base* x, __rb_tree_node_base*& root)
{
    std::cout << "_rb_tree_rotate_rebalance"
        << "  x:" << (static_cast<__rb_tree_node<int> *>(x))->value_field
        << ", root:" << static_cast<__rb_tree_node<int> *>(root)->value_field << std::endl;
    x->color = __rb_tree_red;             // 新节点必为红色
                                          // 父节点为红时
    while (x != root && x->parent->color == __rb_tree_red) {
        // 父节点为左节点时
        if (x->parent == x->parent->parent->left) {
            // 令y为叔父节点
            __rb_tree_node_base* y = x->parent->parent->right;
            // 叔父节点存在且为红色
            // 将叔父与父节点变黑色，祖父节点变红色，x设置为祖父节点
            if (y && y->color == __rb_tree_red) {
                x->parent->color = __rb_tree_black;
                y->color = __rb_tree_black;
                x->parent->parent->color = __rb_tree_red;
                x = x->parent->parent;
            }
            // 叔父节点不存在，或者存在为黑色
            else {
                // 如果新节点为父节点的右子节点，x进行左旋
                if (x == x->parent->right) {
                    x = x->parent;
                    __rb_tree_rotate_left(x, root); // 第一参数为左旋点
                }
                // 父节点变黑，祖父节点变红，祖父右旋
                x->parent->color = __rb_tree_black;
                x->parent->parent->color = __rb_tree_red;
                __rb_tree_rotate_right(x->parent->parent, root);
                // 第一参数为右旋点
            }
        }
        // 父节点为祖父节点之右节点时
        else {
            // y为叔父节点
            __rb_tree_node_base* y = x->parent->parent->left;
            // 叔父节点存在且为红色
            // 将叔父与父节点变黑色，祖父节点变红色，x设置为祖父节点
            if (y && y->color == __rb_tree_red) {
                x->parent->color = __rb_tree_black;
                y->color = __rb_tree_black;
                x->parent->parent->color = __rb_tree_red;
                x = x->parent->parent; // 准备继续往上层检查
            }
            // 叔父节点不存在，或者存在为黑色
            else {
                // 如果新节点为父节点的左子节点，x进行右旋
                if (x == x->parent->left) {
                    x = x->parent;
                    __rb_tree_rotate_right(x, root); // 第一参数为右旋点
                }
                // 父节点变黑，祖父节点变红，祖父左旋
                x->parent->color = __rb_tree_black;
                x->parent->parent->color = __rb_tree_red;
                __rb_tree_rotate_left(x->parent->parent, root);
                // 第一参数为左旋点
            }
        }
    }// while 结束
    // std::cout << "end rebalance while" << std::endl;
    // 根节点永远为黑色
    root->color = __rb_tree_black;
}

template <typename T>
struct identity {
    constexpr T operator()(const T& t) const { return t; }
};


// 计算树的最大深度
template <typename T>
int maxDepth(T* root) {
    if (!root) return 0;
    return 1 + std::max(maxDepth(root->left), maxDepth(root->right));
}
//template <typename T>
//int printTreeGrid(T & Root, bool printline=false)
    template <typename Key, typename Value, typename KeyOfValue, typename Compare, typename Alloc>
int printTreeGrid(rb_tree<Key, Value, KeyOfValue, Compare, Alloc>& tree, bool printline = false)
{
    auto root = tree.root();
    //using value_type = decltype(root);
    using value_type= typename rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::link_type;
    //using value_type = tree.link_type;


    int depth = maxDepth(root);
    int last_layer_offset = 1 ; //(1 << (depth - 1)) - 1; // 最后一层的offset
    int max_width = (last_layer_offset * 2 + 1) * (1 << (depth - 1)); // 确保最后一层有足够的空间
                                                                      //std::cout << "last_layer_offset:" << last_layer_offset << ",max_width:" << max_width << std::endl;

    std::vector<std::vector<char>> grid(depth * 2 - 1, std::vector<char>(max_width, ' '));
    std::vector<std::vector<bool>> color(depth * 2 - 1, std::vector<bool>(max_width, false));

    //std::queue<std::tuple<__rb_tree_node<Value> *, int, int>> q; // (node, row, col)
    //std::queue<std::tuple<value_type,int, int>> q; // (node, row, col)
    std::queue<std::tuple<__rb_tree_node<Value>*, int, int>> q;
    q.push({root, 0, (max_width - 1) / 2}); // 根节点在第一行中间


    std::vector<int> offsets(depth, 0); // 存储每一层的offset
    offsets[depth - 1] = last_layer_offset;

    // 计算每一层的offset
    for (int i = depth - 2; i >= 0; --i) {
        offsets[i] = offsets[i + 1] * 2;
        //std::cout << "offset:" << i << " :" << offsets[i] << std::endl;
    }
    for (auto i=0; i< offsets.size(); ++i) {
        //std::cout << "offset:" << i << " :" << offsets[i] << std::endl;
    }

    while (!q.empty()) {
        auto [node, row, col] = q.front();
        //std::cout << "row:" << row << ",col:" << col << ",value:" << node->value << std::endl;
        q.pop();
        grid[row][col] = node->value_field + '0'; // 将数值转换为字符
        if (node->color == true) {
            color[row][col] = true;
        }

        int next_row = row + 2; // 下一层的行索引，考虑到行间距
        int current_offset = offsets[row/2];
        //std::cout << "row:" << row << ";current_offset:" << current_offset << std::endl;

        if (node->left) {
            int left_col = col - current_offset/2;
            //std::cout << "left_col:" << col-current_offset/2 << std::endl;
            //q.push({node->left, next_row, left_col});
            //q.push(std::make_tuple(node->left, next_row, left_col));
            q.push(std::make_tuple(
                        static_cast<__rb_tree_node<Value>*>(node->left),
                        next_row, left_col));
            if (printline) {
                grid[row + 1][col - (current_offset / 4)] = '/'; // 斜杠位置
            }
        }
        if (node->right) {
            int right_col = col + current_offset/2;
            //std::cout << "right_col:" << col+current_offset/2 << std::endl;
            //q.push({node->right, next_row, right_col});
            //q.push(std::make_tuple(node->right, next_row, right_col));
            q.push(std::make_tuple(
                        static_cast<__rb_tree_node<Value>*>(node->right),
                        next_row, right_col));
            if (printline) {
                grid[row + 1][col + (current_offset / 4)] = '\\'; // 斜杠位置
            }
        }
    }

    // 有红黑颜色判断:
    for (int i = 0; i < grid.size(); ++i) {
        for (int j = 0; j < grid[i].size(); ++j) {
            auto c = grid[i][j];
            switch (c) {
                case ' ':
                case '/':
                case '\\':
                    std::cout << (c) << " ";
                    break;
                default:
                    if (color[i][j] == false) {
                        // std::cout << "\e[31m这是红色文本\e[0m" << std::endl;
                        std::cout << "\e[31m" << (c-'0') << "\e[0m" << " ";
                    } else {
                        // std::cout << "\e[30m这是黑色文本\e[0m" << std::endl;
                        // std::cout << "\e[30m" << (c-'0') << "\e[0m" << " " ;
                        // std::cout << "\033[47;30m这是黑色文本在白色背景上\033[0m" << std::endl;
                        // std::cout << "\033[47;30m" << (c-'0') << "\033[0m" << " ";
                        std::cout << (c-'0') << " ";
                    }
                    break;
            }
        }
        std::cout << std::endl;
    }
    // 没有颜色:
#if 0
    for (const auto& row : grid)
    {
        for (char c : row)
        {
            switch (c)
            {
                case ' ':
                case '/':
                case '\\':
                    std::cout << (c) << " ";
                    break;
                default:
                    std::cout << (c-'0') << " ";
                    break;
            }
        }
        std::cout << std::endl;
    }
#endif
    return 0;
}
int test1()
{
    rb_tree<int,int,identity<int>, less<int>> itree;
    std::cout << "main:" << std::endl;
    std::cout << "RBTree:" << std::endl;
    itree.insert_unique(10);
    std::cout << "insert_unique:" << std::endl;
    std::cout << "end" << std::endl;
    itree.insert_unique(7);
    itree.insert_unique(8);
    printTreeGrid(itree);
    itree.insert_unique(6);
    itree.insert_unique(5);
    itree.insert_unique(4);
    itree.insert_unique(3);
    printTreeGrid(itree);
    return 0;
}
int test()
{
    rb_tree<int,int,identity<int>, less<int>> itree;
    itree.insert_unique(10);      // __rb_tree_rebalance
    itree.insert_unique(7);       // __rb_tree_rebalance
    itree.insert_unique(8);       // __rb_tree_rebalance
                                      // __rb_tree_rotate_left
                                      // __rb_tree_rotate_right
    itree.insert_unique(15);      // __rb_tree_rebalance
    itree.insert_unique(5);       // __rb_tree_rebalance
    itree.insert_unique(6);       // __rb_tree_rebalance
                                      // __rb_tree_rotate_left
                                      // __rb_tree_rotate_right
    itree.insert_unique(11);       // __rb_tree_rebalance
                                      // __rb_tree_rotate_right
                                      // __rb_tree_rotate_left
    itree.insert_unique(13);       // __rb_tree_rebalance
    itree.insert_unique(12);
    printTreeGrid(itree);

    std::cout << "tree size:" << itree.size() << endl;   // 9
    rb_tree<int, int, identity<int>, less<int>>::iterator ite1 = itree.begin();
    rb_tree<int, int, identity<int>, less<int>>::iterator ite2 = itree.end();
    for (; ite1 != ite2; ++ite1)
        cout << *ite1 << ' ';        // 5 6 7 8 10 11 12 13 15
    std::cout << endl;
    ite1 = itree.begin();

    // 测试颜色和operator ++ (亦即 __rb_tree_iterator_base::increment)
    __rb_tree_base_iterator rbtite;
    for (; ite1 != ite2; ++ite1) {
        rbtite = __rb_tree_base_iterator(ite1);
        // 以上，向上转型up-casting，永远没问题。见《多型与虚拟2/e》第三章
        cout << *ite1 << "(" << rbtite.node->color << ") ";
    }
    cout << endl;
    return 0;
}
int main()
{
    test();
    return 0;
}
