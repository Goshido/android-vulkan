# Release build

## Brief
_Android_ development ecosystem requires to sign every release _APK_. It's achieved via `app/build.gragle` file. The sign certificate can be generated locally per user basis via built-in [_Android Studio_ tool](https://developer.android.com/studio/publish/app-signing#generate-key).

## Release environment setup
As mentioned above the certificate information must be linked to the building process via `app/build.gradle` file. So it was decided to use approach based on user _environment variables_. This allows to not include security sensitive information under development repo. So the user have to declare the following environment variables:

Name | Description | Example
--- | --- | ---
`ANDROID_VULKAN_CERTIFICATE_STORE_FILE` | Full path to the `.jks` file | `D:\Development\android-certificates\android-vulkan.jks`
`ANDROID_VULKAN_CERTIFICATE_STORE_PASSWORD` | Password to the `.jks` file | `Secret`
`ANDROID_VULKAN_CERTIFICATE_KEY_ALIAS` | Certificate alias name | `android-vulkan`
`ANDROID_VULKAN_CERTIFICATE_KEY_PASSWORD` | Password for certificate alias | `Secret`

## Selecting release build
`Build Variats` subsystem is responsible for the selection:

<img src="./images/release-build-selector.png" width="629" />

## Recommended `CMakeLists.txt` settings

```cmake
...

# See docs/preprocessor-macros.md
target_compile_definitions ( android-vulkan PRIVATE
#    ANDROID_ENABLE_TRACE
    ANDROID_NATIVE_MODE_PORTRAIT
#    ANDROID_VULKAN_DEBUG
#    ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION
#    ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS
#    ANDROID_VULKAN_STRICT_MODE
    AV_ARM_NEON
    VK_NO_PROTOTYPES
    VK_USE_PLATFORM_ANDROID_KHR
)

...
```
