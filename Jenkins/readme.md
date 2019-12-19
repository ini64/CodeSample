windows script

```console
SET "BUILD_TYPE=Internal"
SET "DOCKER_NAME=internal"
SET "RUN_PORT=15500"
SET DESIGN_SVN=https://172.20.103.4:8443/svn/Abyssworld/Design/trunk/Main/000_DataTable/%BUILD_TYPE%
SET DESIGN_DIR=D:\GameData\DesignData\%BUILD_TYPE%

SET META_SVN=https://172.20.103.4:8443/svn/Abyssworld/Server/Meta/%BUILD_TYPE%
SET META_DIR=D:\GameData\Meta\%BUILD_TYPE%

SET MAKER_DIR=D:\GameData\Maker\%BUILD_TYPE%

SET ASSET_SVN=https://172.20.103.4:8443/svn/Abyssworld/Client/trunk/Abyssworld_Framework/AssetBundles
SET ASSET_DIR=D:\ClientAssets\%BUILD_TYPE%

SVN %DESIGN_DIR% %DESIGN_SVN% checkOut
SVN %META_DIR% %META_SVN% checkOut
SVN %ASSET_DIR% %ASSET_SVN% checkOut


for %%e in (cs json) do (
    DEL %META_DIR%\*.%%e /f /s /q
)

for %%e in (cs json) do (
    XCOPY %DESIGN_DIR%\*.%%e %META_DIR% /s /y
)

robocopy %ASSET_DIR% %META_DIR%\clientAssets /MIR

for %%e in (cs json) do (
    DEL %MAKER_DIR%\Lib\*.%%e /f /s /q
)

for %%e in (cs json) do (
    XCOPY %DESIGN_DIR%\*.%%e %MAKER_DIR%\Lib /s /y
)

cd %MAKER_DIR%
dotnet clean
dotnet build -c Release > maker.log
type maker.log
findstr /m "error" maker.log
if %errorlevel%==0 (
exit 1
)

cd %MAKER_DIR%\bin\Release\netcoreapp2.2
dotnet Maker.dll %META_DIR% %BUILD_NUMBER% %BUILD_TYPE% autoPublish > publish.log
type publish.log
findstr /m "Fail" publish.log
if %errorlevel%==0 (
exit 1
)
Akamai %BUILD_TYPE%

SVN %META_DIR% %META_SVN% commit

DEL %WORKSPACE%\server\Lib\ServerLib\table\Code\*.cs /f /s /q
DEL %WORKSPACE%\server\AppServer\table\*.json /f /s /q
DEL %WORKSPACE%\server\AppServer\version\*.json /f /s /q
DEL %WORKSPACE%\server\Fiddler\table\*.json /f /s /q
DEL %WORKSPACE%\server\AdminTool\table\*.json /f /s /q
DEL %WORKSPACE%\server\XTest\XTest\table\*.json /f /s /q

COPY %META_DIR%\version\ServerMD5.json %WORKSPACE%\server\AppServer\version /y

XCOPY %META_DIR%\code\*.cs %WORKSPACE%\server\Lib\ServerLib\table\Code /s /y
XCOPY %META_DIR%\json\*.json %WORKSPACE%\server\AppServer\table /s /y
XCOPY %META_DIR%\json\*.json %WORKSPACE%\server\Fiddler\table /s /y
XCOPY %META_DIR%\json\*.json %WORKSPACE%\server\AdminTool\table /s /y
XCOPY %META_DIR%\json\*.json %WORKSPACE%\server\XTest\XTest\table /s /y

cd %WORKSPACE%
dotnet clean Abyssworld.sln
dotnet build -c Release Abyssworld.sln
dotnet publish server/AppServer/AppServer.csproj -o %WORKSPACE%\publish\running  -c Release
```