#pragma once

#include <functional>
#include <thread>

#include "napi.h"

template <class T>
struct ExtractJSFunctionImpl;

class NodeJSContext final {
 private:
  struct Impl final {
    Napi::Env env_;
    Napi::Promise::Deferred promise_;
    std::vector<std::thread> threads_;

    explicit Impl(Napi::Env env) : env_(env), promise_(Napi::Promise::Deferred::New(env)) {}

    ~Impl() {
      for (std::thread& t : threads_) {
        t.join();
      }
      // NOTE(dkorolev): This promise can be set to something other than `true`.
      promise_.Resolve(Napi::Boolean::New(env_, true));
    }
  };
  std::shared_ptr<Impl> impl_;

 public:
  explicit NodeJSContext(Napi::Env env) : impl_(std::make_shared<Impl>(env)) {}
  explicit NodeJSContext(const Napi::CallbackInfo& info) : NodeJSContext(info.Env()) {}

  template <typename T>
  typename ExtractJSFunctionImpl<T>::retval_t ExtractJSFunction(T f) {
    return ExtractJSFunctionImpl<T>::DoIt(*this, f);
  }

  void RunAsync(std::function<void()> f) { impl_->threads_.emplace_back(f); }

  Napi::Env GetEnv() const { return impl_->env_; }
  Napi::Value GetPromise() const { return impl_->promise_.Promise(); }
};

template <class TUPLE, size_t I, size_t N>
struct ArgsPopulator final {
  static void DoIt(Napi::Env env, const TUPLE& input, std::vector<napi_value>& output) {
    PopulateArg(env, std::get<I>(input), output[I]);
    ArgsPopulator<TUPLE, I + 1u, N>::DoIt(env, input, output);
  }
  static void PopulateArg(Napi::Env env, int input, napi_value& output) { output = Napi::Number::New(env, input); }
  static void PopulateArg(Napi::Env env, const std::string& input, napi_value& output) {
    output = Napi::String::New(env, input);
  }
  // NOTE(dkorolev): Add more type wrappers or find the right way to do it within Napi.
};

template <class TUPLE, size_t N>
struct ArgsPopulator<TUPLE, N, N> final {
  static void DoIt(Napi::Env, const TUPLE&, std::vector<napi_value>&) {}
};

class NodeJSFunction final {
 private:
  struct Impl final {
    // The `NodeJSContext` is captured into `std::shared_ptr<NodeJSFunction::Impl>`, to ensure proper cleanup order.
    NodeJSContext context_;
    Napi::ThreadSafeFunction function_;

    Impl(NodeJSContext context, Napi::Function jsf)
        : context_(context),
          function_(Napi::ThreadSafeFunction::New(
              context_.GetEnv(),
              jsf,
              "dkorolev_cpp_callaback",
              0,  // Max queue size (0 = unlimited).
              1,  // Initial thread count.
              static_cast<void*>(nullptr),
              [context](Napi::Env, void*, void*) {
                // NOTE(dkorolev): The *IMPORTANT* part is to capture `context` by value here.
                // If this is not done, the reference counter for the very `NodeJSContext` would drop to zero,
                // the functions will get called, but the cleanup would fail, crashing the application.
              },
              reinterpret_cast<int*>(0))) {}
    ~Impl() {
      // NOTE(dkorolev): This `.Release()` would eventually call the finalization lambda, which, in its turn,
      // would release the captured-by-copy `context`, ensuring the cleanup is happening as it should,
      // first the captured functions, then by joining the async threads, and finally by setting the promise.
      function_.Release();
    }
  };
  std::shared_ptr<Impl> impl_;

 public:
  NodeJSFunction(NodeJSContext context, Napi::Function fun) : impl_(std::make_shared<Impl>(context, fun)) {}

  template <typename... ARGS>
  void operator()(ARGS&&... args) const {
    auto args_as_tuple_to_copy = std::make_tuple(std::forward<ARGS>(args)...);
    if (impl_->function_.BlockingCall(
            reinterpret_cast<int*>(0), [args_as_tuple_to_copy](Napi::Env env, Napi::Function jsf, int*) {
              std::vector<napi_value> params;
              using tuple_t = decltype(args_as_tuple_to_copy);
              params.resize(std::tuple_size<tuple_t>::value);
              ArgsPopulator<tuple_t, 0, std::tuple_size<tuple_t>::value>::DoIt(env, args_as_tuple_to_copy, params);
              jsf.Call(params);
              // TODO(dkorolev): Process the return value as needed.
            }) != napi_ok) {
      Napi::Error::Fatal("NAPI", "`Napi::ThreadSafeNapi::Function.BlockingCall() != napi_ok`.");
    }
  }
};

template <>
struct ExtractJSFunctionImpl<Napi::Function> {
  using retval_t = NodeJSFunction;
  static NodeJSFunction DoIt(NodeJSContext self, Napi::Function js_function) {
    return NodeJSFunction(self, js_function);
  }
};

template <>
struct ExtractJSFunctionImpl<Napi::Value> {
  using retval_t = NodeJSFunction;
  static NodeJSFunction DoIt(NodeJSContext self, Napi::Value js_function) {
    return NodeJSFunction(self, js_function.As<Napi::Function>());
  }
};
