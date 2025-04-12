#ifndef FILES_H
#define FILES_H   

#include <shlobj.h> 
#include <shobjidl.h> 
#include <string>
#include <windows.h>
#include <commdlg.h>
#include <filesystem>


std::string OpenFileDialog();
std::string OpenFolderDialogWithIFileDialog();

#endif // FILES_H