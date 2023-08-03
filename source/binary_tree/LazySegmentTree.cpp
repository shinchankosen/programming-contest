/**
 * @brief 遅延伝搬セグメント木のCRTP基底クラス
 * 
 * @tparam S 値モノイドの型
 * @tparam F 作用素モノイドの型
 * @tparam ActualSegTree 派生クラス
 */
template <typename S, typename F, typename ActualLazySegTree>
class LazySegTreeBase {
    S op(const S& a, const S& b) const { return static_cast<const ActualLazySegTree&>(*this).op(a, b); }
    S e() const { return static_cast<const ActualLazySegTree&>(*this).e(); }
    S mapping(const F& f, const S& x, int l, int r) const { return static_cast<const ActualLazySegTree&>(*this).mapping(f, x, l, r); }
    F composition(const F& f, const F& g) const { return static_cast<const ActualLazySegTree&>(*this).composition(f, g); }
    F id() const { return static_cast<const ActualLazySegTree&>(*this).id(); }

    int n, sz, height;
    std::vector<S> data;
    std::vector<F> lazy;

    void update(int k) { data[k] = op(data[2 * k], data[2 * k + 1]); }
    void apply_node(int k, int h, const F& f) {
        int l = (k << h) & (sz - 1);
        int r = l + (1 << h);
        data[k] = mapping(f, data[k], l, r);
        if(k < sz) lazy[k] = composition(f, lazy[k]);
    }
    void push(int k, int h) {
        apply_node(2 * k, h-1, lazy[k]);
        apply_node(2 * k + 1, h-1, lazy[k]);
        lazy[k] = id();
    }

    class LazySegTreeReference {
        LazySegTreeBase& segtree;
        int k;
    public:
        LazySegTreeReference(LazySegTreeBase& segtree, int k) : segtree(segtree), k(k) {}
        LazySegTreeReference& operator=(const S& x) {
            segtree.set(k, x);
            return *this;
        }
        operator S() { return segtree.get(k); }
    };

protected:
    void construct_data() {
        sz = 1;
        height = 0;
        while (sz < n) {
            sz <<= 1;
            height++;
        }
        data.assign(sz * 2, e());
        lazy.assign(sz * 2, id());
    }
    void initialize(const std::vector<S>& v) {
        for (int i = 0; i < n; i++) data[sz + i] = v[i];
        for (int i = sz - 1; i > 0; i--) update(i);
    }

public:
    // Warning: 継承先のコンストラクタでconstruct_data()を必ず呼び出す！
    LazySegTreeBase(int n = 0) : n(n) {}

    /**
     * @brief 指定された要素の値を返す
     * 
     * @param k インデックス
     * @return S 値
     */
    S get(int k) {
        k += sz;
        for(int h = height; h > 0; h--) {
            push(k >> h, h);
        }
        return data[k];
    }
    /**
     * @brief 指定された要素への参照を返す
     * 
     * @param k 
     * @return SegTreeReference 要素への参照 代入されるとset()が呼ばれる
     */
    LazySegTreeReference operator[] (int k) { return LazySegTreeReference(*this, k); }

