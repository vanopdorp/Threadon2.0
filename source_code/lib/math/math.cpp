#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <limits>
#include <numeric>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>
#include <complex>
#include <tuple>

namespace math {

// =========================
// Constants en utilities
// =========================
namespace consts {
    template <class T> constexpr T pi  = T(3.141592653589793238462643383279502884L);
    template <class T> constexpr T tau = T(6.283185307179586476925286766559005768L);
    template <class T> constexpr T e   = T(2.718281828459045235360287471352662498L);
    template <class T> constexpr T phi = T(1.618033988749894848204586834365638118L);
    template <class T> constexpr T sqrt2 = T(1.414213562373095048801688724209698079L);
}

template <class T>
constexpr T eps() { return std::numeric_limits<T>::epsilon(); }

template <class T>
constexpr bool is_finite(T x) { return std::isfinite(static_cast<long double>(x)); }

template <class T>
constexpr bool almost_equal(T a, T b, T rel = T(1e-12), T abs = T(0)) {
    T diff = std::abs(a - b);
    if (diff <= abs) return true;
    return diff <= rel * std::max(T(1), std::max(std::abs(a), std::abs(b)));
}

template <class T>
constexpr T clamp(T v, T lo, T hi) {
    return std::max(lo, std::min(hi, v));
}

template <class T>
constexpr T lerp(T a, T b, T t) { return a + (b - a) * t; }

template <class T>
constexpr T map_range(T x, T in_min, T in_max, T out_min, T out_max) {
    return out_min + (x - in_min) * (out_max - out_min) / (in_max - in_min);
}

// =========================
// Number theory / integers
// =========================
template <class I, class = std::enable_if_t<std::is_integral<I>::value>>
I gcd(I a, I b) {
    if (a < 0) a = -a; if (b < 0) b = -b;
    while (b != 0) { I t = a % b; a = b; b = t; }
    return a;
}

template <class I, class = std::enable_if_t<std::is_integral<I>::value>>
I lcm(I a, I b) {
    if (a == 0 || b == 0) return 0;
    return (a / gcd(a,b)) * b;
}

template <class I>
std::tuple<I,I,I> ext_gcd(I a, I b) {
    // returns (g, x, y) s.t. ax + by = g = gcd(a,b)
    I x0 = 1, y0 = 0, x1 = 0, y1 = 1;
    while (b) {
        I q = a / b;
        I a2 = a - q*b; a = b; b = a2;
        I x2 = x0 - q*x1; x0 = x1; x1 = x2;
        I y2 = y0 - q*y1; y0 = y1; y1 = y2;
    }
    return {a, x0, y0};
}

template <class I, class = std::enable_if_t<std::is_integral<I>::value>>
I mod_pow(I base, I exp, I mod) {
    if (mod <= 0) throw std::domain_error("mod must be positive");
    base %= mod; if (base < 0) base += mod;
    I res = 1 % mod;
    while (exp > 0) {
        if (exp & 1) res = ( (__int128)res * base ) % mod;
        base = ( (__int128)base * base ) % mod;
        exp >>= 1;
    }
    return res;
}

template <class I, class = std::enable_if_t<std::is_integral<I>::value>>
I mod_inv(I a, I mod) {
    auto [g, x, y] = ext_gcd(a, mod);
    if (g != 1 && g != -1) throw std::domain_error("mod inverse does not exist");
    I r = x % mod;
    if (r < 0) r += mod;
    return r;
}

// Miller-Rabin deterministic for 64-bit
inline bool is_probable_prime(uint64_t n) {
    if (n < 2) return false;
    for (uint64_t p : {2ULL,3ULL,5ULL,7ULL,11ULL,13ULL,17ULL,19ULL,23ULL,29ULL,31ULL,37ULL}) {
        if (n%p==0) return n==p;
    }
    auto decompose = [](uint64_t n)->std::pair<uint64_t,uint64_t>{
        uint64_t d = n-1, s=0; while ((d & 1) == 0) { d >>= 1; ++s; } return {d,s};
    };
    auto [d, s] = decompose(n);
    auto check = [&](uint64_t a)->bool{
        __int128 x = 1, p = a % n, e = d;
        while (e) { if (e&1) x = (x*p) % n; p = (p*p) % n; e >>= 1; }
        if (x==1 || x==n-1) return true;
        for (uint64_t i=1;i<s;i++){ x=(x*x)%n; if (x==n-1) return true; }
        return false;
    };
    // Deterministic bases for 64-bit
    for (uint64_t a : {2ULL, 325ULL, 9375ULL, 28178ULL, 450775ULL, 9780504ULL, 1795265022ULL})
        if (a % n && !check(a % n)) return false;
    return true;
}

constexpr unsigned long long factorial(unsigned n) {
    return (n <= 1) ? 1ULL : (n * factorial(n - 1));
}

inline unsigned long long binom(unsigned n, unsigned k) {
    if (k > n) return 0ULL;
    if (k == 0 || k == n) return 1ULL;
    if (k > n - k) k = n - k;
    unsigned long long res = 1;
    for (unsigned i = 1; i <= k; ++i) {
        res = (res * (n - k + i)) / i;
    }
    return res;
}

inline unsigned long long catalan(unsigned n) {
    // Cn = binom(2n, n)/(n+1)
    return binom(2*n, n) / (n + 1);
}

// =========================
// Vectors & Matrices
// =========================
struct Vec2 {
    double x=0, y=0;
    Vec2() = default;
    Vec2(double x_, double y_) : x(x_), y(y_) {}
    double& operator[](size_t i){ return i==0?x:y; }
    double  operator[](size_t i) const { return i==0?x:y; }
    Vec2 operator+(const Vec2& o) const { return {x+o.x, y+o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x-o.x, y-o.y}; }
    Vec2 operator*(double s) const { return {x*s, y*s}; }
    Vec2 operator/(double s) const { return {x/s, y/s}; }
    Vec2& operator+=(const Vec2& o){ x+=o.x; y+=o.y; return *this; }
    Vec2& operator-=(const Vec2& o){ x-=o.x; y-=o.y; return *this; }
};
inline double dot(const Vec2& a, const Vec2& b){ return a.x*b.x + a.y*b.y; }
inline double norm2(const Vec2& v){ return dot(v,v); }
inline double norm(const Vec2& v){ return std::sqrt(norm2(v)); }
inline Vec2 normalize(const Vec2& v){ double n=norm(v); return n? v*(1.0/n) : v; }

struct Vec3 {
    double x=0, y=0, z=0;
    Vec3() = default;
    Vec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
    double& operator[](size_t i){ return i==0?x:(i==1?y:z); }
    double  operator[](size_t i) const { return i==0?x:(i==1?y:z); }
    Vec3 operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3 operator*(double s) const { return {x*s, y*s, z*s}; }
    Vec3 operator/(double s) const { return {x/s, y/s, z/s}; }
    Vec3& operator+=(const Vec3& o){ x+=o.x; y+=o.y; z+=o.z; return *this; }
    Vec3& operator-=(const Vec3& o){ x-=o.x; y-=o.y; z-=o.z; return *this; }
};
inline double dot(const Vec3& a, const Vec3& b){ return a.x*b.x + a.y*b.y + a.z*b.z; }
inline Vec3 cross(const Vec3& a, const Vec3& b){
    return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
}
inline double norm2(const Vec3& v){ return dot(v,v); }
inline double norm(const Vec3& v){ return std::sqrt(norm2(v)); }
inline Vec3 normalize(const Vec3& v){ double n=norm(v); return n? v*(1.0/n) : v; }

// Dynamische vector
struct Vec {
    std::vector<double> d;
    Vec() = default;
    explicit Vec(size_t n, double v = 0) : d(n, v) {}
    size_t size() const { return d.size(); }
    double& operator[](size_t i){ return d[i]; }
    double  operator[](size_t i) const { return d[i]; }
    Vec& operator+=(const Vec& o){ assert(size()==o.size()); for(size_t i=0;i<size();++i) d[i]+=o[i]; return *this; }
    Vec& operator-=(const Vec& o){ assert(size()==o.size()); for(size_t i=0;i<size();++i) d[i]-=o[i]; return *this; }
};
inline Vec operator+(Vec a, const Vec& b){ a+=b; return a; }
inline Vec operator-(Vec a, const Vec& b){ a-=b; return a; }
inline Vec operator*(const Vec& a, double s){ Vec r=a; for(double& v:r.d) v*=s; return r; }
inline Vec operator*(double s, const Vec& a){ return a*s; }
inline double dot(const Vec& a, const Vec& b){ assert(a.size()==b.size()); double s=0; for(size_t i=0;i<a.size();++i) s+=a[i]*b[i]; return s; }
inline double norm2(const Vec& v){ return dot(v,v); }
inline double norm(const Vec& v){ return std::sqrt(norm2(v)); }

// Dynamische matrix
struct Mat {
    size_t r=0, c=0;
    std::vector<double> a; // row-major
    Mat() = default;
    Mat(size_t r_, size_t c_, double v = 0) : r(r_), c(c_), a(r_*c_, v) {}
    static Mat identity(size_t n){ Mat I(n,n); for(size_t i=0;i<n;++i) I(i,i)=1; return I; }
    double& operator()(size_t i, size_t j){ return a[i*c + j]; }
    double  operator()(size_t i, size_t j) const { return a[i*c + j]; }
    Mat T() const { Mat t(c,r); for(size_t i=0;i<r;++i) for(size_t j=0;j<c;++j) t(j,i)=(*this)(i,j); return t; }
    size_t rows() const { return r; }
    size_t cols() const { return c; }
};

inline Mat operator+(const Mat& A, const Mat& B){
    assert(A.r==B.r && A.c==B.c);
    Mat C(A.r,A.c);
    for (size_t i=0;i<A.a.size();++i) C.a[i]=A.a[i]+B.a[i];
    return C;
}
inline Mat operator-(const Mat& A, const Mat& B){
    assert(A.r==B.r && A.c==B.c);
    Mat C(A.r,A.c);
    for (size_t i=0;i<A.a.size();++i) C.a[i]=A.a[i]-B.a[i];
    return C;
}
inline Mat operator*(const Mat& A, const Mat& B){
    assert(A.c==B.r);
    Mat C(A.r, B.c, 0);
    for (size_t i=0;i<A.r;++i)
        for (size_t k=0;k<A.c;++k){
            double aik = A(i,k);
            for (size_t j=0;j<B.c;++j)
                C(i,j) += aik * B(k,j);
        }
    return C;
}
inline Vec operator*(const Mat& A, const Vec& x){
    assert(A.c == x.size());
    Vec y(A.r, 0);
    for (size_t i=0;i<A.r;++i){
        double s=0;
        for (size_t j=0;j<A.c;++j) s += A(i,j)*x[j];
        y[i]=s;
    }
    return y;
}

// Gauss-elimination met partiële pivot
inline Vec solve(Mat A, Vec b){
    size_t n = A.rows();
    assert(A.cols()==n && b.size()==n);
    for (size_t k=0;k<n;++k){
        // pivot
        size_t piv = k;
        double best = std::abs(A(k,k));
        for (size_t i=k+1;i<n;++i){
            double v = std::abs(A(i,k));
            if (v > best){ best=v; piv=i; }
        }
        if (best == 0.0) throw std::runtime_error("singular matrix");
        if (piv != k){
            for (size_t j=k;j<n;++j) std::swap(A(k,j), A(piv,j));
            std::swap(b[k], b[piv]);
        }
        // eliminate
        double akk = A(k,k);
        for (size_t i=k+1;i<n;++i){
            double f = A(i,k)/akk;
            if (f==0) continue;
            A(i,k)=0;
            for (size_t j=k+1;j<n;++j) A(i,j) -= f*A(k,j);
            b[i] -= f*b[k];
        }
    }
    // back-substitution
    Vec x(n,0);
    for (int i=int(n)-1;i>=0;--i){
        double s=b[i];
        for (size_t j=i+1;j<n;++j) s -= A(i,j)*x[j];
        x[i]= s / A(i,i);
    }
    return x;
}

inline double determinant(Mat A){
    size_t n = A.rows();
    assert(A.cols()==n);
    double det = 1;
    int sign = 1;
    for (size_t k=0;k<n;++k){
        size_t piv=k; double best=std::abs(A(k,k));
        for (size_t i=k+1;i<n;++i){ double v=std::abs(A(i,k)); if (v>best){best=v;piv=i;} }
        if (best==0.0) return 0.0;
        if (piv!=k){ for (size_t j=k;j<n;++j) std::swap(A(k,j),A(piv,j)); sign = -sign; }
        det *= A(k,k);
        double akk = A(k,k);
        for (size_t i=k+1;i<n;++i){
            double f = A(i,k)/akk;
            for (size_t j=k+1;j<n;++j) A(i,j) -= f*A(k,j);
        }
    }
    return sign * det;
}

inline Mat inverse(Mat A){
    size_t n = A.rows();
    assert(A.cols()==n);
    Mat I = Mat::identity(n);
    // Augmented Gauss-Jordan
    for (size_t k=0;k<n;++k){
        size_t piv=k; double best=std::abs(A(k,k));
        for (size_t i=k+1;i<n;++i){ double v=std::abs(A(i,k)); if (v>best){best=v;piv=i;} }
        if (best==0.0) throw std::runtime_error("singular matrix");
        if (piv!=k){
            for (size_t j=0;j<n;++j){ std::swap(A(k,j),A(piv,j)); std::swap(I(k,j),I(piv,j)); }
        }
        double akk=A(k,k);
        for (size_t j=0;j<n;++j){ A(k,j)/=akk; I(k,j)/=akk; }
        for (size_t i=0;i<n;++i){
            if (i==k) continue;
            double f=A(i,k);
            if (f==0) continue;
            for (size_t j=0;j<n;++j){
                A(i,j) -= f*A(k,j);
                I(i,j) -= f*I(k,j);
            }
        }
    }
    return I;
}

// =========================
// Numerieke calculus
// =========================
template <class F, class T>
T bisection(F f, T a, T b, T tol = T(1e-12), int maxit = 200) {
    T fa = f(a), fb = f(b);
    if (fa==0) return a; if (fb==0) return b;
    if (fa * fb > 0) throw std::domain_error("bisection: f(a) and f(b) have same sign");
    for (int it=0; it<maxit; ++it) {
        T m = (a + b) / 2;
        T fm = f(m);
        if (std::abs(fm) < tol || std::abs(b-a) < tol) return m;
        if (fa * fm <= 0) { b=m; fb=fm; } else { a=m; fa=fm; }
    }
    return (a+b)/2;
}

template <class F, class T>
T newton(F f, F df, T x0, T tol = T(1e-12), int maxit = 100) {
    T x = x0;
    for (int it=0; it<maxit; ++it) {
        T fx = f(x);
        T dfx = df(x);
        if (dfx == 0) break;
        T x1 = x - fx/dfx;
        if (std::abs(x1 - x) < tol) return x1;
        x = x1;
    }
    return x;
}

template <class F, class T>
T derivative_central(F f, T x, T h = T(1e-5)) {
    return (f(x + h) - f(x - h)) / (2*h);
}

template <class F, class T>
T derivative_ridder(F f, T x, T h = T(0.1), T tol = T(1e-12)) {
    // Ridder's method for derivative
    const int MAX = 10;
    std::vector<std::vector<T>> D(MAX, std::vector<T>(MAX));
    for (int i=0; i<MAX; ++i) {
        T hi = h / std::pow(2, i);
        D[i][0] = (f(x + hi) - f(x - hi)) / (2*hi);
        T p4 = 4;
        for (int j=1; j<=i; ++j) {
            D[i][j] = (p4*D[i][j-1] - D[i-1][j-1]) / (p4 - 1);
            p4 *= 4;
        }
        if (i>0 && std::abs(D[i][i] - D[i-1][i-1]) < tol) return D[i][i];
    }
    return D.back().back();
}

template <class F, class T>
T adaptive_simpson(F f, T a, T b, T eps = T(1e-10), int max_rec = 20) {
    auto simpson = [&](T a, T b)->T {
        T c = (a + b)/2;
        return (b - a) * (f(a) + 4*f(c) + f(b)) / 6;
    };
    std::function<T(T,T,T,T,int)> asr = [&](T a, T b, T eps, T whole, int rec)->T{
        T c = (a + b)/2;
        T left = simpson(a,c);
        T right = simpson(c,b);
        T delta = left + right - whole;
        if (rec <= 0 || std::abs(delta) <= 15*eps)
            return left + right + delta/15;
        return asr(a,c,eps/2,left,rec-1) + asr(c,b,eps/2,right,rec-1);
    };
    return asr(a,b,eps,simpson(a,b),max_rec);
}

template <class F, class T>
std::vector<T> rk4(F f, T t0, T y0, T t1, int steps) {
    // y' = f(t,y), scalar RK4
    T h = (t1 - t0)/steps;
    std::vector<T> y(steps+1);
    T t = t0, v = y0;
    y[0] = v;
    for (int i=1;i<=steps;++i){
        T k1 = f(t, v);
        T k2 = f(t + h/2, v + h*k1/2);
        T k3 = f(t + h/2, v + h*k2/2);
        T k4 = f(t + h,   v + h*k3);
        v += (h/6)*(k1 + 2*k2 + 2*k3 + k4);
        t += h;
        y[i] = v;
    }
    return y;
}

// =========================
// Optimalisatie (1D)
// =========================
template <class F, class T>
T golden_section(F f, T a, T b, T tol = T(1e-10), int maxit = 200, bool minimize = true) {
    const T gr = (T(1) + std::sqrt(T(5))) / T(2); // golden ratio
    T c = b - (b - a) / gr;
    T d = a + (b - a) / gr;
    T fc = f(c), fd = f(d);
    auto better = [&](T x, T y){ return minimize ? x < y : x > y; };
    for (int it=0; it<maxit && std::abs(b - a) > tol; ++it) {
        if (better(fc, fd)) { b = d; d = c; fd = fc; c = b - (b - a)/gr; fc = f(c); }
        else { a = c; c = d; fc = fd; d = a + (b - a)/gr; fd = f(d); }
    }
    return (a + b)/2;
}

// =========================
// Polynomen
// =========================
template <class T>
T poly_eval(const std::vector<T>& c, T x) {
    // c[0] + c[1] x + ... + c[n] x^n
    T y = 0;
    for (int i=int(c.size())-1; i>=0; --i) y = y * x + c[size_t(i)];
    return y;
}

template <class T>
std::vector<T> poly_derivative(const std::vector<T>& c) {
    std::vector<T> d;
    if (c.size() <= 1) return d;
    d.resize(c.size()-1);
    for (size_t i=1;i<c.size();++i) d[i-1] = c[i]*T(i);
    return d;
}

template <class T>
std::vector<T> poly_integral(const std::vector<T>& c, T constant = T(0)) {
    std::vector<T> I(c.size()+1);
    I[0] = constant;
    for (size_t i=0;i<c.size();++i) I[i+1] = c[i] / T(i+1);
    return I;
}

template <class T>
std::vector<T> poly_add(const std::vector<T>& a, const std::vector<T>& b) {
    std::vector<T> c(std::max(a.size(), b.size()), T(0));
    for (size_t i=0;i<a.size();++i) c[i] += a[i];
    for (size_t i=0;i<b.size();++i) c[i] += b[i];
    return c;
}

template <class T>
std::vector<T> poly_mul(const std::vector<T>& a, const std::vector<T>& b) {
    std::vector<T> c(a.size()+b.size()-1, T(0));
    for (size_t i=0;i<a.size();++i)
        for (size_t j=0;j<b.size();++j)
            c[i+j] += a[i]*b[j];
    return c;
}

// Reële wortels voor kwadratisch en kubisch
template <class T>
std::vector<T> roots_quadratic(T a, T b, T c) {
    if (a == 0) {
        if (b == 0) return {};
        return { -c / b };
    }
    T disc = b*b - 4*a*c;
    if (disc < 0) return {};
    T s = std::sqrt(disc);
    T x1 = (-b - std::copysign(s, b)) / (2*a); // numerically stable
    T x2 = c / (a * x1);
    if (almost_equal(x1, x2, T(1e-14))) return {x1};
    if (x1 < x2) return {x1, x2};
    return {x2, x1};
}

template <class T>
std::vector<T> roots_cubic(T a, T b, T c, T d) {
    if (a == 0) return roots_quadratic(b,c,d);
    // Depressed cubic t^3 + pt + q = 0
    T A = b/a, B = c/a, C = d/a;
    T A2 = A*A;
    T p = B - A2/3;
    T q = (2*A2*A)/27 - (A*B)/3 + C;
    T disc = (q*q)/4 + (p*p*p)/27;
    std::vector<T> roots;
    if (disc > 0) {
        T s = std::sqrt(disc);
        T u = std::cbrt(-q/2 + s);
        T v = std::cbrt(-q/2 - s);
        roots.push_back(u + v - A/3);
    } else if (almost_equal(disc, T(0), T(1e-16))) {
        T u = std::cbrt(-q/2);
        roots.push_back(2*u - A/3);
        roots.push_back(-u - A/3);
    } else {
        T r = std::sqrt(-p*p*p/27);
        T phi = std::acos(-q/(2*std::sqrt(-p*p*p/27)));
        T t = 2*std::cbrt(std::sqrt(-p/3));
        roots.push_back(t*std::cos(phi/3) - A/3);
        roots.push_back(t*std::cos((phi + 2*consts::pi<T>)/3) - A/3);
        roots.push_back(t*std::cos((phi + 4*consts::pi<T>)/3) - A/3);
        std::sort(roots.begin(), roots.end());
    }
    return roots;
}

// =========================
// Statistiek
// =========================
template <class T>
T sum(const std::vector<T>& x){ T s=0; for(const auto& v:x) s+=v; return s; }

template <class T>
T mean(const std::vector<T>& x){ return x.empty()? T(0) : sum(x)/T(x.size()); }

template <class T>
T variance(const std::vector<T>& x, bool sample = true) {
    if (x.size() < 2) return T(0);
    T m = mean(x); T s=0;
    for (const auto& v:x) { T d=v-m; s+=d*d; }
    return s / T(x.size() - (sample?1:0));
}

template <class T>
T stddev(const std::vector<T>& x, bool sample = true){
    return std::sqrt(variance(x, sample));
}

template <class T>
T median(std::vector<T> x) {
    if (x.empty()) return T(0);
    size_t n = x.size();
    size_t mid = n/2;
    std::nth_element(x.begin(), x.begin()+mid, x.end());
    if (n%2) return x[mid];
    auto m1 = *std::max_element(x.begin(), x.begin()+mid);
    return (m1 + x[mid])/T(2);
}

template <class T>
T percentile(std::vector<T> x, double p) {
    if (x.empty()) return T(0);
    if (p<=0) return *std::min_element(x.begin(), x.end());
    if (p>=100) return *std::max_element(x.begin(), x.end());
    double pos = (p/100.0) * (x.size()-1);
    size_t i = size_t(pos);
    double frac = pos - i;
    std::nth_element(x.begin(), x.begin()+i, x.end());
    T a = x[i];
    if (frac == 0) return a;
    std::nth_element(x.begin()+i+1, x.begin()+i+1, x.end());
    T b = *std::min_element(x.begin()+i+1, x.end());
    return a + (b - a) * T(frac);
}

template <class T>
T cov(const std::vector<T>& x, const std::vector<T>& y, bool sample = true) {
    assert(x.size()==y.size() && x.size()>=2);
    T mx=mean(x), my=mean(y), s=0;
    for (size_t i=0;i<x.size();++i) s += (x[i]-mx)*(y[i]-my);
    return s / T(x.size() - (sample?1:0));
}

template <class T>
T corr(const std::vector<T>& x, const std::vector<T>& y) {
    return cov(x,y) / (stddev(x) * stddev(y));
}

template <class T>
std::pair<T,T> linear_regression(const std::vector<T>& x, const std::vector<T>& y) {
    assert(x.size()==y.size() && !x.empty());
    T sx=sum(x), sy=sum(y), sxx=0, sxy=0;
    for (size_t i=0;i<x.size();++i){ sxx+=x[i]*x[i]; sxy+=x[i]*y[i]; }
    T n = T(x.size());
    T denom = n*sxx - sx*sx;
    if (denom == 0) throw std::runtime_error("singular regression");
    T slope = (n*sxy - sx*sy)/denom;
    T intercept = (sy - slope*sx)/n;
    return {slope, intercept};
}

// =========================
// Geometrie 2D
// =========================
struct Point { double x=0, y=0; };
inline double cross(const Point& a, const Point& b, const Point& c){
    return (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
}
inline double dist2(const Point& a, const Point& b){
    double dx=a.x-b.x, dy=a.y-b.y; return dx*dx+dy*dy;
}

inline bool seg_intersect(Point p1, Point p2, Point q1, Point q2) {
    auto orient = [&](const Point& a, const Point& b, const Point& c){
        double v = cross(a,b,c);
        if (std::abs(v) < 1e-12) return 0;
        return v > 0 ? 1 : -1;
    };
    auto onseg = [&](const Point& a, const Point& b, const Point& p){
        return std::min(a.x,b.x)-1e-12 <= p.x && p.x <= std::max(a.x,b.x)+1e-12 &&
               std::min(a.y,b.y)-1e-12 <= p.y && p.y <= std::max(a.y,b.y)+1e-12 &&
               std::abs(cross(a,b,p)) < 1e-12;
    };
    int o1 = orient(p1,p2,q1), o2 = orient(p1,p2,q2), o3 = orient(q1,q2,p1), o4 = orient(q1,q2,p2);
    if (o1!=o2 && o3!=o4) return true;
    if (o1==0 && onseg(p1,p2,q1)) return true;
    if (o2==0 && onseg(p1,p2,q2)) return true;
    if (o3==0 && onseg(q1,q2,p1)) return true;
    if (o4==0 && onseg(q1,q2,p2)) return true;
    return false;
}

inline std::vector<Point> convex_hull(std::vector<Point> pts) {
    if (pts.size() <= 1) return pts;
    std::sort(pts.begin(), pts.end(), [](auto& a, auto& b){ return a.x<b.x || (a.x==b.x && a.y<b.y); });
    std::vector<Point> lower, upper;
    for (const auto& p : pts){
        while (lower.size()>=2 && cross(lower[lower.size()-2], lower.back(), p) <= 0) lower.pop_back();
        lower.push_back(p);
    }
    for (int i=int(pts.size())-1; i>=0; --i) {
        const auto& p = pts[size_t(i)];
        while (upper.size()>=2 && cross(upper[upper.size()-2], upper.back(), p) <= 0) upper.pop_back();
        upper.push_back(p);
    }
    lower.pop_back(); upper.pop_back();
    lower.insert(lower.end(), upper.begin(), upper.end());
    return lower;
}

inline double polygon_area(const std::vector<Point>& P) {
    long double s = 0;
    for (size_t i=0, n=P.size(); i<n; ++i) {
        const auto& a = P[i];
        const auto& b = P[(i+1)%n];
        s += (long double)a.x * b.y - (long double)a.y * b.x;
    }
    return double(std::abs(s) / 2.0L);
}

inline bool point_in_polygon(const std::vector<Point>& poly, Point p) {
    bool inside = false;
    for (size_t i=0, j=poly.size()-1; i<poly.size(); j=i++) {
        const auto& a = poly[i], &b = poly[j];
        bool intersect = ((a.y > p.y) != (b.y > p.y)) &&
                         (p.x < (b.x - a.x) * (p.y - a.y) / (b.y - a.y + 1e-18) + a.x);
        if (intersect) inside = !inside;
    }
    return inside;
}

// =========================
// Random utilities
// =========================
struct RNG {
    std::mt19937_64 eng;
    explicit RNG(uint64_t seed = std::random_device{}()) : eng(seed) {}
    double uniform(double a=0.0, double b=1.0){ std::uniform_real_distribution<double> d(a,b); return d(eng); }
    double normal(double mean=0.0, double stddev=1.0){ std::normal_distribution<double> d(mean,stddev); return d(eng); }
    template <class It> void shuffle(It first, It last){ std::shuffle(first, last, eng); }
};

// =========================
// Complex helpers
// =========================
template <class T>
std::complex<T> polar(T r, T theta){ return std::polar(r, theta); }

template <class T>
T phase(const std::complex<T>& z){ return std::arg(z); }

template <class T>
T magnitude(const std::complex<T>& z){ return std::abs(z); }

} // namespace math
#include <iostream>
using namespace math;

int main() {
    // Constants & utils
    std::cout << consts::pi<double> << "\n";
    std::cout << std::boolalpha << almost_equal(0.1+0.2, 0.3, 1e-12) << "\n";

    // Number theory
    std::cout << gcd(48, 18) << " " << lcm(12, 18) << "\n";
    std::cout << is_probable_prime(1'000'000'007ULL) << "\n";
    std::cout << mod_pow(5LL, 117LL, 19LL) << "\n";

    // Vectors & matrices
    Vec3 a{1,2,3}, b{4,5,6};
    std::cout << dot(a,b) << " " << norm(cross(a,b)) << "\n";

    Mat A(3,3); A(0,0)=4;A(0,1)=2;A(0,2)=0; A(1,0)=2;A(1,1)=4;A(1,2)=2; A(2,0)=0;A(2,1)=2;A(2,2)=4;
    Vec rhs(3); rhs[0]=2; rhs[1]=4; rhs[2]=6;
    auto x = solve(A, rhs);
    std::cout << x[0] << " " << x[1] << " " << x[2] << "\n";

    // Numeriek & polynomen
    auto f = [](double x){ return std::cos(x) - x; };
    double root = bisection(f, 0.0, 1.0);
    std::cout << root << "\n";

    std::vector<double> pc = {1, 0, -1}; // 1 - x^2
    std::cout << poly_eval(pc, 0.5) << "\n";
    auto qr = roots_quadratic(1.0, 0.0, -1.0);
    std::cout << qr[0] << " " << qr[1] << "\n";

    // Statistiek
    std::vector<double> s = {1,2,3,4,5};
    std::cout << mean(s) << " " << stddev(s) << " " << median(s) << "\n";

    // Geometrie
    std::vector<Point> pts = {{0,0},{1,0},{1,1},{0,1},{0.5,0.5}};
    auto hull = convex_hull(pts);
    std::cout << "hull size=" << hull.size() << " area=" << polygon_area(hull) << "\n";

    return 0;
}
