#include <napi.h>

#include <string>

void PopulateResultFromExtension(std::string&);

void Magic(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  std::string result = "<unset>";
  PopulateResultFromExtension(result);
  info[0].As<Napi::Function>().Call(env.Global(), {Napi::String::New(env, result)});
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  static_cast<void>(exports);
  return Napi::Function::New(env, Magic);
}

NODE_API_MODULE(lib, Init)
