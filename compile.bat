@echo off

if exist "editme.o" del editme.o
if exist "libeditme.a" del libeditme.a

gcc.exe -Wall -Wextra -pedantic -Wpedantic -Ofast -c editme.c -std=c11

ar.exe -rU -o libeditme.a editme.o

if exist "editme.o" del editme.o


pause