// Copyright 2010       Sven Peter <svenpeter@gmail.com>
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

//
// Thanks to xorloser for his selftool!
// (see xorloser.com)
//


#include <stdio.h>
#include <string.h>
#include "tools.h"

static u8 *self;
static u8 *elf;

static struct elf_hdr ehdr;

static int arch64;
static u32 meta_offset;
static u64 elf_offset;
static u64 elf2_offset;
static u64 phdr_offset;
static u64 shdr_offset;
static u64 filesize;
static u64 authid;
static u64 app_version;
static u32 app_type;
static u16 sdk_type;


struct id2name_tbl {
	u32 id;
	const char *name;
};

static const char *id2name(u32 id, struct id2name_tbl *t, const char *unk)
{
	while (t->name != NULL) {
		if (id == t->id)
			return t->name;
		t++;
	}
	return unk;
}

struct id2name_tbl t_sdk_type[] = {
	{0, "Retail (Type 0)"},
	{1, "Retail"},
	{2, "Retail (Type 1)"},
	{0x8000, "Devkit"},
	{0, NULL}
};

struct id2name_tbl t_app_type[] = {
	{1, "level 0"},
	{2, "level 1"},
	{3, "level 2"},
	{4, "application"},
	{5, "isolated SPU module"},
	{6, "secure loader"},
	{8, "NP-DRM application"},
	{0, NULL}
};

static struct id2name_tbl t_shdr_type[] = {
	{0, "NULL"},
	{1, "PROGBITS"},
	{2, "SYMTAB"},
	{3, "STRTAB"},
	{4, "RELA"},
	{5, "HASH"},
	{6, "DYNAMIC"},
	{7, "NOTE"},
	{8, "NOBITS"},
	{9, "REL"},
	{10, "SHLIB"},
	{11, "DYNSYM"},
	{12, NULL},
};

static struct id2name_tbl t_elf_type[] = {
	{ET_NONE, "None"},
	{ET_REL, "Relocatable file"},
	{ET_EXEC, "Executable file"},
	{ET_DYN, "Shared object file"},
	{ET_CORE, "Core file"},
	{0, NULL}
};

static struct id2name_tbl t_elf_machine[] = {
	{20, "PowerPC"},
	{21, "PowerPC64"},
	{23, "SPE"},
	{0, NULL}
};


static struct id2name_tbl t_phdr_type[] = {
	{0, "NULL"},
	{1, "LOAD"},
	{2, "DYN"},
	{3, "INTPR"},
	{4, "NOTE"},
	{5, "SHLIB"},
	{6, "PHDR"},
	{0, NULL}
};

static void parse_self(void)
{
	meta_offset = be32(self + 12);
	elf_offset = be64(self + 48);
	elf2_offset = be64(self + 0x10);
	phdr_offset = be64(self + 56) - elf_offset;
	shdr_offset = be64(self + 64) - elf_offset;
	filesize = be64(self + 24);
	authid = be64(self + 0x70);
	app_version = be64(self + 0x80);
	app_type = be32(self + 0x7c);
	sdk_type = be16(self + 0x08);

	elf = self + elf_offset;
	arch64 = elf_read_hdr(elf, &ehdr);
}


static const char *get_auth_type(void)
{
	return "Unknown";
}

static void show_self_header(void)
{
	printf("SELF header\n");
	printf("  elf #1 offset:  %08x_%08x\n", (u32)(elf_offset>>32), (u32)elf_offset);
	printf("  elf #2 offset:  %08x_%08x\n", (u32)(elf2_offset>>32), (u32)elf2_offset);
	printf("  meta offset:    %08x_%08x\n", 0, meta_offset);
	printf("  phdr offset:    %08x_%08x\n", (u32)(phdr_offset>>32), (u32)phdr_offset);
	printf("  shdr offset:    %08x_%08x\n", (u32)(shdr_offset>>32), (u32)shdr_offset);
	printf("  file size:      %08x_%08x\n", (u32)(filesize>>32), (u32)filesize);
	printf("  auth id:        %08x_%08x (%s)\n", (u32)(authid>>32), (u32)authid, get_auth_type());
	printf("  app version:    %d.%d.%d\n", (u16)(app_version >> 48), (u16)(app_version >> 32), (u32)app_version);
	printf("  SDK type:       %s\n", id2name(sdk_type, t_sdk_type, "unknown"));
	printf("  app type:       %s\n", id2name(app_type, t_app_type, "unknown"));

	printf("\n");
}

