#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include "fat.h"
#include <sys/stat.h>

int main(int argc, char **argv)
{
  
  int fd = open("/dev/sda1", O_RDWR);
  BootSector_structure *boot_sector = mmap(NULL, sizeof(BootSector_structure), PROT_READ, MAP_PRIVATE, fd, 0);

  undelete(fd, boot_sector, "arquivo.txt");
  //print_fat_information(boot_sector);
  // cluster_visualize(fd, boot_sector);
  close(fd);
  return 0;
}

void print_fat_information(BootSector_structure *boot_sector)
{
  printf("Jump instruction:                   0x%X\n", boot_sector->BS_jmpBoot[0]);
  printf("OEM name:                           %s\n", boot_sector->BS_OEMName);
  printf("Bytes per sector:                   0x%X\n", boot_sector->BPB_BytesPerSec);
  printf("Sector per cluster:                 0x%X\n", boot_sector->BPB_SecPerClus);
  printf("Number of reserved sectors:         0x%X\n", boot_sector->BPB_RsvdSecCnt);
  printf("Number of FATs:                     0x%X\n", boot_sector->BPB_NumFATs);
  printf("Directory entries:                  0x%X\n", boot_sector->BPB_RootEntCnt);
  printf("16-bit sector count:                0x%X\n", boot_sector->BPB_TotSec16);
  printf("Media:                              0x%X\n", boot_sector->BPB_Media);
  printf("Bytes per sector:                   0x%X\n", boot_sector->BPB_BytesPerSec);
  printf("Sector occupied by one 16-bit fat:  0x%X\n", boot_sector->BPB_FATSz16);
  printf("Sector per track:                   0x%X\n", boot_sector->BPB_SecPerTrk);
  printf("Number of heads:                    0x%X\n", boot_sector->BPB_NumHeads);
  printf("Hidden sectors:                     0x%X\n", boot_sector->BPB_HiddSec);
  printf("32-bit sector count:                0x%X\n", boot_sector->BPB_TotSec32);
  printf("Sector occupied by one 32-bit fat:  0x%X\n", boot_sector->BPB_FATSz32);
  printf("Ext flags:                          0x%X\n", boot_sector->BPB_ExtFlags);
  printf("FAT32 volume version:               0x%X\n", boot_sector->BPB_FSVer);
  printf("Cluster number:                     0x%X\n", boot_sector->BPB_RootClus);
  printf("Sector number of FSINFO:            0x%X\n", boot_sector->BPB_FSInfo);
  printf("Sector number of boot record copy:  0x%X\n", boot_sector->BPB_BkBootSec);
  printf("Reserved:                           \n");
  printf("MS-DOS support field:               0x%X\n", boot_sector->BS_DrvNum);
  printf("Reserved 1:                         0x%X\n", boot_sector->BS_Reserved1);
  printf("Volume serial number:               0x%X\n", boot_sector->BS_VolID);
}

void cluster_visualize(int fd, BootSector_structure *boot_sector)
{

  int current_cluster = boot_sector->BPB_RootClus;
  int totalPossibleEntry = (boot_sector->BPB_SecPerClus * boot_sector->BPB_BytesPerSec) / sizeof(Dir_structure);

  char *file_content = get_boot_sector(fd);
  while (1)
  {
    unsigned int root_sector = (boot_sector->BPB_RsvdSecCnt + boot_sector->BPB_NumFATs * boot_sector->BPB_FATSz32) + (current_cluster - 2) * boot_sector->BPB_SecPerClus;
    unsigned int root_cluster = root_sector * boot_sector->BPB_BytesPerSec;

    Dir_structure *dir_structure = (Dir_structure *)(file_content + root_cluster);
    for (int index = 0; index < totalPossibleEntry; index++)
    {
      if (dir_structure->DIR_Attr == 0x00)
        break;

      if (dir_structure->DIR_Name[0] != 0xe5)
        show_root_directory(dir_structure);
      dir_structure++;
    }
    unsigned int *fat = (unsigned int *)(file_content + boot_sector->BPB_RsvdSecCnt * boot_sector->BPB_BytesPerSec + 4 * current_cluster);
    if (*fat >= 0xFFFFFF8)
      break;
    current_cluster = *fat;
  }
}

