# Introduction

This repository is a copy of LZMA SDK https://www.7-zip.org/sdk.html https://www.7-zip.org/a/lzma2301.7z

This redistribution is released under GNU LGPL license in License.txt in root directory. The documentation and original licenses in DOC directory are preserved for clarity.

Repository is used for development of custom sfx headers for installers. Project CPP/7zip/Bundles/SFXSetup is upgraded from VC++ 6 workspace to VS 17 solution. Some of the behaviors of the project have been modernized, for easier build and debugging. Sln file should be starting location for anyone interested in development and compilation of this project.

# SFX size

Due to toolset update to v143 and latest windows 10 sdk, generated sfx files are larger than ones delivered with LZMA SDK.
