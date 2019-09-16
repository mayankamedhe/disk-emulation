#include "emufs.h"

/**************************** File system helper functions ****************************/

//	Function to encrypt a block of data 
int encrypt(char* input, char* encrypted)
{
	for(int i=0; i<BLOCKSIZE; i++)
	{
		encrypted[i] = ~input[i];
	}
}

//	Function to decrypt a block of data 
int decrypt(char* input, char* decrypted)
{
	for(int i=0; i<BLOCKSIZE; i++)
	{
		decrypted[i] = ~input[i];
	}
}

//	The following helper functions are used to read and write 
//	superblock and metadata block. 
//	Change the function definitions with required arguments
//////////////////////////
struct superblock_t* readSuperblock(int dev_fd)
{
	char buf[BLOCKSIZE];
	if(readblock(dev_fd, 0, buf) == -1) 
	{
		printf("ERROR: Couldn't read superblock\n");
	}
	struct superblock_t* sb = malloc(sizeof(struct superblock_t));
	memcpy(sb, buf, sizeof(struct superblock_t));
	return sb;
	/*
		* Read 0th block from the device into a blocksize buffer
		* Create superblock_t variable and fill it using reader buffer
		* Return the superblock_t variable
	*/
}

int writeSuperblock(int dev_fd, struct superblock_t* superblock)
{
	char buf[BLOCKSIZE];
	readblock(dev_fd, 0, buf);
	memcpy(buf, superblock, sizeof(struct superblock_t));
	if(writeblock(dev_fd, 0, buf) == -1)
	{
		printf("ERROR: Couldn't write on superblock\n");
		return -1;
	} 
	// printf("in writesupbuf = %d \n", superblock->fs_number);
	return 0;

	/*
		* Read the 0th block from device into a buffer
		* Write the superblock into the buffer
		* Write back the buffer into block 0
	*/
}

struct metadata_t* readMetadata(int dev_fd, int fs_number)
{
	char buf[BLOCKSIZE];
	// char encrypt_buf[BLOCKSIZE];
	if(readblock(dev_fd, 1, buf) == -1)
	{
		printf("ERROR: Couldn't read metadata\n");
	}
	else
	{
		struct metadata_t* sb = malloc(sizeof(struct metadata_t));
	
	if(fs_number)
	{
		decrypt(buf, buf);
	}
		memcpy(sb, buf, sizeof(struct metadata_t));
	return sb;
	}
	
	// Same as readSuperBlock(), but it is stored on block 1
	// Need to decrypt if emufs-encrypted is used  
}

int writeMetadata(int dev_fd, struct metadata_t* metadata, int fs_number)
{
	char buf[BLOCKSIZE];
	struct superblock_t * sb = readSuperblock(dev_fd);
	memcpy(buf, metadata, sizeof(struct metadata_t));
	if(sb->fs_number == 1)
	{
		encrypt(buf, buf);
	}

	if(writeblock(dev_fd, 1, buf) == -1)
	{
		printf("ERROR: Couldn't write on metadata\n");
		return -1;
	}
	return 0;
	// Same as writeSuperblock(), but it is stored on block 1
	// Need to decrypt/encrypt if emufs-encrypted is used  
}
////////////////////////
// struct superblock_t* readSuperblock(int dev_fd)
// {
// 	char buf[BLOCKSIZE];
// 	readblock(dev_fd, 0, buf);
// 	struct superblock_t* sb = malloc(sizeof(struct superblock_t));
// 	memcpy(buf, sb, sizeof(struct superblock_t));
// 	return sb;
// 	/*
// 		* Read 0th block from the device into a blocksize buffer
// 		* Create superblock_t variable and fill it using reader buffer
// 		* Return the superblock_t variable
// 	*/
// }

// int writeSuperblock(int dev_fd, struct superblock_t* superblock)
// {
// 	char buf[BLOCKSIZE];
// 	readblock(dev_fd, 0, buf);
// 	memcpy(superblock, buf, sizeof(struct superblock_t));
// 	writeblock(dev_fd, 0, buf);

// 	/*
// 		* Read the 0th block from device into a buffer
// 		* Write the superblock into the buffer
// 		* Write back the buffer into block 0
// 	*/
// }

// struct metadata_t* readMetadata(int dev_fd, int fs_number)
// {
// 	char buf[BLOCKSIZE];
// 	char encrypt_buf[BLOCKSIZE];
// 	readblock(dev_fd, 1, buf);
// 	struct metadata_t* sb = malloc(sizeof(struct metadata_t));
	
// 	if(fs_number)
// 	{
// 		decrypt(buf, encrypt_buf);
// 		memcpy(sb, encrypt_buf, sizeof(struct metadata_t));
// 	}
// 	else
// 	{
// 		memcpy(sb, buf, sizeof(struct metadata_t));
// 	}
// 	return sb;
// 	// Same as readSuperBlock(), but it is stored on block 1
// 	// Need to decrypt if emufs-encrypted is used  
// }

// int writeMetadata(int dev_fd, struct metadata_t* metadata, int fs_number)
// {
// 	char buf[BLOCKSIZE];
// 	char encrypt_buf[BLOCKSIZE];
// 	readblock(dev_fd, 1, buf);
// 	if(fs_number)
// 	{
// 		encrypt(buf, encrypt_buf);
// 		memcpy(encrypt_buf, metadata, sizeof(struct metadata_t));

// 	}

// 	else
// 	{
// 		memcpy(buf, metadata, sizeof(struct metadata_t));		
// 	}
// 	writeblock(dev_fd, 1, buf);
// 	return 1;
// 	// Same as writeSuperblock(), but it is stored on block 1
// 	// Need to decrypt/encrypt if emufs-encrypted is used  
// }

