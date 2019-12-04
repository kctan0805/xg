call "C:\Program Files (x86)\Microsoft Visual Studio\2017\WDExpress\Common7\Tools\VsDevCmd.bat"
git submodule update --init
cmake -H. -B./build -G"Visual Studio 15 2017 Win64"
@pause
