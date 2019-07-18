if not exist ".\include\" mkdir ".\include\"
xcopy ".\src\*.h*" ".\include\" /D /E /I /F /Y
rmdir /s /q "./include/servercore/thirdparty"