# SMP FaceGen
A tool to fix Skyrim face data files containing physics parts
## Requirements
Microsoft Windows with Visual C++ Runtime Libraries
## Installation
Download the archive and extract it to any location. To uninstall, just delete the files.
## Use
Generate face data normally with the Creation Kit (you may need [CKPE](https://www.nexusmods.com/skyrimspecialedition/mods/71371) for this). Store these files in a temporary location. Subfolders are allowed.

In "paths.txt", enter the path to your Skyrim "data" folder (or some other location where you store the original model files) after "data=" and the location of your temporary face data files after "in=". After "out=", enter the path to where you want the fixed files to be written. This should be a temporary location, since anything in it may be overwritten without warning. Absolute and relative paths are allowed. Paths should be in UTF-8 encoding.

Run SMP FaceGen.exe. Fixed face data files will be written to the 'out' location, keeping the names and folder structure of 'in'. No input files are modified, but (again!) *existing files in the 'out' location will be overwritten without warning*.

The program adds missing HDT-SMP physics XML paths and corrects bone hierarchies and rest poses that the Creation Kit gets wrong. If, for any reason, you don't want a bone to be modified, add its name to "exclude.txt". 

Affected head parts are listed in "head parts.txt", along with their corresponding NIF file. This list can be edited as desired.
## Credits
This tool uses [nifly](https://github.com/ousnius/nifly).
