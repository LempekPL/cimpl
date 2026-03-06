#include <stdio.h>
#define NOB_IMPLEMENTATION
#include "nob.h"
#include <string.h>
#include <stdio.h>

#define BUILD_FOLDER "build/"
#define SRC_FOLDER "src/"

void walk_the_dir(Nob_Cmd* cmd, char* name) {
    DIR* dir = opendir(name);
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        size_t lens = sizeof(char)*(strlen(name)+strlen(entry->d_name)+2);
        char* new_name = malloc(lens);
        snprintf(new_name, lens, "%s/%s", name, entry->d_name);
        if (entry->d_name[0] == '.') continue;
        if (entry->d_type == 4) {
            walk_the_dir(cmd, new_name);
        } else {
            nob_cc_inputs(cmd, new_name);
        }
    }
    closedir(dir);
}

int main(int argc, char** argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!nob_mkdir_if_not_exists(BUILD_FOLDER)) return 1;

    Nob_Cmd cmd = {0};

    nob_cc(&cmd);
    nob_cc_flags(&cmd);
    nob_cc_output(&cmd, BUILD_FOLDER "cimpl");
    walk_the_dir(&cmd, "src");
    if (!nob_cmd_run(&cmd)) return 1;

    nob_cmd_append(&cmd, BUILD_FOLDER"cimpl");
    if (!nob_cmd_run(&cmd)) return 1;


    return 0;
}