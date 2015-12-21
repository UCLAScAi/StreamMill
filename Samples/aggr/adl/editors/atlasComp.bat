@ECHO OFF
c:\Atlas\axl\adlc.EXE -fc:\Atlas\adlout.cc %1 %2 %3 %4
c:\cygwin\bin\g++ -oc:\Atlas\adlout.o -IC:\Atlas\include -g -c C:\Atlas\adlout.cc
c:\cygwin\bin\g++ -oc:\Atlas\adlout c:\Atlas\adlout.o c:\Atlas\libraries\libadl.a c:\Atlas\libraries\libdb.a c:\Atlas\libraries\libimdb.a c:\Atlas\libraries\librtree.a
pause