
# 在 plot 中添加一类自定义绘制图形的序列的方法

- 在 src/mvAppItemTypes.inc 的 MV_ITEM_TYPES 中添加一个 enum 项
- 在 src/ext/ext.cpp 中的 GetEntityCommand 函数中添加新的 case 项
- 在 src/ext/ 目录下新增一对相应名称的 .h .cpp  文件， 并修改其中的类型， 根据 mv??? 名称搜索修改, 
  - 注1： 在ext下新增代码文件需要 cmake:configure 一次
  - 注2： 记得改.h文件中的 第1 2 行的 #ifdefine 后的值
- 在 src/ext/ext.h 中添加 #include
- 在 src/ext/ext.cpp 中添加各 switch 项下增加新类别的 case 项
- 在 dearpygui/dearpygui.py 中添加对应的函数

# 配置更新

有新增代码文件在 src/ext 下后， 需要cmake配置一次， 命令行为 

  python build.py config

# 编译

python build.py

# 注意

- 编译完成后， 会向当前python环境中更新 dearpygui 的包文件， 所以需要注意以下两点
  - 执行编译过程需要先切换到要使用 dpg 的python环境
  - 改环境中需要预先安装 dearpygui