    /**
     * @brief 内容を出力する
     * 
     * @tparam CharT 出力ストリームの文字型
     * @tparam Traits 出力ストリームの文字型特性
     * @param os 出力ストリーム
     * @param rhs セグメント木
     * @return std::basic_ostream<CharT, Traits>& 出力ストリーム 
     */
    template <class CharT, class Traits>
    friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, LazySegTreeBase& rhs) {
        for(int i = 0; i < rhs.n; i++) {
            if(i != 0) os << CharT(' ');
            os << rhs[i];
        }
        return os;
    }

    /**
     * @brief 指定された要素の値をxに更新する
     * 
     * @param k インデックス
     * @param x 新しい値
     */
    void set(int k, const S& x) {
        k += sz;
        for(int h = height; h > 0; h--) {
            push(k >> h, h);
        }
        data[k] = x;
        while(k >>= 1) update(k);
    }

    /**
     * @brief [l, r)の区間の総積を返す
     * 
     * @param l 半開区間の開始
     * @param r 半開区間の終端
     * @return S 総積
     */
    S prod(int l, int r) {
        l += sz; r += sz;
        for(int h = height; h > 0; h--) {
            if(((l >> h) << h) != l) push(l >> h, h);
            if(((r >> h) << h) != r) push(r >> h, h);
        }
        S left_prod = e(), right_prod = e();
        while(l < r) {
            if(l & 1) left_prod = op(left_prod, data[l++]);
            if(r & 1) right_prod = op(data[--r], right_prod);
            l >>= 1; r >>= 1;
        }
        return op(left_prod, right_prod);
    }
    /**
     * @brief すべての要素の総積を返す
     * 
     * @return S 総積
     */
    S all_prod() const { return data[1]; }
    
    /**
     * @brief 指定された要素の値にxを作用させる
     * 
     * @param k インデックス
     * @param x 作用素
     */
    void apply(int k, const F& f) {
        k += sz;
        for(int h = height; h > 0; h--) {
            push(k >> h, h);
        }
        data[k] = mapping(f, data[k]);
        while(k >>= 1) update(k);
    }
    /**
     * @brief [l, r)の区間の値にxを作用させる
     * 
     * @param l 半開区間の開始
     * @param r 半開区間の終端
     * @param f 作用素
     */
    void apply(int l, int r, const F& f) {
        if(l == r) return;
        l += sz; r += sz;
        for(int h = height; h > 0; h--) {
            if(((l >> h) << h) != l) push(l >> h, h);
            if(((r >> h) << h) != r) push(r >> h, h);
        }
        {
            int l2 = l, r2 = r;
            int h = 0;
            while(l < r) {
                if(l & 1) apply_node(l++, h, f);
                if(r & 1) apply_node(--r, h, f);
                l >>= 1; r >>= 1;
                h++;
            }
            l = l2; r = r2;
        }
        for(int h = 1; h <= height; h++) {
            if(((l >> h) << h) != l) update(l >> h);
            if(((r >> h) << h) != r) update((r - 1) >> h);
        }
    }

    /**
     * @brief (r = l or g(prod([l, r))) = true) and (r = n or g(prod([l, r+1))) = false)となるrを返す
     * gが単調なら、g(prod([l, r))) = trueとなる最大のr
     * 
     * @tparam G
     * @param l 半開区間の開始
     * @param g 判定関数 g(e) = true
     * @return int
     */
    template <typename G>
    int max_right(int l, G g) {
        assert(g(e()));
        if(l == n) return n;
        l += sz;
        for(int h = height; h > 0; h--) {
            push(l >> h, h);
        }
        int h = 0;
        while(l % 2 == 0) {
            l >>= 1;
            h++;
        }
        S sum = e();
        while(g(op(sum, data[l]))) {
            sum = op(sum, data[l]);
            l++;
            while(l % 2 == 0) {
                l >>= 1;
                h++;
            }
            if(l == 1) return n;
        }
        while(l < sz) {
            push(l, h);
            if(!g(op(sum, data[l*2]))) {
                l = l*2;
            } else {
                sum = op(sum, data[l*2]);
                l = l*2+1;
            }
            h--;
        }
        return l - sz;
    }
    /**
     * @brief (l = 0 or g(prod([l, r))) = true) and (l = r or g(prod([l-1, r))) = false)となるlを返す
     * gが単調なら、g(prod([l, r))) = trueとなる最小のl
     * 
     * @tparam G
     * @param r 半開区間の終端
     * @param g 判定関数 g(e) = true
     * @return int
     */
    template <typename G>
    int min_left(int r, G g) {
        assert(g(e()));
        if (r == 0) return 0;
        r += sz;
        for(int h = height; h > 0; h--) {
            push(r >> h, h);
        }
        int h = 0;
        while(r % 2 == 0) {
            r >>= 1;
            h++;
        }
        S sum = e();
        while(g(op(data[r-1], sum))) {
            sum = op(data[r-1], sum);
            r--;
            while(r % 2 == 0) {
                r >>= 1;
                h++;
            }
            if(r == 1) return 0;
        }
        while(r < sz) {
            push(r - 1, h);
            if(!g(op(data[r*2-1], sum))) r *= 2;
            else {
                sum = op(data[r*2-1], sum);
                r = r*2 - 1;
            }
            h--;
        }
        return r - sz;
    }
};

/**
 * @brief ファンクタが静的な場合の遅延伝搬セグメント木の実装
 * 
 * @tparam S 値モノイドの型
 * @tparam Op 値の積のファンクタ
 * @tparam E 積の単位元を返すファンクタ
 * @tparam F 作用素モノイドの型
 * @tparam Mapping 作用素を適用するファンクタ 引数は(作用素, 値)または(作用素, 値, サイズ)(作用素, 値, 左の子, 右の子)
 * @tparam Composition 作用素の積のファンクタ
 * @tparam ID 作用素の単位元を返すファンクタ
 */
template <typename S, typename Op, typename E, typename F, typename Mapping, typename Composition, typename ID>
class StaticLazySegTree : public LazySegTreeBase<S, F, StaticLazySegTree<S, Op, E, F, Mapping, Composition, ID>> {
    using BaseType = LazySegTreeBase<S, F, StaticLazySegTree<S, Op, E, F, Mapping, Composition, ID>>;

