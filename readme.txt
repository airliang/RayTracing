.vs隐藏目录，配置launch.vs.json
详情可以看：https://docs.microsoft.com/zh-cn/cpp/build/configure-cmake-debugging-sessions?view=msvc-160

json配置把下面代码复制粘贴进去：
"args": [
        "-w 512 -h 512",
        "-spp 16",
        "-xspp 4 -yspp 4",
        "-integrator volpath",
        "E:\\github\\RayTracing\\scenes\\scene.rt"
      ]