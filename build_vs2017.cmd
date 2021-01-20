git -C third_party/KTX-Software lfs pull
call "D:\Program Files (x86)\Microsoft Visual Studio\2017\WDExpress\Common7\Tools\VsDevCmd.bat"
cmake -H. -B./build -G"Visual Studio 15 2017 Win64"
@pause
