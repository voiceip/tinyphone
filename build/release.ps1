# Build Script...

Write-Host 'Hello, World!'
scoop install unzip make
 

cd C:\Code

git config --global url.https://github.com/.insteadOf git@github.com:
git clone --recurse-submodules -j8 https://github.com/voiceip/tinyphone.git
#cd C:\Code\tinyphone\

subst E: C:\Code\tinyphone


cd E:\lib\curl\
.\buildconf.bat
cd E:\lib\curl\winbuild

nmake /f Makefile.vc mode=dll VC=15 DEBUG=no

cd E:\lib\curl\builds
cmd /c MKLINK /D E:\lib\curl\builds\libcurl-vc-x86-release-dll-ipv6-sspi-winssl E:\lib\curl\builds\libcurl-vc15-x86-release-dll-ipv6-sspi-winssl
ls E:\lib\curl\builds
cmd /c .\libcurl-vc15-x86-release-dll-ipv6-sspi-winssl\bin\curl.exe https://wttr.in/seattle


#great 


#G729
cd E:\lib\bcg729\build\
cmake ..
msbuild /m bcg729.sln /p:Configuration=Debug /p:Platform=Win32


cd E:\lib\cryptopp
msbuild /m cryptlib.vcxproj /p:Configuration=Debug /p:Platform=Win32 /p:PlatformToolset=v140_xp

$wc = New-Object net.webclient; $wc.Downloadfile("https://download.steinberg.net/sdk_downloads/asiosdk_2.3.3_2019-06-14.zip", "E:\lib\portaudio\src\hostapi\asio\asiosdk_2.3.3_2019-06-14.zip")
cd E:\lib\portaudio\src\hostapi\asio
unzip asiosdk_2.3.3_2019-06-14.zip
mv asiosdk_2.3.3_2019-06-14 ASIOSDK
cd E:\lib\portaudio\build\msvc
msbuild /m portaudio.sln /p:Configuration=Debug /p:Platform=Win32

cd E:\lib\pjproject
msbuild /m pjproject-vs14.sln -target:libpjproject:Rebuild /p:Configuration=Debug-Static /p:Platform=Win32

cd E:\lib\statsd-cpp
cmake .
msbuild /m statsd-cpp.vcxproj /p:Configuration=Debug /p:Platform=Win32



#ls E:\lib\curl\builds\libcurl-vc-x86-release-dll-ipv6-sspi-winssl
cd E:\tinyphone
#msbuild /m tinyphone.sln -target:tinyphone /p:Configuration=Debug /p:Platform=x86
#msbuild /m tinyphone.sln -target:tinyphone:Rebuild /p:Configuration=Debug /p:Platform=x86
msbuild /m tinyphone.sln /p:Configuration=Debug /p:Platform=x86
 

