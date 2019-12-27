@echo off
cd .
copy src\SYSTEM.CNF bin\SYSTEM.CNF
copy obj\MAIN.EXE bin\MAIN.EXE
gencti obj\disc.cti /dir=bin /system=C:\psyq\cdgen\LCNSFILE\licensee.dat
buildcd -l -icd\disc.img obj\disc.cti
stripiso s 2352 cd\disc.img cd\disc.iso
psxlicense /eu /i cd\disc.iso
