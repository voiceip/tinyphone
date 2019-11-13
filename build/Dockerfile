# escape=`


# FROM mcr.microsoft.com/dotnet/framework/runtime:3.5-windowsservercore-ltsc2016
# FROM microsoft/windowsservercore:ltsc2016
FROM microsoft/dotnet-framework:3.5-windowsservercore-ltsc2016

# Install VS components

RUN mkdir C:\BuildTools

# Reset the shell.
SHELL ["cmd", "/S", "/C"]

# Set up environment to collect install errors.
COPY Install.cmd C:\TEMP\
ADD https://aka.ms/vscollect.exe C:\TEMP\collect.exe

# Download and install Build Tools 14.0
ADD https://download.microsoft.com/download/E/E/D/EEDF18A8-4AED-4CE0-BEBE-70A83094FC5A/BuildTools_Full.exe C:\TEMP\msbuild14.exe
RUN start /wait C:\TEMP\msbuild14.exe /q /full /log C:\TEMP\msbuild14.log

# Download channel for fixed install.
ARG CHANNEL_URL=https://aka.ms/vs/15/release/channel
ADD ${CHANNEL_URL} C:\TEMP\VisualStudio.chman

# Download and install Build Tools for Visual Studio 2017 for native desktop workload.
ADD https://aka.ms/vs/15/release/vs_buildtools.exe C:\TEMP\vs_buildtools.exe
RUN C:\TEMP\Install.cmd C:\TEMP\vs_buildtools.exe --quiet --wait --norestart --nocache `
    --channelUri C:\TEMP\VisualStudio.chman `
    --installChannelUri C:\TEMP\VisualStudio.chman `
    --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended`
    --add Microsoft.Component.MSBuild `
    --add Microsoft.VisualStudio.Component.VC.140 `
    --add Microsoft.VisualStudio.Component.WinXP `
    --installPath C:\BuildTools 

# Install Scoop
RUN powershell.exe -ExecutionPolicy RemoteSigned `
  iex (new-object net.webclient).downloadstring('https://get.scoop.sh');

# Install Git & other tools
RUN powershell.exe scoop install git wget cmake openssh unzip make

RUN powershell.exe [environment]::setenvironmentvariable('GIT_SSH', (resolve-path (scoop which ssh)), 'USER')

ADD https://dl.bintray.com/boostorg/release/1.68.0/binaries/boost_1_68_0-msvc-14.0-32.exe C:\TEMP\boost_1_68_0-msvc-14.0-32.exe
RUN C:\TEMP\boost_1_68_0-msvc-14.0-32.exe /SILENT

ADD https://github.com/wixtoolset/wix3/releases/download/wix3112rtm/wix311.exe C:\TEMP\wix311.exe
RUN C:\TEMP\Install.cmd C:\TEMP\wix311.exe /install /quiet /norestart

RUN mkdir C:\Code

# fix for paste issue https://github.com/moby/moby/issues/29646#issuecomment-300483809
WORKDIR "Program Files"
RUN rd /S /Q WindowsPowerShell\Modules\PSReadLine

#subst E: C:\Code\tinyphone

ADD VsDevCmdPowerShell.bat C:\BuildTools\
ENTRYPOINT C:\BuildTools\VsDevCmdPowerShell.bat

# Start developer command prompt with any other commands specified.
# ENTRYPOINT C:\BuildTools\Common7\Tools\VsDevCmd.bat &&

# Default to PowerShell if no other command specified.
# CMD ["powershell.exe", "-NoLogo", "-ExecutionPolicy", "Bypass"]