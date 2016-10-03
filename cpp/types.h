// Provides a function to print types for debug

const char* types() { return "void"; }

template <class... Args>
const char* types() { return __PRETTY_FUNCTION__; }

template <class... Args>
const char* types(Args&&...) { return types<Args&&...>(); }
