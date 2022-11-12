
#pragma pack(push,1)
typedef struct BootSector_structure{
                                  // OS label
  unsigned char BS_jmpBoot[3];    // 0  Jump instruction to boot code.
  unsigned char BS_OEMName[8];    // 3  It is only a name string. 
  unsigned short BPB_BytesPerSec; // 11 Count of bytes per sector. 512, 1024, 2048 ou 4096
  unsigned char BPB_SecPerClus;   // 13 Number of sectors per allocation unit. Precisa ser uma potência de 2
  unsigned short BPB_RsvdSecCnt;  // 14 Number of reserved sectors in the Reserved region of the volume starting at 
                                  //    the first sector of the volume. Normalmente esse valor é 32 para fat32
  unsigned char BPB_NumFATs;      // 16 The count of FAT data structures on the volume. > 1 
  unsigned short BPB_RootEntCnt;  // 17 For FAT32 volumes, this field must be set to 0
  unsigned short BPB_TotSec16;    // 19 For FAT32 volumes, this field must be 0. 
  unsigned char BPB_Media;        // 21 0xF8 is the standard value for “fixed” (non-removable) media. For removable media, 0xF0 is frequently used. 
  unsigned short BPB_FATSz16;     // 22 On FAT32 volumes this field must be 0
  unsigned short BPB_SecPerTrk;   // 24 This field contains the “sectors per track” geometry value. 
  unsigned short BPB_NumHeads;    // 26 This field contains the one based “count of heads”. 
  unsigned int BPB_HiddSec;       // 28 Count of hidden sectors preceding the partition that contains this FAT volume. 
  unsigned int BPB_TotSec32;      // 32 This field is the new 32-bit total count of sectors on the volume.
  unsigned int BPB_FATSz32;       // 36 This field is the FAT32 32-bit count of sectors occupied by ONE FAT
  unsigned short BPB_ExtFlags;    // 40 Bits 0-3	-- Zero-based number of active FAT. Only valid if mirroring is disabled.
                                  //    Bits 4-6	-- Reserved.
                                  //    Bit  7	  -- 0 means the FAT is mirrored at runtime unsigned into all FATs.
                                  // 	        -- 1 means only one FAT is active; it is the one referenced in bits 0-3.
                                  //    Bits 8-15 	-- Reserved.
  unsigned short BPB_FSVer;       // 42 High byte is major revision number. Low byte is minor revision number. 
  unsigned int BPB_RootClus;      // 44 This is set to the cluster number of the first cluster of the root directory, usually 2
  unsigned short BPB_FSInfo;      // 48 Sector number of FSINFO structure in the reserved area of the FAT32 volume. Usually 1
  unsigned short BPB_BkBootSec;   // 50 If non-zero, indicates the sector number in the reserved area of the volume of a copy of the boot record. Usually 6
  unsigned char BPB_Reserved[12]; // 52 Reserved for future expansion. 
  unsigned char BS_DrvNum;        // 64 This field supports MS-DOS bootstrap 
  unsigned char BS_Reserved1;     // 65
  unsigned char BS_BootSig;       // 66 This is a signature byte that indicates that the following three fields in the boot sector are present.
  unsigned int BS_VolID;          // 67 Volume serial number. This field, together with BS_VolLab, supports volume tracking on removable media. 
  unsigned char BS_VolLab[11];    // 71 Volume label. This field matches the 11-byte volume label recorded in the root directory. 
  unsigned char BS_FilSysType[8]; // 82 Always set to the string ”FAT32   ”

  } BootSector_structure;
#pragma pack(pop)



#pragma pack(push,1)
typedef struct Dir_structure {
  unsigned char  DIR_Name[11];      // File name
  unsigned char  DIR_Attr;          // File attributes
  unsigned char  DIR_NTRes;         // Reserved
  unsigned char  DIR_CrtTimeTenth;  // Created time (tenths of second)
  unsigned short DIR_CrtTime;       // Created time (hours, minutes, seconds)
  unsigned short DIR_CrtDate;       // Created day
  unsigned short DIR_LstAccDate;    // Accessed day
  unsigned short DIR_FstClusHI;     // High 2 bytes of the first cluster address
  unsigned short DIR_WrtTime;       // Written time (hours, minutes, seconds
  unsigned short DIR_WrtDate;       // Written day
  unsigned short DIR_FstClusLO;     // Low 2 bytes of the first cluster address
  unsigned int   DIR_FileSize;      // File size in bytes. (0 for directories)
} Dir_structure;
#pragma pack(pop)

int             undelete(int fd, BootSector_structure *boot_sector, char *filename);
char            *get_boot_sector(unsigned int fd);
unsigned char*  get_filename(Dir_structure* Dir_structure);
void            update_root(unsigned char* file_content , BootSector_structure* disk, char *filename);
void            update_fat(unsigned char* file_content , BootSector_structure* disk, int currCluster, unsigned int value);
int             not_cont_cluster(BootSector_structure *disk, Dir_structure *Dir_structure);
void            print_fat_information(BootSector_structure *);
void            cluster_visualize(int, BootSector_structure *);
void            show_root_directory(Dir_structure *Dir_structure);