#pragma once

#include <utility>
#include <iostream>

class Logger {
  private:
    enum class State : char {
      valid,
      moved,
      destroyed
    };

    volatile State state = State::valid;

    static void check(Logger const& logger) {
      switch (logger.state) {
        case State::valid:
          return;
        case State::moved:
          logger.log() << "/!\\ used after move" << std::endl;
          return;
        case State::destroyed:
          logger.log() << "/!\\ used after destruction" << std::endl;
          return;
        default:
          logger.log() << "/!\\ used in unknown state" << std::endl;
          return;
      }
    }

    static void use(Logger const&  logger) { check(logger); }
    static void use(Logger      && logger) { check(logger); logger.state = State::moved; }

    template <class T>
    struct addr {
      const void* p;
      const char* cv;
      friend std::ostream& operator<<(std::ostream& out, addr a) {
        return out << a.p << a.cv;
      }
      addr(T      && t) : p(&t), cv(" & ") {}
      addr(T      &  t) : p(&t), cv(" &&") {}
      addr(T const&  t) : p(&t), cv(" const& ") {}
      addr(T const&& t) : p(&t), cv(" const&&") {}
    };
    template <class T>
    addr<typename std::decay<T>::type> address(T&& t) {
      return std::forward<T>(t);
    }

    std::ostream& log()      &  { return std::cerr << "[" << this << " &]       "; }
    std::ostream& log()      && { return std::cerr << "[" << this << " &&]      "; }
    std::ostream& log() const&  { return std::cerr << "[" << this << " const&]  "; }
    std::ostream& log() const&& { return std::cerr << "[" << this << " const&&] "; }

    Logger(State state) : state(state) {}

    template <class U, class V>
    using both = typename std::enable_if<std::is_same<typename std::decay<U>::type, Logger>::value && std::is_same<typename std::decay<V>::type, Logger>::value, Logger>::type;
  public:
    friend std::ostream& operator<<(std::ostream& out, Logger      &  logger) { return out << "{" << &logger << " &} "; }
    friend std::ostream& operator<<(std::ostream& out, Logger      && logger) { return out << "{" << &logger << " &&}"; }
    friend std::ostream& operator<<(std::ostream& out, Logger const&  logger) { return out << "{" << &logger << " const&} "; }
    friend std::ostream& operator<<(std::ostream& out, Logger const&& logger) { return out << "{" << &logger << " const&&}"; }

    Logger() { log() << "construction: default" << std::endl; }
    Logger(           Logger      &  logger) { use(static_cast<Logger      & >(logger)); log() << "construction: copied from " << static_cast<Logger      & >(logger) << std::endl; }
    Logger(           Logger      && logger) { use(static_cast<Logger      &&>(logger)); log() << "construction: moved  from " << static_cast<Logger      &&>(logger) << std::endl; }
    Logger(           Logger const&  logger) { use(static_cast<Logger const& >(logger)); log() << "construction: copied from " << static_cast<Logger const& >(logger) << std::endl; }
    Logger(           Logger const&& logger) { use(static_cast<Logger const&&>(logger)); log() << "construction: copied from " << static_cast<Logger const&&>(logger) << std::endl; }
    Logger& operator=(Logger      &  logger) { use(static_cast<Logger      & >(logger)); log() <<   "assignment: copied from " << static_cast<Logger      & >(logger) << std::endl; return *this; }
    Logger& operator=(Logger      && logger) { use(static_cast<Logger      &&>(logger)); log() <<   "assignment: moved  from " << static_cast<Logger      &&>(logger) << std::endl; return *this; }
    Logger& operator=(Logger const&  logger) { use(static_cast<Logger const& >(logger)); log() <<   "assignment: copied from " << static_cast<Logger const& >(logger) << std::endl; return *this; }
    Logger& operator=(Logger const&& logger) { use(static_cast<Logger const&&>(logger)); log() <<   "assignment: copied from " << static_cast<Logger const&&>(logger) << std::endl; return *this; }

    ~Logger() {
      if (state == State::destroyed) {
        log() << "/!\\ destroyed after destruction" << std::endl;
      }
      state = State::destroyed;
      log() << "destruction" << std::endl;
    }

    template <class T, class = typename std::enable_if<!std::is_same<typename std::decay<T>::type, Logger>::value>::type>
    Logger(T&& t) { log() << "construction: conversion from " << std::forward<T>(t) << " {" << address(std::forward<T>(t)) << "}" << std::endl; }

    explicit operator bool()      &  { use(*this); log() << "conversion to bool" << std::endl; return state == State::valid; }
    explicit operator bool()      && { use(*this); log() << "conversion to bool" << std::endl; return state == State::valid; }
    explicit operator bool() const&  { use(*this); log() << "conversion to bool" << std::endl; return state == State::valid; }
    explicit operator bool() const&& { use(*this); log() << "conversion to bool" << std::endl; return state == State::valid; }
    
    template <class T>
    friend Logger operator+(T&& t) {
      Logger r = Logger(State::valid);
      use(std::forward<T>(t));
      r.log() << "construction: +" << std::forward<T>(t) << std::endl;
      return r;
    }
    
    template <class T>
    friend Logger operator-(T&& t) {
      Logger r = Logger(State::valid);
      use(std::forward<T>(t));
      r.log() << "construction: -" << std::forward<T>(t) << std::endl;
      return r;
    }
    
    template <class T>
    friend Logger operator!(T&& t) {
      Logger r = Logger(State::valid);
      use(std::forward<T>(t));
      r.log() << "construction: !" << std::forward<T>(t) << std::endl;
      return r;
    }
    
