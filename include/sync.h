#ifndef sync_h
#define sync_h

#include "std.h"

int genPatch(String old, String new, String patch);
int applyPatch(String old, String patch);
int removePatch(String new, String patch);

int renameFile(String from_name, String to_name);
int copyFile(String src, String trg);
int deleteFile(String file);

#endif
