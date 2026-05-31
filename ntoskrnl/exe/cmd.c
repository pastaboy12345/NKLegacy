#include "cmd.h"
#include "../driver/keyboard/keyboard.h"
#include "../driver/serial/serial.h"
#include "../driver/vga/vga.h"
#include "../fs/nkfs/nkfs.h"
#include "../kernel/kprintf.h"
#include "nkbuild.h"
#include "pe.h"
#include <nk/string.h>

static nkfs_node_t* cwd_node = (void*)0;
static char cwd_path[128] = "C:";
#include <nk/string.h>

static void cmd_execute(char *cmd) {
  if (cmd[0] == '\0')
    return;

  if (strcmp(cmd, "help") == 0) {
    kprintf("CD             Displays the name of or changes the current directory.\n");
    kprintf("CLS            Clears the screen.\n");
    kprintf("DIR            Displays a list of files and subdirectories in a directory.\n");
    kprintf("ECHO           Displays messages or directs output to files.\n");
    kprintf("MKDIR          Creates a directory.\n");
    kprintf("TYPE           Displays the contents of a text file.\n");
    kprintf("VER            Displays the Windows version.\n");
    kprintf("BUILD          Starts the NKBuild minimal PE builder.\n");
    kprintf("RUN            Executes the built PE binary.\n");
  } else if (strcmp(cmd, "cls") == 0) {
    vga_clear();
    vga_set_cursor(0, 0); 
  } else if (strcmp(cmd, "ver") == 0) {
    kprintf("\nMicrosoft Windows [Version NKLegacy]\n");
  } else if (strcmp(cmd, "dir") == 0) {
    kprintf("\n Directory of %s\n\n", cwd_path);
    if (cwd_node && cwd_node->type == NKFS_DIR) {
        nkfs_node_t* child = cwd_node->first_child;
        int count = 0;
        while (child) {
            kprintf("%s\t%s\t%d\n", child->type == NKFS_DIR ? "<DIR>" : "     ", child->name, child->size);
            child = child->next_sibling;
            count++;
        }
        kprintf("               %d File(s)\n", count);
    }
  } else if (strncmp(cmd, "cd ", 3) == 0) {
    char* dir = cmd + 3;
    if (strcmp(dir, "..") == 0) {
        if (cwd_node && cwd_node->parent) {
            cwd_node = cwd_node->parent;
            int len = strlen(cwd_path);
            while (len > 0 && cwd_path[len-1] != '\\') len--;
            if (len > 0) cwd_path[len-1] = '\0';
            if (cwd_path[0] == '\0') strcpy(cwd_path, "C:");
        }
    } else {
        nkfs_node_t* child = nkfs_find_child(cwd_node, dir);
        if (child && child->type == NKFS_DIR) {
            cwd_node = child;
            if (strcmp(cwd_path, "C:") == 0) {
                strcpy(cwd_path, "C:\\");
                strcat(cwd_path, dir);
            } else {
                strcat(cwd_path, "\\");
                strcat(cwd_path, dir);
            }
        } else {
            kprintf("The system cannot find the path specified.\n");
        }
    }
  } else if (strncmp(cmd, "mkdir ", 6) == 0 || strncmp(cmd, "md ", 3) == 0) {
    char* dir = (cmd[1] == 'k') ? cmd + 6 : cmd + 3;
    if (!nkfs_mkdir(cwd_node, dir)) kprintf("A subdirectory or file %s already exists.\n", dir);
  } else if (strncmp(cmd, "type ", 5) == 0) {
    char* file = cmd + 5;
    nkfs_node_t* child = nkfs_find_child(cwd_node, file);
    if (child && child->type == NKFS_FILE) {
        for (uint32_t i = 0; i < child->size; i++) {
            vga_putchar(child->data[i]);
            serial_putchar(COM1, child->data[i]);
            if (child->data[i] == '\n') serial_putchar(COM1, '\r');
        }
        kprintf("\n");
    } else {
        kprintf("The system cannot find the file specified.\n");
    }
  } else if (strncmp(cmd, "echo ", 5) == 0) {
    char* text = cmd + 5;
    char* redir = (void*)0;
    for (int i = 0; text[i]; i++) {
        if (text[i] == '>') { redir = &text[i]; break; }
    }
    if (redir) {
        *redir = '\0';
        redir++;
        while (*redir == ' ') redir++;
        int tlen = strlen(text);
        while (tlen > 0 && text[tlen-1] == ' ') { text[tlen-1] = '\0'; tlen--; }
        nkfs_node_t* f = nkfs_create_file(cwd_node, redir);
        if (f) nkfs_write_file(f, text, strlen(text));
        else kprintf("Access denied.\n");
    } else {
        kprintf("%s\n", text);
    }
  } else if (strcmp(cmd, "echo") == 0) {
    kprintf("ECHO is on.\n");
  } else if (strcmp(cmd, "build") == 0) {
    kprintf("NKBuild: Cannot run 'build' inside execute normally. Call nkbuild from main loop.\n");
  } else if (strcmp(cmd, "run") == 0) {
    void *buf = nkbuild_get_exe_buffer();
    uint32_t size = nkbuild_get_exe_size();
    if (buf && size > 0) pe_load_and_run(buf, size);
    else kprintf("No executable built yet. Run 'build' first.\n");
  } else {
    kprintf("'%s' is not recognized as an internal or external command,\noperable program or batch file.\n", cmd);
  }
}

void cmd_run(void) {
  char cmd_buf[128];
  int cmd_i = 0;
  int build_mode = 0;

  vga_set_color(0x07);
  kprintf("\nMicrosoft Windows [Version NKLegacy]\n");
  kprintf("(c) William Beauregard. All rights reserved.\n\n");

  cwd_node = nkfs_get_root();

  for (;;) {
    if (!build_mode) {
      vga_set_color(0x07);
      if (strcmp(cwd_path, "C:") == 0) {
         vga_print("C:\\>");
      } else {
         vga_print(cwd_path);
         vga_print(">");
      }
    } else {
      vga_set_color(0x0C);
      vga_print("nkbuild> ");
      vga_set_color(0x07);
    }

    cmd_i = 0;
    for (;;) {
      char c = keyboard_getchar();
      if (!c)
        c = serial_getchar(COM1);

      if (c) {
        if (c == '\n' || c == '\r') {
          vga_putchar('\n');
          cmd_buf[cmd_i] = '\0';
          break;
        } else if (c == '\b') {
          if (cmd_i > 0) {
            cmd_i--;
            vga_putchar('\b');
            vga_putchar(' ');
            vga_putchar('\b');
          }
        } else {
          if (cmd_i < 127) {
            cmd_buf[cmd_i++] = c;
            vga_putchar(c);
          }
        }
      } else {
        __asm__ volatile("pause");
      }
    }

    if (build_mode) {
      if (strcmp(cmd_buf, "done") == 0) {
        nkbuild_process_line(cmd_buf);
        build_mode = 0;
      } else {
        nkbuild_process_line(cmd_buf);
      }
    } else {
      if (strcmp(cmd_buf, "build") == 0) {
        build_mode = 1;
        nkbuild_start();
      } else {
        cmd_execute(cmd_buf);
      }
    }
  }
}
