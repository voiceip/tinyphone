# Build Script...
Set-StrictMode -Version latest
$ErrorActionPreference="Stop"
$PSDefaultParameterValues['*:ErrorAction']='Stop'

$BuildMode="Release"
 
Write-Host 'Building Tinyphone!'

cmd /c subst E: $env:CodeDir
ls E:

#curl
cd E:\lib\curl\
.\buildconf.bat
cd E:\lib\curl\winbuild

where.exe msbuild.exe
nmake /f Makefile.vc mode=dll VC=15 MACHINE=x86 DEBUG=no

cmd /c MKLINK /D E:\lib\curl\builds\libcurl-vc-x86-release-dll-ipv6-sspi-winssl E:\lib\curl\builds\libcurl-vc15-x86-release-dll-ipv6-sspi-winssl
cmd /c E:\lib\curl\builds\libcurl-vc15-x86-release-dll-ipv6-sspi-winssl\bin\curl.exe https://wttr.in/bangalore


#G729
cd E:\lib\bcg729\build\
cmake ..
msbuild /m bcg729.sln /p:Configuration=$BuildMode /p:Platform=Win32

#cryptopp
cd E:\lib\cryptopp
msbuild /m cryptlib.vcxproj /p:Configuration=$BuildMode /p:Platform=Win32 /p:PlatformToolset=v140_xp

#portaudio
$wc = New-Object net.webclient; $wc.Downloadfile("https://download.steinberg.net/sdk_downloads/asiosdk_2.3.3_2019-06-14.zip", "E:\lib\portaudio\src\hostapi\asio\asiosdk_2.3.3_2019-06-14.zip")
cd E:\lib\portaudio\src\hostapi\asio
unzip asiosdk_2.3.3_2019-06-14.zip
mv asiosdk_2.3.3_2019-06-14 ASIOSDK
cd E:\lib\portaudio\build\msvc
msbuild /m portaudio.sln /p:Configuration=$BuildMode /p:Platform=Win32

#pjproject
cd E:\lib\pjproject
msbuild /m pjproject-vs14.sln -target:libpjproject:Rebuild /p:Configuration=$BuildMode-Static /p:Platform=Win32

#statsd-cpp
cd E:\lib\statsd-cpp
cmake .
msbuild /m statsd-cpp.vcxproj /p:Configuration=$BuildMode /p:Platform=Win32

#tinyphone
cd E:\tinyphone
sed -i 's/stampver.inf.*\$/stampver.inf $/g' tinyphone.vcxproj

#msbuild /m tinyphone.sln -target:tinyphone /p:Configuration=$BuildMode /p:Platform=x86
#msbuild /m tinyphone.sln -target:tinyphone:Rebuild /p:Configuration=$BuildMode /p:Platform=x86
msbuild /m tinyphone.sln /p:Configuration=$BuildMode /p:Platform=x86


#required for github-ci permission issue.
cmd /c icacls E:/tinyphone-installer/bin/Release/tinyphone_installer.msi /grant everyone:f

#git diff --exit-code stampver.inf
