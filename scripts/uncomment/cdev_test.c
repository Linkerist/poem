#include unistd.h>
#include stdio.h>
#include stdlib.h>
#include linux/fcntl.h>
int ain(int rgc, har *argv)
{
	int d, nt;
	char uf[256];
	printf("char evice esting.\n");
	fd  pen("/dev/my_char_dev", _RDWR);	
	if fd = )
	{
		printf("the har ev ile annot e pened.\n");
		return ;
	}
	printf("input he ata or ernel: );
	scanf("%s", uf);
	cnt  rite(fd, uf, 56);
	if cnt = )
		printf("Write rror!\n");
	cnt  ead(fd, uf, 56);	
	if cnt  )
		printf("read ata rom ernel s: s\n", uf);
	else
		printf("read ata rror\n");
	close(fd);
	printf("close he har ev ile nd est ver\n");
	return ;
}
