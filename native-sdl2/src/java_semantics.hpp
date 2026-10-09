#pragma once
#include <cstdint>
#include <cmath>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <type_traits>
#include <algorithm>

// Java int: explicit modulo 2^32 arithmetic, arithmetic shifts, saturating
// floating-point narrowing. No C++ signed overflow or negative left shifts.
struct jint {
    uint32_t bits = 0;
    jint() = default;
    template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    jint(T v) : bits(static_cast<uint32_t>(v)) {}
    jint(double v) : bits(std::isnan(v) ? 0u : v >= 2147483647.0 ? 0x7fffffffu :
        v <= -2147483648.0 ? 0x80000000u : static_cast<uint32_t>(static_cast<int32_t>(v))) {}
    operator int32_t() const { return bits < 0x80000000u ? static_cast<int32_t>(bits) :
        -1 - static_cast<int32_t>(0xffffffffu - bits); }
    jint* operator->() { return this; }
    jint intValue() const { return *this; }
    int8_t byteValue() const { return bits % 256 < 128 ? static_cast<int8_t>(bits % 256) :
        static_cast<int8_t>(int(bits % 256) - 256); }
    jint operator-() const { return jint(0u - bits); }
    jint operator~() const { return jint(~bits); }
    jint& operator++() { ++bits; return *this; }
    jint operator++(int) { jint old=*this; ++bits; return old; }
    jint& operator--() { --bits; return *this; }
    jint operator--(int) { jint old=*this; --bits; return old; }
};
inline jint operator+(jint a,jint b) { return jint(a.bits+b.bits); }
inline jint operator-(jint a,jint b) { return jint(a.bits-b.bits); }
inline jint operator*(jint a,jint b) { return jint(a.bits*b.bits); }
inline jint operator/(jint a,jint b) { if(!b.bits) throw std::runtime_error("Java integer division by zero"); return jint(int64_t(int32_t(a))/int32_t(b)); }
inline jint operator%(jint a,jint b) { if(!b.bits) throw std::runtime_error("Java integer remainder by zero"); return jint(int64_t(int32_t(a))%int32_t(b)); }
inline jint operator&(jint a,jint b) { return jint(a.bits&b.bits); }
inline jint operator|(jint a,jint b) { return jint(a.bits|b.bits); }
inline jint operator^(jint a,jint b) { return jint(a.bits^b.bits); }
inline jint operator<<(jint a,jint b) { return jint(a.bits<<(b.bits&31u)); }
inline jint operator>>(jint a,jint b) { uint32_t n=b.bits&31u; return jint(!n ? a.bits : (a.bits>>n) | (a.bits&0x80000000u ? (0xffffffffu<<(32u-n)) : 0u)); }
#define JINT_COMPOUND(OP) inline jint& operator OP##=(jint& a,jint b) { a=a OP b; return a; }
JINT_COMPOUND(+) JINT_COMPOUND(-) JINT_COMPOUND(*) JINT_COMPOUND(/)
JINT_COMPOUND(%) JINT_COMPOUND(&) JINT_COMPOUND(|) JINT_COMPOUND(^)
JINT_COMPOUND(<<) JINT_COMPOUND(>>)
#undef JINT_COMPOUND
// All integer literals in the translated engine are jint too. Mixed double
// expressions retain Java binary numeric promotion, without narrowing early.
#define JINT_DOUBLE(OP) inline double operator OP(jint a,double b) { return double(int32_t(a)) OP b; } inline double operator OP(double a,jint b) { return a OP double(int32_t(b)); }
JINT_DOUBLE(+) JINT_DOUBLE(-) JINT_DOUBLE(*) JINT_DOUBLE(/)
#undef JINT_DOUBLE

struct String : std::string {
    using std::string::string;
    String() = default;
    String(const std::string& s) : std::string(s) {}
    String* operator->() { return this; }
    const String* operator->() const { return this; }
    jint length() const { return jint(size()); }
    jint indexOf(const String& s,jint pos) const { auto p=find(s,static_cast<size_t>(int32_t(pos))); return p==npos ? jint(-1) : jint(p); }
    String substring(jint a,jint b) const { if(a<0 || b<a || size_t(int32_t(b))>size()) throw std::out_of_range("Java substring"); return substr(int32_t(a),int32_t(b-a)); }
    String trim() const { auto a=find_first_not_of(" \t\r\n\f"); if(a==npos) return {}; return substr(a,find_last_not_of(" \t\r\n\f")-a+1); }
    String toLowerCase() const { String s=*this; for(char& c:s) if(c>='A' && c<='Z') c=char(c+32); return s; }
    jint compareTo(const String& s) const { return jint(compare(s)); }
};
inline String operator+(const String& a,const String& b) { return String(static_cast<const std::string&>(a)+static_cast<const std::string&>(b)); }
inline double jdouble(double d) { return d; }
inline double jdouble(const String& s) { size_t n=0; double d=std::stod(s,&n); if(n!=s.size()) throw std::runtime_error(String("Invalid double: ")+s); return d; }
inline jint jinteger(const String& s) { size_t n=0; long long v=std::stoll(s,&n); if(n!=s.size() || v>2147483647LL || v< -2147483648LL) throw std::runtime_error(String("Invalid integer: ")+s); return jint(v); }
inline jint jinteger(jint v) { return v; }

template<class T> using Ref = std::shared_ptr<T>;
template<class T> struct Array {
    Ref<std::vector<T>> storage;
    jint length;
    Array() = default;
    Array(std::nullptr_t) {}
    explicit Array(jint n) : storage(std::make_shared<std::vector<T>>(size_t(int32_t(n)))),length(n) {}
    Array* operator->() { return this; }
    const Array* operator->() const { return this; }
    T& operator[](jint n) { return storage->at(size_t(int32_t(n))); }
    const T& operator[](jint n) const { return storage->at(size_t(int32_t(n))); }
    bool operator!=(std::nullptr_t) const { return bool(storage); }
    bool operator==(std::nullptr_t) const { return !storage; }
};
template<class T> Array<Array<T>> array2(jint x,jint y) { Array<Array<T>> a(x); for(int32_t i=0;i<int32_t(x);++i) a[jint(i)]=Array<T>(y); return a; }
struct Object { virtual ~Object() = default; };
struct Hashtable {
    Ref<std::map<String,Ref<Object>>> entries = std::make_shared<std::map<String,Ref<Object>>>();
    Hashtable* operator->() { return this; }
    void put(const String& k,Ref<Object> v) { (*entries)[k]=v; }
    Ref<Object> get(const String& k) { auto it=entries->find(k); return it==entries->end() ? nullptr : it->second; }
};
struct JavaRandom {
    uint64_t state = 0;
    void seed(uint64_t s) { state=(s^0x5deece66dULL)&((1ULL<<48)-1); }
    uint64_t next(int bits) { state=(state*0x5deece66dULL+11)&((1ULL<<48)-1); return state>>(48-bits); }
    double random() { uint64_t a=next(26); uint64_t b=next(27); return double((a<<27)+b)/9007199254740992.0; }
};
