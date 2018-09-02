#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
/*
#define LED_ON   1
#define LED_OFF  2
#define LED_TEST 3
*/
typedef struct info
{
	char name[16];
	int row, col;

}info;
#define WR	    	 _IOW('c', 1, char[16])//command, 4바이트, 
#define BLK_ON		 _IO('c', 2)
#define LCD_OFF 	 _IO('c', 3)
#define LCD_CLR 	 _IO('c', 4)
#define CUR_OFF		 _IO('c', 5) 
#define SHIFT_R 	 _IOW('c', 6, char[16])
#define SHIFT_R_U 	 _IOW('c', 7, char[16])
#define SHIFT_L_U 	 _IOW('c', 8, char[16])
#define LOCATION	 _IOR('c', 9, info)

int main(int argc, char **argv)
{
	int fd, ch, i;
	char val[16] = "HELLO EVERYONE";
	char filename[128] = "/dev/";
	info val2;
	strcat(filename, argv[1]);
	fd = open(filename, O_RDWR);
	while(1)
	{
		scanf("%d", &ch);		
		switch(ch)
		{
			case 1: //글자 쓰기
				ioctl(fd, WR, &val);
				printf("val : %s\n", val);
				break;
			case 2: //blink on
				ioctl(fd, BLK_ON);
				printf("BLINK_ON\n");
				break;
			case 3: //LCD OFF
				ioctl(fd, LCD_OFF);
				printf("LCD_OFF\n");
				break;
			case 4: //LCD CLEAR
				ioctl(fd, LCD_CLR);
				printf("LCD_Clear\n");
				break;
			case 5: //CURSOR OFF
				ioctl(fd, CUR_OFF);
				printf("CUR_OFF\n");
				break;
			case 6: //shift right
				ioctl(fd, SHIFT_R, &val);
				printf("SHIFT_RIGHT\n");
				printf("val : %s\n", val);
				break;
			case 7: // shift right under
				ioctl(fd, SHIFT_R_U, &val);
				printf("SHIFT_RIGHT_UNDER\n");
				printf("val : %s\n", val);
				break;
			case 8: // shift left under
				ioctl(fd, SHIFT_L_U, &val);
				printf("SHIFT_LEFT_UNDER\n");
				printf("val : %s\n", val);
				break;
			case 9: // location
				strcpy(val2.name, "YO");
				val2.row = 1; val2.col = 7;
				ioctl(fd, LOCATION, &val2);
				printf("LOCATION\n");
				break;
			case 10:
				break;
			default :
				return 0;
		}
	}
	close(fd);
	return 0;
}
