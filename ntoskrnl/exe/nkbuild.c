#include "nkbuild.h"
#include "pe.h"
#include "../kernel/kprintf.h"
#include <nk/string.h>

#define MAX_EXE_SIZE 8192
static uint8_t g_exe_buffer[MAX_EXE_SIZE];
static uint32_t g_exe_size = 0;
static uint32_t g_code_offset = 0;
static uint32_t g_code_size = 0;

static uint8_t* p_code = NULL;

void nkbuild_start(void) {
    memset(g_exe_buffer, 0, MAX_EXE_SIZE);
    
    // We will place headers at 0x0, followed by a Section header, followed by Code.
    // Total header size will be ~512 bytes (FileAlignment)
    g_code_offset = 512;
    g_code_size = 0;
    p_code = g_exe_buffer + g_code_offset;
    
    kprintf("NKBuild: New minimal PE build started.\n");
    kprintf("NKBuild: Type 'done' to finish linking.\n");
}

int nkbuild_process_line(const char* line) {
    // Basic assembler. We support a few instructions
    
    // Skip leading spaces
    while (*line == ' ' || *line == '\t') line++;
    
    if (strcmp(line, "done") == 0) {
        // Link and build headers
        IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)g_exe_buffer;
        dos->e_magic = IMAGE_DOS_SIGNATURE; // 'MZ'
        dos->e_lfanew = sizeof(IMAGE_DOS_HEADER); // Immediately follows

        IMAGE_NT_HEADERS32* nt = (IMAGE_NT_HEADERS32*)(g_exe_buffer + dos->e_lfanew);
        nt->Signature = IMAGE_NT_SIGNATURE; // 'PE\0\0'
        
        nt->FileHeader.Machine = 0x014c; // i386
        nt->FileHeader.NumberOfSections = 1;
        nt->FileHeader.SizeOfOptionalHeader = sizeof(IMAGE_OPTIONAL_HEADER32);
        nt->FileHeader.Characteristics = 0x0102; // EXECUTABLE_IMAGE | 32BIT_MACHINE

        nt->OptionalHeader.Magic = 0x010b; // PE32
        nt->OptionalHeader.AddressOfEntryPoint = 0x1000; // Text section virtual address
        nt->OptionalHeader.ImageBase = 0x01000000;
        nt->OptionalHeader.SectionAlignment = 0x1000;
        nt->OptionalHeader.FileAlignment = 0x200;
        nt->OptionalHeader.SizeOfImage = 0x2000;
        nt->OptionalHeader.SizeOfHeaders = 0x200;
        nt->OptionalHeader.Subsystem = 1; // Native (no windows subsystem needed)
        nt->OptionalHeader.NumberOfRvaAndSizes = 16;
        
        IMAGE_SECTION_HEADER* sec = (IMAGE_SECTION_HEADER*)((uint8_t*)nt + sizeof(IMAGE_NT_HEADERS32));
        strcpy((char*)sec->Name, ".text");
        sec->Misc.VirtualSize = g_code_size;
        sec->VirtualAddress = 0x1000; // Must match EntryPoint
        sec->SizeOfRawData = (g_code_size + 0x1FF) & ~0x1FF; // align to 512
        sec->PointerToRawData = g_code_offset;
        sec->Characteristics = 0x60000020; // READ | EXECUTE | CODE

        g_exe_size = g_code_offset + sec->SizeOfRawData;
        kprintf("NKBuild: Linked successfully. Size = %u bytes.\n", g_exe_size);
        return 0;
    }
    
    if (strncmp(line, "mov eax, ", 9) == 0) {
        uint32_t val = 0;
        const char* p = line + 9;
        while (*p >= '0' && *p <= '9') {
            val = val * 10 + (*p - '0');
            p++;
        }
        *p_code++ = 0xB8; // mov eax, imm32
        *p_code++ = (val & 0xFF);
        *p_code++ = ((val >> 8) & 0xFF);
        *p_code++ = ((val >> 16) & 0xFF);
        *p_code++ = ((val >> 24) & 0xFF);
        g_code_size += 5;
        return 1;
    }
    
    if (strcmp(line, "ret") == 0) {
        *p_code++ = 0xC3;
        g_code_size += 1;
        return 1;
    }

    if (strcmp(line, "int 3") == 0) {
        *p_code++ = 0xCC;
        g_code_size += 1;
        return 1;
    }

    if (line[0] != '\0') {
        kprintf("NKBuild Error: Unknown instruction '%s'\n", line);
    }
    return 1;
}

uint32_t nkbuild_get_exe_size(void) {
    return g_exe_size;
}

void* nkbuild_get_exe_buffer(void) {
    if (g_exe_size == 0) return (void*)0;
    return g_exe_buffer;
}
