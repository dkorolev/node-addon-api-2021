{
  "targets": [
    {
      "target_name": "native",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "REMOVED_cflags_cc": [ "-std=c++17" ],
      "REMOVED_xcode_settings": {
        "OTHER_CFLAGS": [ "-std=c++17" ]
      },
      "sources": [ "native.cc" ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
      "libraries": [ "../extension/build/extension_using_current.a" ]
    }
  ]
}
