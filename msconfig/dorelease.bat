@echo off
setlocal
if exist %1 goto process1
echo Error - directory %1 does not exist
echo Usage: dorelease.bat path_to_release_subdirectory
goto end
:process1
if exist ..\Release goto process2
echo Compile a Release Build of Condor first.
goto end
:process2
REM Set up environment
call set_vars.bat

echo Creating Release Directory...
if not exist %1\bin\NUL mkdir %1\bin
if not exist %1\lib\NUL mkdir %1\lib
if not exist %1\lib\webservice\NUL mkdir %1\lib\webservice
if not exist %1\etc\NUL mkdir %1\etc
if not exist %1\sql\NUL mkdir %1\sql
if not exist %1\src\NUL mkdir %1\src
if not exist %1\src\chirp\NUL mkdir %1\src\chirp
REM if not exist %1\profiles\NUL mkdir %1\profiles

echo. & echo Copying root Condor files...
copy ..\Release\*.exe %1\bin
copy ..\Release\*.dll %1\bin
copy msvcrt.dll %1\bin
copy msvcirt.dll %1\bin
copy pdh.dll %1\bin
copy ..\src\condor_vm-gahp\condor_vm_vmware.pl %1\bin
copy ..\src\condor_vm-gahp\*.dll %1\bin
copy ..\src\condor_vm-gahp\mkisofs.exe %1\bin

echo. & echo Copying Chirp files...
copy ..\src\condor_starter.V6.1\*.class %1\lib
copy ..\src\condor_starter.V6.1\*.jar %1\lib
copy ..\src\condor_chirp\Chirp.jar %1\lib
copy ..\src\condor_chirp\chirp_* %1\src\chirp
copy ..\src\condor_chirp\PROTOCOL %1\src\chirp
copy ..\src\condor_chirp\chirp\LICENSE %1\src\chirp
copy ..\src\condor_chirp\chirp\doc\Condor %1\src\chirp\README

echo. & echo Copying example configurations...
copy ..\src\condor_examples\condor_config.* %1\etc
copy ..\src\condor_examples\condor_vmgahp_config.vmware %1\etc

echo. & echo Copying SQL files...
copy ..\src\condor_tt\*.sql %1\sql

echo. & echo Copying WSDL files...
pushd .
cd ..\src
for /R %%f in (*.wsdl) do copy %%f %1\lib\webservice
popd

echo. & echo Copying symbol files...
for %%f in (master startd quill dbmsd had credd schedd collector negotiator shadow starter) do (
    copy /v ..\Release\condor_%%f.pdb %1\bin
)

echo. & echo Making some aliases...
pushd %1\bin
copy condor_rm.exe condor_hold.exe
copy condor_rm.exe condor_release.exe
copy condor_rm.exe condor_vacate_job.exe
copy condor.exe condor_off.exe
copy condor.exe condor_on.exe
copy condor.exe condor_restart.exe
copy condor.exe condor_reconfig.exe
copy condor.exe condor_reschedule.exe
copy condor.exe condor_vacate.exe
copy condor_cod.exe condor_cod_request.exe

echo. & echo Copying DRMAA files...
cd ..
if not exist include\NUL mkdir include
copy %EXT_INSTALL%\%EXT_DRMAA_VERSION%\include\* include
copy %EXT_INSTALL%\%EXT_DRMAA_VERSION%\lib\* lib
if not exist src\drmaa\NUL mkdir src\drmaa
copy %EXT_INSTALL%\%EXT_DRMAA_VERSION%\src\* src\drmaa
popd
:end
