@echo off

pushd %~dp0
cls
set BINPLATFORM=Win32
set BINCONFIG=Release
set SLEEPPATH="C:\WFA\CLIs\Intel\sleep.exe"

start dut.cmd
%SLEEPPATH% 1
start ca.cmd
