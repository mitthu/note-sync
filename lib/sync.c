/* Standard Library */
#include <stdio.h>
#include <stdlib.h>
/* Support Library */
#include <std.h>
#include <sync.h>
#include <err.h>

int genPatch(String old, String new, String patch) {
    char buf[LINE];
    
    sprintf(buf, "diff -u %s %s >%s", old, new, patch);
    if (system(buf) == -1) {
        err_log("Diff unsuccessful");
        return FAILURE;
    }
    return SUCCESS;
}

int applyPatch(String old, String patch) {
    char buf[LINE];
    
    sprintf(buf, "patch %s <%s", old, patch);
    if (system(buf) == -1) {
        err_log("Applying patch unsuccessful");
        return FAILURE;
    }
    return SUCCESS;
}

int removePatch(String new, String patch) {
    char buf[LINE];
    
    sprintf(buf, "patch -R %s <%s 1>/dev/null 2>/dev/null", new, patch);
    if (system(buf) == -1) {
        err_log("Patch removal failed");
        return FAILURE;
    }
    return SUCCESS;
}

int renameFile(String from_name, String to_name) {
    char buf[LINE];
    
    sprintf(buf, "mv %s %s", from_name, to_name);
    if (system(buf) == -1) {
        err_log("Rename failed");
        return FAILURE;
    }
    return SUCCESS;
}

int copyFile(String src, String trg) {
    char buf[LINE];
    
    sprintf(buf, "cp %s %s", src, trg);
    if (system(buf) == -1) {
        err_log("Copy failed");
        return FAILURE;
    }
    return SUCCESS;
}

int deleteFile(String file) {
    char buf[LINE];
    
    sprintf(buf, "rm -f %s", file);
    if (system(buf) == -1) {
        err_log("Couldn't delete file");
        return FAILURE;
    }
    return SUCCESS;
}
