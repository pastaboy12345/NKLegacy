#include "pe.h"
#include "../kernel/kprintf.h"
#include <nk/string.h>

int pe_load_and_run(void* file_buffer, uint32_t file_size) {
    if (!file_buffer || file_size < sizeof(IMAGE_DOS_HEADER)) {
        kprintf("PE: Invalid buffer size\n");
        return -1;
    }

    IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)file_buffer;
    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
        kprintf("PE: Invalid DOS signature (0x%x)\n", dos_header->e_magic);
        return -1;
    }

    if (dos_header->e_lfanew >= file_size || (dos_header->e_lfanew + sizeof(IMAGE_NT_HEADERS32)) > file_size) {
        kprintf("PE: Invalid NT headers offset\n");
        return -1;
    }

    IMAGE_NT_HEADERS32* nt_headers = (IMAGE_NT_HEADERS32*)((uint8_t*)file_buffer + dos_header->e_lfanew);
    
    if (nt_headers->Signature != IMAGE_NT_SIGNATURE) {
        kprintf("PE: Invalid NT signature (0x%x)\n", nt_headers->Signature);
        return -1;
    }

    uint16_t machine = nt_headers->FileHeader.Machine;
    if (machine != 0x014C) { // i386
        kprintf("PE: Invalid Machine type (0x%x). Only i386 supported.\n", machine);
        return -1;
    }

    uint32_t entry_point_rva = nt_headers->OptionalHeader.AddressOfEntryPoint;
    uint32_t image_base = nt_headers->OptionalHeader.ImageBase;

    kprintf("PE: Loading %d sections to ImageBase 0x%x...\n", nt_headers->FileHeader.NumberOfSections, image_base);

    IMAGE_SECTION_HEADER* section_headers = (IMAGE_SECTION_HEADER*)((uint8_t*)nt_headers + sizeof(IMAGE_NT_HEADERS32));

    for (int i = 0; i < nt_headers->FileHeader.NumberOfSections; i++) {
        IMAGE_SECTION_HEADER* section = &section_headers[i];
        
        uint32_t dest = image_base + section->VirtualAddress;
        uint32_t src = (uint32_t)file_buffer + section->PointerToRawData;
        uint32_t size = section->SizeOfRawData;

        if (size > 0 && dest != src) {
            // Unsafe direct copy for now (requires identity mapped physical memory)
            memcpy((void*)dest, (void*)src, size);
        }
    }

    uint32_t entry_point = image_base + entry_point_rva;
    kprintf("PE: Jmp to EntryPoint: 0x%x\n", entry_point);

    // Call the entry point
    void (*app_main)(void) = (void(*)(void))entry_point;
    app_main();

    kprintf("\nPE: Application exited.\n");
    return 0;
}
