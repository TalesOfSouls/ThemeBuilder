#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RELEASE 1

#if _WIN32
    #include <windows.h>
#else
    #include <dirent.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <limits.h>
#endif

#include "../cOMS/ui/UITheme.h"
#include "../cOMS/ui/UILayout.h"
#include "../cOMS/ui/UILayout.cpp"
#include "../cOMS/utils/StringUtils.h"

UIThemeStyle* themes;
UILayout* layouts;
int theme_index = 0;
int layout_index = 0;

#if _WIN32
void iter_directories_recursive(RingMemory* ring, const char *dir_path) {
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char path[MAX_PATH];
    char searchPath[MAX_PATH];

    snprintf(searchPath, sizeof(searchPath), "%s\\*", dir_path);

    hFind = FindFirstFileA(searchPath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "FindFirstFile failed (%d)\n", GetLastError());
        fprintf(stderr, "%s\n", searchPath);
        return;
    }

    do {
        if (findFileData.cFileName[0] == '.') {
            continue;
        }

        snprintf(path, sizeof(path), "%s\\%s", dir_path, findFileData.cFileName);

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            iter_directories_recursive(ring, path);
        } else if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
            const char *ext = strrchr(findFileData.cFileName, '.');

            if (ext && strcmp(ext, ".themetxt") == 0) {
                char abs_path[MAX_PATH];
                if (GetFullPathNameA(path, sizeof(abs_path), abs_path, NULL)) {
                    printf("Found .themetxt file: %s\n", abs_path);

                    themes[theme_index].data = (byte *) calloc(10, MEGABYTE);
                    theme_from_file_txt(&themes[theme_index], abs_path, ring);

                    FileBody output = {};
                    output.content = (byte *) calloc(10, MEGABYTE);
                    output.size = theme_to_data(&themes[theme_index], output.content);

                    char new_path[MAX_PATH];
                    str_replace(abs_path, ".themetxt", ".themebin", new_path);
                    file_write(new_path, &output);

                    free(themes[theme_index].data);
                    free(output.content);

                    ++theme_index;
                }
            } else if (ext && strcmp(ext, ".layouttxt") == 0) {
                char abs_path[MAX_PATH];
                if (GetFullPathNameA(path, sizeof(abs_path), abs_path, NULL)) {
                    printf("Found .layouttxt file: %s\n", abs_path);

                    layouts[layout_index].data = (byte *) calloc(10, MEGABYTE);
                    layout_from_file_txt(&layouts[layout_index], abs_path, ring);

                    FileBody output = {};
                    output.content = (byte *) calloc(10, MEGABYTE);
                    output.size = layout_to_data(&layouts[layout_index], output.content);

                    char new_path[MAX_PATH];
                    str_replace(abs_path, ".layouttxt", ".layoutbin", new_path);
                    file_write(new_path, &output);

                    free(layouts[layout_index].data);
                    free(output.content);

                    ++layout_index;
                }
            }
        }
    } while (FindNextFileA(hFind, &findFileData) != 0);

    FindClose(hFind);
}
#else
void iter_directories_recursive(RingMemory* ring, const char *dir_path) {
    struct dirent *entry;
    DIR *dir = opendir(dir_path);

    if (!dir) {
        fprintf(stderr, "Could not open directory: %s\n", dir_path);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        char path[PATH_MAX];
        struct stat statbuf;

        if (entry->d_name == '.') {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);
        if (stat(path, &statbuf) != 0) {
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            iter_directories_recursive(ring, path);
        } else if (S_ISREG(statbuf.st_mode)) {
            const char *ext = strrchr(entry->d_name, '.');
            if (ext && strcmp(ext, ".themetxt") == 0) {
                char abs_path[PATH_MAX];
                if (realpath(path, abs_path)) {
                    printf("Found .themetxt file: %s\n", abs_path);

                    themes[theme_index].data = (byte *) calloc(10, MEGABYTE);
                    theme_from_file_txt(themes + theme_index, abs_path, ring);

                    FileBody output = {};
                    output.content = (byte *) calloc(10, MEGABYTE);
                    theme_to_data(themes + theme_index, output.content);

                    char new_path[MAX_PATH];
                    str_replace(abs_path, ".themetxt", ".themebin", new_path);
                    file_write(new_path, &output);

                    free(themes[theme_index].data);
                    free(output.content);

                    ++theme_index;
                }
            } else if (ext && strcmp(ext, ".layouttxt") == 0) {
                char abs_path[PATH_MAX];
                if (realpath(path, abs_path)) {
                    printf("Found .layouttxt file: %s\n", abs_path);

                    layouts[layout_index].data = (byte *) calloc(10, MEGABYTE);
                    layout_from_file_txt(layouts + layout_index, abs_path, ring);

                    FileBody output = {};
                    output.content = (byte *) calloc(10, MEGABYTE);
                    layout_to_data(layouts + layout_index, output.content);

                    char new_path[MAX_PATH];
                    str_replace(abs_path, ".layouttxt", ".layoutbin", new_path);
                    file_write(new_path, &output);

                    free(layouts[layout_index].data);
                    free(output.content);

                    ++layout_index;
                }
            }
        }
    }

    closedir(dir);
}
#endif

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    RingMemory memory_volatile = {};
    memory_volatile.memory = (byte *) malloc(sizeof(byte) * MEGABYTE * 128);
    memory_volatile.size = sizeof(byte) * MEGABYTE * 128;

    int theme_count = 1000;
    themes = (UIThemeStyle *) malloc(sizeof(UIThemeStyle) * theme_count);

    int layout_count = 1000;
    layouts = (UILayout *) malloc(sizeof(UILayout) * layout_count);

    iter_directories_recursive(&memory_volatile, argv[1]);

    printf("Layouts %d\n", layout_index);
    printf("Themes %d\n", theme_index);
}
