{
  "targets": [
    {
      "target_name": "fip_addon",
      "sources": [
        "fip_addon.cpp",
        "DirectOutputImpl.cpp"
      ],
      "include_dirs": [
        "<!(node -p \"require('node-addon-api').include\")"
      ],
      "defines": ["NAPI_CPP_EXCEPTIONS"],
      "libraries": [],
      "msvs_settings": {
        "VCCLCompilerTool": {
          "ExceptionHandling": 1
        }
      }
    }
  ]
}
