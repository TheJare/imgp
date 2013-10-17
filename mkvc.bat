IF NOT EXIST bin mkdir bin
cl src\Image.cpp src\ImagePacker.cpp src\Rect.cpp src\GuillotineBinPack.cpp src\main.cpp /EHsc /MT /O2 /link setargv.obj /subsystem:console /OUT:bin/imgp.exe
    
