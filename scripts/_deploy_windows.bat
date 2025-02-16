cd ..

@echo off
call qt-cmake -B build
if %errorlevel% neq 0 exit /b %errorlevel%

call cmake --build build --config Release
if %errorlevel% neq 0 exit /b %errorlevel%

call windeployqt --release build\Release\QtSim.exe
if %errorlevel% neq 0 exit /b %errorlevel%

:: Copy FFmpeg DLLs
set FFmpegDir=external\ffmpeg\windows\bin
set OutputDir=build\Release

echo Copying FFmpeg DLLs...
xcopy /Y /D "%FFmpegDir%\avcodec-*.dll" "%OutputDir%\"
xcopy /Y /D "%FFmpegDir%\avformat-*.dll" "%OutputDir%\"
xcopy /Y /D "%FFmpegDir%\avutil-*.dll" "%OutputDir%\"
xcopy /Y /D "%FFmpegDir%\swscale-*.dll" "%OutputDir%\"
xcopy /Y /D "%FFmpegDir%\swresample-*.dll" "%OutputDir%\"

echo Deployment completed!