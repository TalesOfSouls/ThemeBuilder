#include "../cOMS/ui/UITheme.cpp"
#include "../cOMS/ui/UILayout.cpp"
#include "../cOMS/utils/StringUtils.h"

UITheme* themes;
UILayout* layouts;
int theme_index = 0;
int layout_index = 0;

void build_theme(const char* path, RingMemory* const ring) {
    UITheme theme = {0};
    theme.data_size = 5 * MEGABYTE;
    theme.data = memory_get(ring, theme.data_size, alignof(size_t));

    theme_from_file_txt(&theme, path, ring);

    FileBody output = {0};
    output.content = memory_get(ring, 5 * MEGABYTE, alignof(size_t));
    output.size = theme_to_data(&theme, output.content);

    char new_path[PATH_MAX_LENGTH];
    str_replace(path, ".themetxt", ".themebin", new_path);
    file_write(new_path, &output);
}

void build_layout(const char* path, RingMemory* const ring) {
    UILayout layout = {0};
    layout.data_size = 5 * MEGABYTE;
    layout.data = memory_get(ring, layout.data_size, alignof(size_t));

    layout_from_file_txt(&layout, path, ring);

    FileBody output = {0};
    output.content = memory_get(ring, 5 * MEGABYTE, alignof(size_t));
    output.size = layout_to_data(&layout, output.content);

    char new_path[PATH_MAX_LENGTH];
    str_replace(path, ".layouttxt", ".layoutbin", new_path);
    file_write(new_path, &output);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input>\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("Generating theme/layout...\n");

    RingMemory memory_volatile = {0};
    ring_alloc(&memory_volatile, MEGABYTE * 11, MEGABYTE * 11);

    char* extension = strrchr(argv[1], '.');

    if (strncmp(extension, ".themetxt", sizeof(".themetxt") - 1) == 0) {
        build_theme(argv[1], &memory_volatile);
        printf("\x1b[32mCreated theme %s\x1b[0m\n", argv[1]);
    } else if (strncmp(extension, ".layouttxt", sizeof(".layouttxt") - 1) == 0) {
        build_layout(argv[1], &memory_volatile);
        printf("\x1b[32mCreated layout %s\x1b[0m\n", argv[1]);
    } else {
        printf("\x1b[31mUnsupported file extension %s\x1b[0m\n", argv[1]);
    }

    return 0;
}
