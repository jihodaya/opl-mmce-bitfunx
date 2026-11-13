/* POPStarter handoff via the live fileXio server. Based on PS2SDK's ELF loader (AFL-2.0). */
#include <elf.h>
#include <fileio.h>
#include <fileXio_rpc.h>
#include <kernel.h>
#include <sifrpc.h>
#include <stdio.h>
#include <string.h>

#define USER_RAM_START 0x00100000
#define MAX_ELF_HEADERS 32

void _libcglue_init(void) {}
void _libcglue_deinit(void) {}
void _libcglue_args_parse(int argc, char **argv) {}
void _ps2sdk_fileXio_init(void) {}
void _ps2sdk_fileXio_deinit(void) {}
void _ps2sdk_fileXio_set_block_size(int size) {}

static int readAt(int fd, u32 offset, void *buffer, u32 size, u32 fileSize)
{
    if (offset > fileSize || size > fileSize - offset || fileXioLseek(fd, offset, SEEK_SET) < 0)
        return -1;
    u8 *out = buffer;
    while (size > 0) {
        int count = fileXioRead(fd, out, size);
        if (count <= 0)
            return -1;
        out += count;
        size -= count;
    }
    return 0;
}

static int loadElf(int fd, u32 *entry, u32 *gp)
{
    Elf32_Ehdr header;
    Elf32_Phdr segments[MAX_ELF_HEADERS];
    int fileSize = fileXioLseek(fd, 0, SEEK_END);
    if (fileSize < 0 || readAt(fd, 0, &header, sizeof(header), fileSize) < 0 ||
        memcmp(header.e_ident, ELFMAG, SELFMAG) != 0 || header.e_ident[EI_CLASS] != ELFCLASS32 ||
        header.e_ident[EI_DATA] != ELFDATA2LSB || header.e_machine != EM_MIPS || header.e_type != ET_EXEC ||
        header.e_phnum == 0 || header.e_phnum > MAX_ELF_HEADERS || header.e_phentsize != sizeof(segments[0]) ||
        readAt(fd, header.e_phoff, segments, header.e_phnum * sizeof(segments[0]), fileSize) < 0)
        return -1;

    u32 ramEnd = GetMemorySize();
    int validEntry = 0;
    for (int i = 0; i < header.e_phnum; i++) {
        const Elf32_Phdr *segment = &segments[i];
        if (segment->p_type != PT_LOAD || segment->p_memsz == 0)
            continue;
        if (segment->p_vaddr < USER_RAM_START || segment->p_vaddr >= ramEnd ||
            segment->p_memsz > ramEnd - segment->p_vaddr || segment->p_filesz > segment->p_memsz)
            return -1;
        if ((segment->p_flags & PF_X) && header.e_entry >= segment->p_vaddr &&
            header.e_entry - segment->p_vaddr < segment->p_filesz)
            validEntry = 1;
    }
    if (!validEntry)
        return -1;

    memset((void *)USER_RAM_START, 0, ramEnd - USER_RAM_START);
    for (int i = 0; i < header.e_phnum; i++) {
        const Elf32_Phdr *segment = &segments[i];
        if (segment->p_type == PT_MIPS_REGINFO && segment->p_filesz >= sizeof(Elf32_RegInfo)) {
            Elf32_RegInfo info;
            if (readAt(fd, segment->p_offset, &info, sizeof(info), fileSize) < 0)
                return -1;
            *gp = info.ri_gp_value;
        } else if (segment->p_type == PT_LOAD && segment->p_filesz > 0 &&
                   readAt(fd, segment->p_offset, (void *)segment->p_vaddr, segment->p_filesz, fileSize) < 0)
            return -1;
    }
    if (*gp == 0 && header.e_shentsize == sizeof(Elf32_Shdr) && header.e_shoff <= fileSize &&
        header.e_shnum <= (fileSize - header.e_shoff) / sizeof(Elf32_Shdr)) {
        for (int i = 0; i < header.e_shnum; i++) {
            Elf32_Shdr section;
            if (readAt(fd, header.e_shoff + i * sizeof(section), &section, sizeof(section), fileSize) < 0)
                return -1;
            if (section.sh_type == SHT_MIPS_REGINFO && section.sh_size >= sizeof(Elf32_RegInfo)) {
                Elf32_RegInfo info;
                if (readAt(fd, section.sh_offset, &info, sizeof(info), fileSize) < 0)
                    return -1;
                *gp = info.ri_gp_value;
                break;
            }
        }
    }
    *entry = header.e_entry;
    return 0;
}

int main(int argc, char **argv)
{
    u32 entry = 0, gp = 0;
    int result = -1;
    if (argc == 2 && fileXioInit() >= 0) {
        int fd = fileXioOpen(argv[0], FIO_O_RDONLY);
        if (fd >= 0) {
            result = loadElf(fd, &entry, &gp);
            fileXioClose(fd);
        }
        fileXioExit();
    }

    SifExitRpc();
    if (result == 0) {
        FlushCache(0);
        FlushCache(2);
        ExecPS2((void *)entry, (void *)gp, 1, &argv[1]);
    }
    LoadExecPS2("", 0, NULL);
    return -1;
}