    template <class T>
    friend Logger operator~(T&& t) {
      Logger r = Logger(State::valid);
      use(std::forward<T>(t));
      r.log() << "construction: ~" << std::forward<T>(t) << std::endl;
      return r;
    }

    template <class U, class V>
    friend both<U, V> operator+(U&& u, V&& v) {
      Logger r = Logger(State::valid);
      use(std::forward<U>(u));
      use(std::forward<V>(v));
      r.log() << "construction: " << std::forward<U>(u) << " + " << std::forward<V>(v) << std::endl;
      return r;
    }

    template <class U, class V>
    friend both<U, V> operator-(U&& u, V&& v) {
      Logger r = Logger(State::valid);
      use(std::forward<U>(u));
      use(std::forward<V>(v));
      r.log() << "construction: " << std::forward<U>(u) << " - " << std::forward<V>(v) << std::endl;
      return r;
    }

    template <class U, class V>
    friend both<U, V> operator*(U&& u, V&& v) {
      Logger r = Logger(State::valid);
      use(std::forward<U>(u));
      use(std::forward<V>(v));
      r.log() << "construction: " << std::forward<U>(u) << " * " << std::forward<V>(v) << std::endl;
      return r;
    }

    template <class U, class V>
    friend both<U, V> operator/(U&& u, V&& v) {
      Logger r = Logger(State::valid);
      use(std::forward<U>(u));
      use(std::forward<V>(v));
      r.log() << "construction: " << std::forward<U>(u) << " / " << std::forward<V>(v) << std::endl;
      return r;
    }

    template <class U, class V>
    friend both<U, V> operator%(U&& u, V&& v) {
      Logger r = Logger(State::valid);
      use(std::forward<U>(u));
      use(std::forward<V>(v));
      r.log() << "construction: " << std::forward<U>(u) << " % " << std::forward<V>(v) << std::endl;
      return r;
    }

    template <class U, class V>
    friend both<U, V> operator==(U&& u, V&& v) {
      Logger r = Logger(State::valid);
      use(std::forward<U>(u));
      use(std::forward<V>(v));
      r.log() << "construction: " << std::forward<U>(u) << " == " << std::forward<V>(v) << std::endl;
      return r;
    }

    template <class U, class V>
    friend both<U, V> operator!=(U&& u, V&& v) {
      Logger r = Logger(State::valid);
      use(std::forward<U>(u));
      use(std::forward<V>(v));
      r.log() << "construction: " << std::forward<U>(u) << " != " << std::forward<V>(v) << std::endl;
      return r;
    }

    template <class U, class V>
    friend both<U, V> operator>(U&& u, V&& v) {
      Logger r = Logger(State::valid);
      use(std::forward<U>(u));
      use(std::forward<V>(v));
      r.log() << "construction: " << std::forward<U>(u) << " > " << std::forward<V>(v) << std::endl;
      return r;
    }

    template <class U, class V>
    friend both<U, V> operator<(U&& u, V&& v) {
      Logger r = Logger(State::valid);
      use(std::forward<U>(u));
      use(std::forward<V>(v));
      r.log() << "construction: " << std::forward<U>(u) << " < " << std::forward<V>(v) << std::endl;
      return r;
    }

    template <class U, class V>
    friend both<U, V> operator>=(U&& u, V&& v) {
      Logger r = Logger(State::valid);
      use(std::forward<U>(u));
      use(std::forward<V>(v));
      r.log() << "construction: " << std::forward<U>(u) << " >= " << std::forward<V>(v) << std::endl;
      return r;
    }

    template <class U, class V>
    friend both<U, V> operator<=(U&& u, V&& v) {
      Logger r = Logger(State::valid);
      use(std::forward<U>(u));
      use(std::forward<V>(v));
      r.log() << "construction: " << std::forward<U>(u) << " <= " << std::forward<V>(v) << std::endl;
      return r;
    }

    template <class U, class V>
    friend both<U, V> operator&&(U&& u, V&& v) {
      Logger r = Logger(State::valid);
      use(std::forward<U>(u));
      use(std::forward<V>(v));
      r.log() << "construction: " << std::forward<U>(u) << " && " << std::forward<V>(v) << std::endl;
      return r;
    }

    template <class U, class V>
    friend both<U, V> operator||(U&& u, V&& v) {
      Logger r = Logger(State::valid);
      use(std::forward<U>(u));
      use(std::forward<V>(v));
      r.log() << "construction: " << std::forward<U>(u) << " || " << std::forward<V>(v) << std::endl;
      return r;
    }

    template <class U, class V>
    friend both<U, V> operator&(U&& u, V&& v) {
      Logger r = Logger(State::valid);
      use(std::forward<U>(u));
      use(std::forward<V>(v));
      r.log() << "construction: " << std::forward<U>(u) << " & " << std::forward<V>(v) << std::endl;
      return r;
    }

    template <class U, class V>
    friend both<U, V> operator|(U&& u, V&& v) {
      Logger r = Logger(State::valid);
      use(std::forward<U>(u));
      use(std::forward<V>(v));
      r.log() << "construction: " << std::forward<U>(u) << " | " << std::forward<V>(v) << std::endl;
      return r;
    }

    template <class U, class V>
    friend both<U, V> operator^(U&& u, V&& v) {
      Logger r = Logger(State::valid);
      use(std::forward<U>(u));
      use(std::forward<V>(v));
      r.log() << "construction: " << std::forward<U>(u) << " ^ " << std::forward<V>(v) << std::endl;
      return r;
    }
};
