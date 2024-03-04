# SMP FaceGen
A tool to fix Skyrim face data files containing physics parts
## Requirements
Microsoft Windows with Visual C++ Runtime Libraries
## Installation
Download the archive and extract it to any location. To uninstall, just delete the files.
## Use
Generate face data normally with the Creation Kit (you may need [CKPE](https://www.nexusmods.com/skyrimspecialedition/mods/71371) for this). Place these files in the 'in' subfolder. Subfolders are allowed.

Place original head part NIF files in the 'ref' subfolder and rename them to match the EditorID of the corresponding head part (i.e. the name of the hairstyle/beard you see in the Creation Kit). Subfolders are *not* allowed. Only NIF files of head parts with physics are needed here.

Run SMP FaceGen.exe. Fixed face data files will be written to the 'out' subfolder, keeping the names and folder structure of 'in'. No input files are modified, but existing files in the 'out' folder will be overwritten without warning.

The program adds missing HDT-SMP physics XML paths and corrects bone hierarchies and rest poses that the Creation Kit gets wrong. If, for any reason, you don't want a bone to be modified, add its name to exclude.txt.
## Credits
This tool uses [nifly](https://github.com/ousnius/nifly).
