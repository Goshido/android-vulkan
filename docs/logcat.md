# _Logcat™_ best practices

_android-vulkan_ project is using `android_vulkan::C++` tag for logging. But it's better to check output from the following standard libraries too:

- _libc_
- _gralloc_
- _ion_
- _Gralloc3_

So recommended _Logcat_'s regex filter is:

```txt
^(?:android_vulkan\:\:C\+\+|libc|DEBUG|gralloc|ion|Gralloc3)$
```
