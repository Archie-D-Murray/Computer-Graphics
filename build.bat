cmake -S . -B build/ -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF
if %ERRORLEVEL% == 0 goto cmake_build
echo CMake failed
exit

:cmake_build
cmake --build build/
if %ERRORLEVEL% == 0 goto run_exe
echo Build failed
exit

:run_exe
cd /d ".\source"
Computer_Graphics_Coursework.exe
cd /d ..
