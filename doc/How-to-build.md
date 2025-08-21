# Build Guide

## Environment Setup
- Common dependencies:
  - Git >= 2.0
  - CMake >= 3.16
- Recommended for Chinese users:
  * [gh-proxy](https://github.com/hunshcn/gh-proxy)

## Windows Build

### Dependency Installation
- DOC
  - [winget](https://learn.microsoft.com/en-us/windows/package-manager/winget/)

```powershell
winget install cmake "Strawberry Perl" Git.Git "Visual Studio Community 2019"
```

### Build Command
```powershell
# Run in VS Developer Command Prompt
./build_release_win.ps1 -c <Debug|Release> [-g <PROXY_URL>]
```

## macOS Build
### Dependency Preparation
```bash
# Install basic dependencies using Homebrew
brew install cmake gettext
```

### Build Command
```bash
# General build command format
./build_release_macos.sh -a <x86_64|arm64> -c <Debug|Release> [-g <PROXY_URL>]
```

## Linux Build
### Ubuntu/Debian
```bash
# Install basic toolchain
sudo apt install -y build-essential cmake git

# Execute build
bash BuildLinux.sh -c <Debug|Release> [-g <PROXY_URL>]
```



## FAQ
- Q: Dependency download timeout
  A: recommend using `-g` parameter to explicitly specify proxy
- Q: macOS architecture mismatch
  A: Ensure `-a` parameter matches processor architecture (x86_64 for Intel chips, arm64 for M-series)