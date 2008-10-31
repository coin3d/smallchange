@echo off

pushd %COINDIR%

if exist bin\*.* goto binexists
echo mkdir %COINDIR%\bin
mkdir bin
:binexists
if exist lib\*.* goto libexists
echo mkdir %COINDIR%\lib
mkdir lib
:libexists
if exist include\*.* goto includeexists
echo mkdir %COINDIR%\include
mkdir include
:includeexists
chdir include
if exist SmallChange\*.* goto smallchangeexists
echo mkdir %COINDIR%\include\SmallChange
mkdir SmallChange
:smallchangeexists
chdir SmallChange
if exist actions\*.* goto actionsexists
echo mkdir %COINDIR%\include\SmallChange\actions
mkdir actions
:actionsexists
if exist draggers\*.* goto draggersexists
echo mkdir %COINDIR%\include\SmallChange\draggers
mkdir draggers
:draggersexists
if exist elements\*.* goto elementsexists
echo mkdir %COINDIR%\include\SmallChange\elements
mkdir elements
:elementsexists
if exist engines\*.* goto enginesexists
echo mkdir %COINDIR%\include\SmallChange\engines
mkdir engines
:enginesexists
if exist eventhandlers\*.* goto eventhandlersexists
echo mkdir %COINDIR%\include\SmallChange\eventhandlers
mkdir eventhandlers
:eventhandlersexists
if exist misc\*.* goto miscexists
echo mkdir %COINDIR%\include\SmallChange\misc
mkdir misc
:miscexists
if exist nodekits\*.* goto nodekitsexists
echo mkdir %COINDIR%\include\SmallChange\nodekits
mkdir nodekits
:nodekitsexists
if exist nodes\*.* goto nodesexists
echo mkdir %COINDIR%\include\SmallChange\nodes
mkdir nodes
:nodesexists

popd