void show_root_directory(Dir_structure *Dir_structure)
{
  if (Dir_structure->DIR_Attr == 0x10) // directory
  {
    int index = 0;
    while (Dir_structure->DIR_Name[index] != ' ')
    {
      printf("%c", Dir_structure->DIR_Name[index]);
      index++;
    }
    printf("/");
  }
  else
  {
    for (int i = 0; i < 8; i++)
    {
      if (Dir_structure->DIR_Name[i] == ' ')
        break;
      printf("%c", Dir_structure->DIR_Name[i]);
    }
    if (Dir_structure->DIR_Name[8] != ' ')
    {
      printf(".");
      for (int i = 8; i < 12; i++)
      {
        if (Dir_structure->DIR_Name[i] == ' ')
          break;
        printf("%c", Dir_structure->DIR_Name[i]);
      }
    }
  }
  printf("\n");
}

char *get_boot_sector(unsigned int fd)
{
  struct stat fs;
  return mmap(NULL, fs.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
}

int undelete(int fd, BootSector_structure *boot_sector, char *filename)
{
  unsigned int current_cluster = boot_sector->BPB_RootClus;
  unsigned int total_entries = (boot_sector->BPB_SecPerClus * boot_sector->BPB_BytesPerSec) / sizeof(Dir_structure);

  struct stat fs;
  int flag = 0;
  unsigned char *file_content = get_boot_sector(fd);

  Dir_structure *dir_structure;

  while (1)
  {
    unsigned int root_sector = (boot_sector->BPB_RsvdSecCnt + boot_sector->BPB_NumFATs * boot_sector->BPB_FATSz32) + (current_cluster - 2) * boot_sector->BPB_SecPerClus;
    unsigned int root_cluster = root_sector * boot_sector->BPB_BytesPerSec;
    dir_structure = (Dir_structure *)(file_content + root_cluster);

    for (unsigned int index = 0; index < total_entries; index++)
    {
      if (dir_structure->DIR_Attr == 0x00)
        break;
      if ((dir_structure->DIR_Attr == 0x10) == 0 && dir_structure->DIR_Name[0] == 0xe5)
      {
        if (dir_structure->DIR_Name[1] == filename[1])
        {
          char *undelete_file = get_filename(dir_structure);
          if (strcmp(undelete_file + 1, filename + 1) == 0)
          {
            flag = 1;
            break;
          }
        }
      }
      dir_structure++;
    }

    current_cluster = *(file_content + boot_sector->BPB_RsvdSecCnt * boot_sector->BPB_BytesPerSec + 4 * current_cluster);
    if (current_cluster >= 0x0ffffff8 || current_cluster == 0x00)
    {
      break;
    }
  }

  update_root(file_content, boot_sector, filename);
  update_fat(file_content, boot_sector, current_cluster + 1, 0x0ffffff8);
  munmap(file_content, fs.st_size);

  return 1;
}

unsigned char *get_filename(Dir_structure *Dir_structure)
{
  unsigned char *pointer = malloc(12 * sizeof(unsigned char *));
  unsigned int idx = 0;
  for (int i = 0; i < 8; i++)
  {
    if (Dir_structure->DIR_Name[i] == ' ')
      break;
    pointer[idx] = Dir_structure->DIR_Name[i];
    idx++;
  }

  if (Dir_structure->DIR_Name[8] != ' ')
  {
    pointer[idx] = '.';
    idx++;
  }

  for (int i = 8; i < 12; i++)
  {
    if (Dir_structure->DIR_Name[i] == ' ')
      break;
    pointer[idx] = Dir_structure->DIR_Name[i];
    idx++;
  }

  pointer[idx] = '\0';
  return pointer;
}

void update_root(unsigned char *file_content, BootSector_structure *disk, char *filename)
{
  unsigned int root_sector = (disk->BPB_RsvdSecCnt + disk->BPB_NumFATs * disk->BPB_FATSz32);
  unsigned int root_cluster = root_sector * disk->BPB_BytesPerSec;
  file_content[root_cluster + 1] = filename[0];
}

void update_fat(unsigned char *file_content, BootSector_structure *disk, int currCluster, unsigned int value)
{
  for (int i = 0; i < disk->BPB_NumFATs; i++)
  {
    unsigned int *fat = (unsigned int *)(file_content + (disk->BPB_RsvdSecCnt + i * disk->BPB_FATSz32) * disk->BPB_BytesPerSec + 4 * currCluster);
    *fat = value;
  }
}
