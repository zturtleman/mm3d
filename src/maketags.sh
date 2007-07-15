#!/bin/sh

ctags *.h */*.h *.cc */*.cc
cd libmm3d
ctags ../*.h ../*/*.h ../*.cc ../*/*.cc
cd ../mm3dcore
ctags ../*.h ../*/*.h ../*.cc ../*/*.cc
cd ../depui
ctags ../*.h ../*/*.h ../*.cc ../*/*.cc
cd ../qtui
ctags ../*.h ../*/*.h ../*.cc ../*/*.cc
cd ../implui
ctags ../*.h ../*/*.h ../*.cc ../*/*.cc
cd ../commands
ctags ../*.h ../*/*.h ../*.cc ../*/*.cc
cd ../tools
ctags ../*.h ../*/*.h ../*.cc ../*/*.cc
