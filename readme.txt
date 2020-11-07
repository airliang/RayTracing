.vs隐藏目录，配置launch.vs.json
详情可以看：https://docs.microsoft.com/zh-cn/cpp/build/configure-cmake-debugging-sessions?view=msvc-160

json配置把下面代码复制粘贴进去，主要要把括号里的路径改了：
{
  "version": "0.2.1",
  "defaults": {},
  "configurations": [
    {
      "type": "debug",
      "project": "CMakeLists.txt",
      "projectTarget": "RayTracing.exe (E:\\github\\RayTracing\\bin\\debug\\RayTracing.exe)",
      "name": "RayTracing.exe (E:\\github\\RayTracing\\bin\\debug\\RayTracing.exe)",
      "args": [
        "-w 512 -h 512",
        "-spp 16",
        "-xspp 8 -yspp 8",
        "-integrator volpath",
        "E:\\github\\RayTracing\\scenes\\scene.rt"
      ]
    },

    {
      "type": "release",
      "project": "CMakeLists.txt",
      "projectTarget": "RayTracing.exe (E:\\github\\RayTracing\\bin\\release\\RayTracing.exe)",
      "name": "RayTracing.exe (E:\\github\\RayTracing\\bin\\release\\RayTracing.exe)",
      "args": [
        "-w 512 -h 512",
        "-spp 16",
        "-xspp 8 -yspp 8",
        //"-integrator volpath",
        "E:\\github\\RayTracing\\scenes\\scene.rt"
      ]
    }
  ]
}