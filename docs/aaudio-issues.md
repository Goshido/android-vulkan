# _AAudio_ issues

At the moment **[2022/12/31]** it was discovered that [_AAudio_](https://developer.android.com/ndk/guides/audio/aaudio/aaudio) system has a huge latency. This latency does not allow to create sound emitters on demand in runtime. For example creating and playing sound right from _Lua_ script logic thread. So it's needed more complex architecture to hide such huge latency and avoid micro-freezing.

 All measuarements were done on _XIAOMI Redmi Note 8 Pro_, _Android 10_, latest available firmware.

## `AAudioStreamBuilder_openStream` latency

<img src="./images/aadio-open-trace.png"/>

In average: **38.25 ms**

## `AAudioStream_requestStart` latency

<img src="./images/aadio-play-trace.png"/>

In average: **8.7 ms**