    inline static Op operator_object;
    inline static E identity_object;
    inline static Mapping mapping_object;
    inline static Composition lazy_operator_object;
    inline static ID lazy_identity_object;
public:
    S op(const S& a, const S& b) const { return operator_object(a, b); }
    S e() const { return identity_object(); }
    S mapping(const F& f, const S& x, int l, int r) const {
        if constexpr(std::is_invocable_v<Mapping, F, S, int, int>) {
            return mapping_object(f, x, l, r);
        } else if constexpr(std::is_invocable_v<Mapping, F, S, int>) {
            return mapping_object(f, x, r - l);
        } else {
            return mapping_object(f, x);
        }
    }
    F composition(const F& f, const F& g) const {
        return lazy_operator_object(f, g);
    }
    F id() const { return lazy_identity_object(); }

    /**
     * @brief デフォルトコンストラクタ
     * 
    */
    StaticLazySegTree() = default;
    /**
     * @brief コンストラクタ
     * 
     * @param n 要素数
     */
    explicit StaticLazySegTree(int n) : BaseType(n) {
        this->construct_data();
    }
    /**
     * @brief コンストラクタ
     * 
     * @param v 初期の要素
     */
    explicit StaticLazySegTree(const std::vector<S>& v) : StaticLazySegTree(v.size()) {
        this->initialize(v);
    }
};

/**
 * @brief コンストラクタで関数オブジェクトを与えることで積を定義する遅延伝搬セグメント木の実装
 * テンプレート引数を省略することができ、ラムダ式が使える
 * 
 * @tparam S モノイドの型
 * @tparam Op 積の関数オブジェクトの型
 * @tparam F 作用素モノイドの型
 * @tparam Mapping 作用素を適用する関数オブジェクトの型
 * @tparam Composition 作用素の積の関数オブジェクトの型
 */
template <typename S, typename Op, typename F, typename Mapping, typename Composition>
class LazySegTree : public LazySegTreeBase<S, F, LazySegTree<S, Op, F, Mapping, Composition>> {
    using BaseType = LazySegTreeBase<S, F, LazySegTree<S, Op, F, Mapping, Composition>>;

    Op operator_object;
    S identity;
    Mapping mapping_object;
    Composition lazy_operator_object;
    F lazy_identity;

public:
    S op(const S& a, const S& b) const { return operator_object(a, b); }
    S e() const { return identity; }
    S mapping(const F& f, const S& x, int l, int r) const {
        if constexpr(std::is_invocable_v<Mapping, F, S, int, int>) {
            return mapping_object(f, x, l, r);
        } else if constexpr(std::is_invocable_v<Mapping, F, S, int>) {
            return mapping_object(f, x, r - l);
        } else {
            return mapping_object(f, x);
        }
    }
    F composition(const F& f, const F& g) const {
        return lazy_operator_object(f, g);
    }
    F id() const { return lazy_identity; }

    /**
     * @brief デフォルトコンストラクタ
    */
    LazySegTree() = default;
    /**
     * @brief コンストラクタ
     * 
     * @param n 要素数
     * @param op 積の関数オブジェクト
     * @param identity 単位元
     * @param mapping 作用素を適用する関数オブジェクト
     * @param composition 作用素の積の関数オブジェクト
     * @param lazy_identity 作用素の単位元
     */
    explicit LazySegTree(int n, Op op, const S& identity, Mapping mapping, Composition composition, const F& lazy_identity)
        : BaseType(n), operator_object(std::move(op)), identity(identity), mapping_object(std::move(mapping)),
        lazy_operator_object(std::move(composition)), lazy_identity(lazy_identity) {
        this->construct_data();
    }
    /**
     * @brief コンストラクタ
     * 
     * @param v 初期の要素
     * @param op 積の関数オブジェクト
     * @param identity 単位元
     * @param mapping 作用素を適用する関数オブジェクト
     * @param composition 作用素の積の関数オブジェクト
     * @param lazy_identity 作用素の単位元
     */
    explicit LazySegTree(const std::vector<S>& v, Op op, const S& identity, Mapping mapping, Composition composition, const F& lazy_identity)
        : LazySegTree(v.size(), std::move(op), identity, std::move(mapping), std::move(composition), lazy_identity) {
        this->initialize(v);
    }
};

