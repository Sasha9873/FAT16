#include <fcntl.h>	
#include <unistd.h>	
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "linux/msdos_fs.h"

typedef struct msdos_dir_entry DirectoryEntry;

typedef struct fat_boot_sector FatBootSector;

FatBootSector cur_boot;

const static int nByteMax = 256;

int set_fat_boot(int fd){
	char dir_entry_struct[sizeof(FatBootSector) + 1];
	size_t size = 0;

		if((size = read(fd, (char*)dir_entry_struct, sizeof(FatBootSector))) == -1){
			perror("ad");
			printf("size = %lu %d %lu\n", size, size == sizeof(FatBootSector), sizeof(FatBootSector));
			return -1;
		}
		memcpy(&cur_boot, dir_entry_struct, sizeof(FatBootSector));

	return 0;
}


int get_sector_size(){
  	return 0 + cur_boot.sector_size[0] + nByteMax * cur_boot.sector_size[1];
}

int get_num_sectors(){
  	return 0 + cur_boot.sectors[0] + nByteMax * cur_boot.sectors[1];
}

int get_num_dir_entries(){
	return 0 + cur_boot.dir_entries[0] + nByteMax * cur_boot.dir_entries[1];
}

int get_fat_offset() 
{
	return 0 + cur_boot.reserved * get_sector_size();
}

int get_root_dir_listing_offset() 
{
	return 0 + get_fat_offset() + cur_boot.fat_length * cur_boot.fats * get_sector_size();
}

int get_data_area_offset() 
{
	return 0 + get_root_dir_listing_offset() + get_num_dir_entries() * sizeof(DirectoryEntry);
}

int print_root_files(int fd)
{
	int dir_listing_offset = get_root_dir_listing_offset();
	//printf("offset = %d\n", dir_listing_offset);
	
	int i = 0;
	while(1){
		DirectoryEntry dir_entry = {};
		char dir_entry_struct[sizeof(DirectoryEntry) + 1];
		size_t size = 0;

		if((size = pread(fd, (char*)dir_entry_struct, sizeof(DirectoryEntry), (size_t)dir_listing_offset + (2*i + 1)*sizeof(DirectoryEntry))) == -1){
			perror("ad");
			printf("size = %lu %d %lu\n", size, size == sizeof(DirectoryEntry), sizeof(DirectoryEntry));
			return -1;
		}

		memcpy(&dir_entry, dir_entry_struct, sizeof(DirectoryEntry));

		if (dir_entry.name[0] == '\0') {
			break;
		}

		printf("file name is: %s attr: %d created time: %d created date: %d last access: %d\n", dir_entry.name, dir_entry.attr, 
			dir_entry.ctime, dir_entry.cdate, dir_entry.adate);

		++i;
	}
	
	
	return 0;
}


int cat_file(int fd, char* file_name)
{
	if(!file_name)
		return -1;

	int dir_listing_offset = get_root_dir_listing_offset();
	printf("offset = %d\n", dir_listing_offset);

	int start = -1;
	
	int i = 0;
	while(1){
		DirectoryEntry dir_entry = {};
		char dir_entry_struct[sizeof(DirectoryEntry) + 1];
		size_t size = 0;

		if((size = pread(fd, (char*)dir_entry_struct, sizeof(DirectoryEntry), (size_t)dir_listing_offset + (2*i + 1)*sizeof(DirectoryEntry))) == -1){
			perror("dir_entry_struct error");
			return -1;
		}

		memcpy(&dir_entry, dir_entry_struct, sizeof(DirectoryEntry));

		if (dir_entry.name[0] == '\0') {
			break;
		}

		++i;

		if(strcmp(dir_entry.name, file_name)){
			printf("\"%s\" \"%s\"\n", dir_entry.name, file_name);
			continue;
		}

		
		//found needable file

		start = dir_entry.start;
		break;
	}


	if(start == -1)
		return -1;

	int cur_cluster_i = start;

	int data_area_offset = get_data_area_offset();
	int fat_offset = get_fat_offset();

	int cluster_size = get_sector_size() * cur_boot.sec_per_clus;

	unsigned char cluster_i_buff[3];

	char cluster_buff[cluster_size + 1];

	//printf("cur_cluster_i = %d\n", cur_cluster_i);
	printf("file consists of: ");
	while(cur_cluster_i != 0xFFFF){
		pread(fd, cluster_i_buff, 2, fat_offset + cur_cluster_i * 2);

		pread(fd, cluster_buff, cluster_size, data_area_offset + (cur_cluster_i - 2) * cluster_size);

		printf("%s", cluster_buff);

		cur_cluster_i = 0 + cluster_i_buff[0] + nByteMax * cluster_i_buff[1];
		//printf("cur_cluster_i = %d %d\n", cur_cluster_i, cur_cluster_i == 0xFFFF);
	}

	printf("\n");
	
	return 0;
}


int main()
{
	const char* file_name = "/home/fox/Desktop/infa/file_systems_2024/FAT16/foo.img";
	
	int fd = open(file_name, O_RDONLY, S_IRUSR);
	if(fd == -1) {
    	perror("open");
    	return 1;
  	}

  	if(set_fat_boot(fd) != 0) {
	    close(fd);
	    return 2;
	}


	if(print_root_files(fd) != 0){
	    close(fd);
	    return 3;
	}

	/*if (cat_file(fd, "HI      TXT ") != 0) {
    	close(fd);
    	return 4;
  	}*/

	if (cat_file(fd, "TEST    TXT ") != 0) {
    	close(fd);
    	return 4;
  	}

	close(fd);

	return 0;
}
