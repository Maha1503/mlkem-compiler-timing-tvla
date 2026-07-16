@echo off
mkdir bin 2>nul
for %%P in (0 1 2) do (
    for %%M in (0 1) do (
        for %%O in (Od O1 O2 Ox) do (
            echo Building msvc %%O p%%P m%%M...
            cl /%%O -DPRIMITIVE=%%P -DMODE=%%M src\harness.c /Febin\msvc_%%O_p%%P_m%%M.exe
        )
    )
)