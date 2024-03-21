# $ErrorActionPreference="Stop"
$BuildMode = "Release"
set-psdebug -trace 0

Write-Host "WiX Toolset path added to PATH: $wixPath" -ForegroundColor Yellow

Write-Host "`nUpdating Submodules......"
git submodule -q update --init

cmd /c subst E: C:\Users\alyss\source\Repos\tinyphone

Push-Location "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools"
cmd /c "VsDevCmd.bat&set" |
ForEach-Object {
  if ($_ -match "=") {
    $v = $_.split("="); set-item -force -path "ENV:\$($v[0])"  -value "$($v[1])"
  }
}
Pop-Location
Write-Host "`nVisual Studio 2019 Command Prompt variables set." -ForegroundColor Yellow

Set-Location E:\lib\curl\
Get-ChildItem
.\buildconf.bat
Set-Location E:\lib\curl\winbuild

where.exe msbuild.exe
nmake /f Makefile.vc mode=dll VC=19 DEBUG=no

cmd /c MKLINK /D E:\lib\curl\builds\libcurl-vc-x86-release-dll-ipv6-sspi-winssl E:\lib\curl\builds\libcurl-vc19-x86-release-dll-ipv6-sspi-winssl
cmd /c .\libcurl-vc19-x86-release-dll-ipv6-sspi-winssl\bin\curl.exe https://wttr.in/bangalore

#G729
Set-Location E:\lib\bcg729\build\
cmake .. -A Win32
msbuild /m bcg729.sln /p:Configuration=$BuildMode /p:Platform=Win32 /p:PlatformToolset=v142
if ($LastExitCode -ne 0) { $host.SetShouldExit($LastExitCode) }

Set-Location E:\lib\cryptopp
msbuild /m cryptlib.vcxproj /p:Configuration=$BuildMode /p:Platform=Win32 /p:PlatformToolset=v142
if ($LastExitCode -ne 0) { $host.SetShouldExit($LastExitCode) }

$wc = New-Object net.webclient; $wc.Downloadfile("https://download.steinberg.net/sdk_downloads/asiosdk_2.3.3_2019-06-14.zip", "E:\lib\portaudio\src\hostapi\asio\asiosdk_2.3.3_2019-06-14.zip")
Set-Location E:\lib\portaudio\src\hostapi\asio
unzip asiosdk_2.3.3_2019-06-14.zip
Move-Item asiosdk_2.3.3_2019-06-14 ASIOSDK
Set-Location E:\lib\portaudio\build\msvc
msbuild /m portaudio.sln /p:Configuration=$BuildMode /p:Platform=Win32 /p:PlatformToolset=v142 /p:WindowsTargetPlatformVersion=10.0
if ($LastExitCode -ne 0) { $host.SetShouldExit($LastExitCode) }

Set-Location E:\lib\pjproject
msbuild /m pjproject-vs14.sln -target:libpjproject:Rebuild /p:Configuration=$BuildMode-Static /p:Platform=Win32 /p:PlatformToolset=v142
if ($LastExitCode -ne 0) { $host.SetShouldExit($LastExitCode) }

Set-Location E:\lib\statsd-cpp
cmake .
msbuild /m statsd-cpp.vcxproj /p:Configuration=$BuildMode /p:Platform=Win32 /p:PlatformToolset=v142 /p:WindowsTargetPlatformVersion=10.0
if ($LastExitCode -ne 0) { $host.SetShouldExit($LastExitCode) }

Set-Location E:\tinyphone

sed -i 's/stampver.inf.*\$/stampver.inf $/g' tinyphone.vcxproj

msbuild /m tinyphone.sln /p:Configuration=$BuildMode /p:Platform=x86 /p:PlatformToolset=v142 /p:WindowsTargetPlatformVersion=10.0
if ($LastExitCode -ne 0) { $host.SetShouldExit($LastExitCode) }

Write-Host "`nBuild Completed." -ForegroundColor Yellow