namespace lazy_segtree {
    template <typename S>
    struct Max {
        const S operator() (const S& a, const S& b) const { return std::max(a, b); }
    };
    template <typename S>
    struct Min {
        const S operator() (const S& a, const S& b) const { return std::min(a, b); }
    };
    template <typename S, std::enable_if_t<std::is_scalar_v<S>>* = nullptr>
    struct MaxLimit {
        constexpr S operator() () const { return std::numeric_limits<S>::max(); }
    };
    template <typename S, std::enable_if_t<std::is_scalar_v<S>>* = nullptr>
    struct MinLimit {
        constexpr S operator() () const { return std::numeric_limits<S>::lowest(); }
    };
    template <typename S>
    struct AddWithSize {
        S operator() (const S& f, const S& x, int sz) const { return x + f * sz; }
    };
    template <typename S>
    struct Zero {
        S operator() () const { return S(0); }
    };
    template <typename S, S ID>
    struct Update {
        S operator() (const S& f, const S& x) const { return f == ID ? x : f; }
    };
    template <typename S, S ID>
    struct UpdateWithSize {
        S operator() (const S& f, const S& x, int sz) const { return f == ID ? x : f * sz; }
    };
    template <typename S, S ID>
    struct UpdateComposition {
        S operator() (const S& f, const S& g) const { return f == ID ? g : f; }
    };
}

/**
 * @brief 区間最小値更新、区間最小値
 * 
 * @tparam S 型
 */
template <typename S>
using RChminMinQ = StaticLazySegTree<
    S,
    lazy_segtree::Min<S>,
    lazy_segtree::MaxLimit<S>,
    S,
    lazy_segtree::Min<S>,
    lazy_segtree::Min<S>,
    lazy_segtree::MaxLimit<S>
>;
/**
 * @brief 区間最大値更新、区間最大値
 * 
 * @tparam S 型
 */
template <typename S>
using RChmaxMaxQ = StaticLazySegTree<
    S,
    lazy_segtree::Max<S>,
    lazy_segtree::MinLimit<S>,
    S, // F
    lazy_segtree::Max<S>,
    lazy_segtree::Max<S>,
    lazy_segtree::MinLimit<S>
>;
/**
 * @brief 区間加算、区間最小値
 * 
 * @tparam S 型
 */
template <typename S>
using RAddMinQ = StaticLazySegTree<
    S, // S
    lazy_segtree::Min<S>,
    lazy_segtree::MaxLimit<S>,
    S,
    std::plus<S>,
    std::plus<S>,
    lazy_segtree::Zero<S>
>;
/**
 * @brief 区間加算、区間最大値
 * 
 * @tparam S 型
 */
template <typename S>
using RAddMaxQ = StaticLazySegTree<
    S,
    lazy_segtree::Max<S>,
    lazy_segtree::MinLimit<S>,
    S,
    std::plus<S>,
    std::plus<S>,
    lazy_segtree::Zero<S>
>;
/**
 * @brief 区間加算、区間和
 * 
 * @tparam S 型
 */
template <typename S>
using RAddSumQ = StaticLazySegTree<
    S,
    std::plus<S>,
    lazy_segtree::Zero<S>,
    S,
    lazy_segtree::AddWithSize<S>,
    std::plus<S>,
    lazy_segtree::Zero<S>
>;
/**
 * @brief 区間変更、区間最小値
 * 注意: numeric_limits<S>::max()での更新がないことをが要件
 * 
 * @tparam S 型
 */
template <typename S>
using RUpdateMinQ = StaticLazySegTree<
    S,
    lazy_segtree::Min<S>,
    lazy_segtree::MaxLimit<S>,
    S,
    lazy_segtree::Update<S, lazy_segtree::MaxLimit<S>{}()>,
    lazy_segtree::UpdateComposition<S, lazy_segtree::MaxLimit<S>{}()>,
    lazy_segtree::MaxLimit<S>
>;
/**
 * @brief 区間変更、区間最大値
 * 注意: numeric_limits<S>::max()での更新がないことをが要件
 * 
 * @tparam S 型
 */
template <typename S>
using RUpdateMaxQ = StaticLazySegTree<
    S,
    lazy_segtree::Max<S>,
    lazy_segtree::MinLimit<S>,
    S,
    lazy_segtree::Update<S, lazy_segtree::MaxLimit<S>{}()>,
    lazy_segtree::UpdateComposition<S, lazy_segtree::MaxLimit<S>{}()>,
    lazy_segtree::MaxLimit<S>
>;
/**
 * @brief 区間変更、区間和
 * 注意: numeric_limits<S>::max()での更新がないことをが要件
 * 
 * @tparam S 型
 */
template <typename S>
using RUpdateSumQ = StaticLazySegTree<
    S,
    std::plus<S>,
    lazy_segtree::Zero<S>,
    S,
    lazy_segtree::UpdateWithSize<S, lazy_segtree::MaxLimit<S>{}()>,
    lazy_segtree::UpdateComposition<S, lazy_segtree::MaxLimit<S>{}()>,
    lazy_segtree::MaxLimit<S>
>