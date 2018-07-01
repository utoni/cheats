@ECHO OFF
SET PATH=%PATH%;..\libghack\CSFML-2.1-windows-32bits\CSFML-2.1\bin;..\libghack\CSFML-2.1-windows-64bits\CSFML-2.1\bin;..\libghack
IF NOT "%1" == "" (
.\%1
)
IF "%1" == "" (
.\bin\h1z1_hrad.exe
)