/**************************** File system API ****************************/

int create_file_system(struct mount_t *mount_point, int fs_number)
{
	struct superblock_t* rsb = readSuperblock(mount_point->device_fd);

	rsb->fs_number = fs_number;
	rsb->bitmap[MAX_BLOCKS] = 0;
	
	struct metadata_t* md = malloc(sizeof(struct metadata_t));


	if(writeMetadata(mount_point->device_fd, md, fs_number) == -1)
	{
		return -1;
	}

    if(writeSuperblock(mount_point->device_fd, rsb) == -1)
    {
    	return -1;
    }

	/*
	   	* Read the superblock.
	    * Set file system number on superblock
		* Clear the bitmaps.  values on the bitmap will be either '0', or '1', or'x'. 
		* Create metadata block in disk
		* Write superblock and metadata block back to disk.

		* Return value: -1,		error
						 1, 	success
	*/

	return 1;
}


struct file_t* eopen(struct mount_t* mount_point, char* filename)
{
	struct metadata_t* rm = readMetadata(mount_point->device_fd, mount_point->fs_number);
	int present = -1;
	int i;
	int inode = -1;
	for (i = 0; i < 10; ++i)
	{
		if(rm->inodes[i].status == USED && strcmp(filename, rm->inodes[i].name) == 0)
		{
			present = 1;
			break;
		}
	}

	if(present)
	{
		inode = i;
	}
	else
	{
		for (int j = 0; j < 10; ++j)
		{
			if(!rm->inodes[j].status)
			{
				rm->inodes[j].status = 1;
				inode = j;
				strcpy(rm->inodes[j].name, filename);
				writeMetadata(mount_point->device_fd, rm, mount_point->fs_number);
				break;	
			}
		}

		if(inode == -1)
		{

			printf("No free inode\n");	
			return NULL;
		}
	}

	struct file_t* temp_file = malloc(sizeof(struct file_t));
	temp_file->offset = 0; 
	temp_file->inode_number = inode;
	temp_file->mount_point = mount_point;
	return temp_file;
	/* 
		* If file exist, get the inode number. inode number is the index of inode in the metadata.
		* If file does not exist, 
			* find free inode.
			* allocate the free inode as USED
			* if free id not found, print the error and return -1
		* Create the file hander (struct file_t)
		* Initialize offset in the file hander
		* Return file handler.

		* Return NULL on error.
	*/

	// return NULL;
}

int ewrite(struct file_t* file, char* data, int size)
{
	int start = file->offset/BLOCKSIZE;
	int end = (file->offset + size -1)/BLOCKSIZE;
	char  buf[512];
	int num = file->inode_number;
	struct metadata_t* temp = readMetadata(file->mount_point->device_fd, file->mount_point->fs_number);

	struct superblock_t* sb = readSuperblock(file->mount_point->device_fd);

	if(start >= 4 || end >= 4 || start <= 0 || end <= 0)
	{
		return -1;
	}
	else
	{
		for (int i = start; i < end; ++i)
		{
			if(temp->inodes[num].status != 0)
			{
				writeblock(file->mount_point->device_fd, temp->inodes[num].blocks[i], data + (i - start)* 512);
			}
			else
			{
				for (int j = 0; j < MAX_BLOCKS; ++j)
				{
					if(!sb->bitmap[j])
					{
						temp->inodes[num].blocks[i] = j;
						temp->inodes[num].file_size += 512;
						writeblock(file->mount_point->device_fd, j ,data + (i - start)* 512);

					}
				}
				writeSuperblock(file->mount_point->device_fd, sb);
				return size;
			}
		}
	}
	

	// return 0;
}

int eread(struct file_t* file, char* data, int size)
{
	int start = file->offset/BLOCKSIZE;
	int end = (file->offset + size -1)/BLOCKSIZE;
	char  buf[512];
	int num = file->inode_number;
	struct metadata_t* temp = readMetadata(file->mount_point->device_fd, file->mount_point->fs_number);


	if(start >= 4 || end >= 4 || start < 0 || end < 0)
	{
		return -1;
	}
	else
	{
		for (int i = start; i <= end; ++i)
		{

			readblock(file->mount_point->device_fd, temp->inodes[num].blocks[i], buf);
			memcpy(data + (i-start)* 512, buf, sizeof(buf));	
		}
		file->offset += size; 
		return size;

	}
	// NO partial READS.

	// Return value: 	-1,  error
	//					Number of bytes read

}

void eclose(struct file_t* file)
{

	free(file);
	// free the memory allocated for the file handler 'file'
}

int eseek(struct file_t *file, int offset)
{
	// Change the offset in file hanlder 'file'
	file->offset = offset;
	return 1;
}

void fsdump(struct mount_t* mount_point)
{

	printf("\n[%s] fsdump \n", mount_point->device_name);
	printf("%-10s %6s \t[%s] \t%s\n", "  NAME", "SIZE", "BLOCKS", "LAST MODIFIED");
	
	// struct metadata_t* temp = malloc(sizeof(struct metadata_t));

	struct metadata_t *temp = readMetadata(mount_point->device_fd, mount_point->fs_number);

	for (int i = 0; i < 10; ++i)
	{
		if(temp->inodes[i].status)
		{
		printf("%-10s %6d \t[%d %d %d %d] \t%s", 
			temp->inodes[i].name, 
			temp->inodes[i].file_size,
			temp->inodes[i].blocks[0],
			temp->inodes[i].blocks[1],
			temp->inodes[i].blocks[2],
			temp->inodes[i].blocks[3],
			asctime(localtime(&(temp->inodes[i].modtime))));
		}

		
	}
	

}
