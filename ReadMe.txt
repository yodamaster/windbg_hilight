feature:
windbg文本高亮，支持x64

changelog:
2013年4月26日
增加地址、opcode、jumpcall指令的颜色设置
增加命令窗口用户输入的颜色设置(只是简单地对>后的文字进行加色)

2013年4月25日
首个版本发布，暂时只有Disassembly窗口的高亮

使用方法：
将hs.dll放入32位windbg的安装目录，hs-x64.dll放入64位(x64)windbg的安装目录，
运行windbg，输入命令
.load hs
64位是
.load hs-x64

note:
首次运行是会产生asm.ini文件，里面可以设置颜色、汇编指令等，如果开了UAC而以普通权限运行windbg就可能没有asm.ini产生，只需要以管理员权限运行windbg然后加载一次就行了

特别感谢linx2008 http://hi.baidu.com/linx2008/item/0bee3aedc6d49e275b2d6441

by lynnux，2013年4月25日