Aparapi: API for data parallel Java.
====================================
Allows suitable code to be executed on GPU via OpenCL.

Aparapi is developed by AMD. It was opensourced recently (http://code.google.com/p/aparapi/) under
a MIT-style license. It allows to dynamically recompile Java methods into OpenCL kernels on AMD
GPU hardware (or using CPU version of OpenCL from AMD APP SDK). Aparapi is supported on Linux and Windows.


This repository will hold my experimental patches to Aparapi. Currently the main goals are supporting
different hardware (non-AMD) and adding support for MacOS X. I do *NOT* intend to fork Aparapi or
provide any support for it. If any of my patches will bring any value to you - that's good. If not -
no problem. When I'll have anything valuable here I'll try to bring it into offical aparapi source tree,
but currently it's way too early for this.

Current status
==============
29-09-2011: Simple modification of C++ code was enough to run on non-AMD devices. Version currently avaliable in the repo
was tested using Intel OpenCL 1.1 SDK on Windows 7 64-bit and it more or less works, but still some small changes to build.xml
and build.properties are needed to complie without issues. Priliminary MacOSX support is also avaliable!



28-09-2011: I have a working MacOSX version (tested on CPU only) of Aprapi locally. I need to work out some
Ant issues and I will publish it here soon.