static void show_elf_header(void)
{
	printf("ELF header\n");

	printf("  type:                                 %s\n", id2name(ehdr.e_type, t_elf_type, "unknown"));
	printf("  machine:                              %s\n", id2name(ehdr.e_machine, t_elf_machine, "unknown"));
	printf("  version:                              %d\n", ehdr.e_version);

	if (arch64) {
		printf("  phdr offset:                          %08x_%08x\n",
				(u32)(ehdr.e_phoff>>32), (u32)ehdr.e_phoff);  
		printf("  shdr offset:                          %08x_%08x\n",
				(u32)(ehdr.e_phoff>>32), (u32)ehdr.e_shoff);  
		printf("  entry:                                %08x_%08x\n",
				(u32)(ehdr.e_entry>>32), (u32)ehdr.e_entry);  
	} else {
		printf("  phdr offset:                          %08x\n",
				(u32)ehdr.e_phoff);  
		printf("  shdr offset:                          %08x\n",
				(u32)ehdr.e_shoff);  
		printf("  entry:                                %08x\n",
				(u32)ehdr.e_entry);  
	}

	printf("  flags:                                %08x\n", ehdr.e_flags);
	printf("  header size:                          %08x\n", ehdr.e_ehsize);
	printf("  program header size:                  %08x\n",
			ehdr.e_phentsize);
	printf("  program headers:                      %d\n", ehdr.e_phnum);
	printf("  section header size:                  %08x\n",
			ehdr.e_shentsize);
	printf("  section headers:                      %d\n", ehdr.e_shnum);
	printf("  section header string table index:    %d\n", ehdr.e_shtrndx);

	printf("\n");
}

static void get_flags(u32 flags, char *ptr)
{
	memset(ptr, '-', 3);
	ptr[3] = 0;

	if (flags & 4)
		ptr[0] = 'r';
	if (flags & 2)
		ptr[1] = 'w';
	if (flags & 1)
		ptr[2] = 'x';
}

static void show_phdr(unsigned int idx)
{
	struct elf_phdr p;
	char ppc[4], spe[4], rsx[4];

	elf_read_phdr(arch64, elf + phdr_offset + (ehdr.e_phentsize * idx), &p);
	if (arch64) {
	} else {
		get_flags(p.p_flags, ppc);
		get_flags(p.p_flags >> 20, spe);
		get_flags(p.p_flags >> 24, rsx);
		printf("    %5s %08x %08x %08x "
		       "%08x %08x  %s  %s  %s  %08x\n",
		       id2name(p.p_type, t_phdr_type, "?????"),
		       (u32)p.p_off, (u32)p.p_vaddr,
		       (u32)p.p_paddr, (u32)p.p_memsz, (u32)p.p_filesz,
		       ppc, spe, rsx, (u32)p.p_align);
	}
}

static void get_shdr_flags(u32 flags, char *ptr)
{
	memset(ptr, ' ', 3);
	ptr[3] = 0;

	if (flags & 4)
		ptr[0] = 'w';
	if (flags & 2)
		ptr[1] = 'a';
	if (flags & 1)
		ptr[2] = 'e';
}

static void show_shdr(unsigned int idx)
{
	struct elf_shdr s;
	char flags[4];

	elf_read_shdr(arch64, elf + shdr_offset + (ehdr.e_shentsize * idx), &s);
	get_shdr_flags(s.sh_flags, flags);

	if (arch64) {
	} else {
		printf("  [%02d] %-15s %-9s %08x"
			" %08x %08x   %02d %-3s %02d  %02d %02d\n",
			idx, "<no-name>",
			id2name(s.sh_type, t_shdr_type, "????"),
			(u32)s.sh_addr, (u32)s.sh_offset, s.sh_size, s.sh_entsize,
		       	flags, s.sh_link, s.sh_info, s.sh_addralign);

	}
}

static void show_phdrs(void)
{
	unsigned int i;

	printf("Program headers\n");

	if (ehdr.e_phnum == 0) {
		printf("No program headers in this file.\n");
	} else {
		if (arch64)
			printf("\n");
		else
		printf("    type  offset   vaddr    paddr    "
		       "memsize  filesize  PPU  SPE  RSX  align\n");
		for (i = 0; i < ehdr.e_phnum; i++)
			show_phdr(i);
	}

	printf("\n");
}

static void show_shdrs(void)
{
	unsigned int i;

	printf("Section headers\n");

	if (ehdr.e_shnum == 0) {
		printf("No section headers in this file.\n");
	} else {
		if (arch64)
			printf("\n");
		else
			printf("  [Nr] Name            Type      Addr"
				"     Off      Size       ES Flg Lk Inf Al\n");
		for (i = 0; i < ehdr.e_shnum; i++)
			show_shdr(i);
	}

	printf("\n");
}

int main(int argc, char *argv[])
{
	self = mmap_file(argv[1]);

	parse_self();

	show_self_header();
	show_elf_header();
	show_phdrs();
	show_shdrs();

	return 0;
}
