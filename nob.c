#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_FOLDER "build/"
#define SRC_FOLDER "src/"

bool add_all_files(Nob_Walk_Entry entry) {
    Nob_Cmd* cmd = (Nob_Cmd*)entry.data;
    if (entry.type != NOB_FILE_DIRECTORY) {
        size_t strlength = strlen(entry.path) + 1;
        char* new_name = malloc(strlength);
        if (new_name == NULL) return false;
        #ifdef _WIN32
        strcpy_s(new_name, strlength, entry.path);
        #else
        strcpy(new_name, entry.path);
        #endif // _WIN32
        nob_cc_inputs(cmd, new_name);
    }
    return true;
}

typedef struct {
    bool debug;
    char* runner;
} Args;

int main(int argc, char** argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);
    
    Args args = {0};
    for (size_t i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
            args.debug = true;
        } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--runWith") == 0) {
            i++;
            args.runner = argv[i];
        }
    }

    if (!nob_mkdir_if_not_exists(BUILD_FOLDER)) return 1;

    Nob_Cmd cmd = {0};

    nob_cc(&cmd);
    nob_cc_flags(&cmd);
    if (args.debug) nob_cmd_append(&cmd, "-g");
    nob_cc_output(&cmd, BUILD_FOLDER "cimpl");

    if (!nob_walk_dir("src", add_all_files, &cmd)) return 1;
    if (!nob_cmd_run(&cmd)) return 1;

    if (strlen(args.runner) > 0) nob_cmd_append(&cmd, args.runner);
    nob_cmd_append(&cmd, BUILD_FOLDER"cimpl");
    if (!nob_cmd_run(&cmd)) return 1;

    return 0;
}
