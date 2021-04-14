#include <napi.h>

void Magic(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  info[0].As<Napi::Function>().Call(env.Global(), {Napi::String::New(env, "The Magic happened.")});
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  static_cast<void>(exports);
  return Napi::Function::New(env, Magic);
}

NODE_API_MODULE(lib, Init)
