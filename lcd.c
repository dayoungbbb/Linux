#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h> //파일
#include <linux/io.h> //io를 건드리니깐 가상 mapping할 때 필요
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
MODULE_LICENSE("GPL");

#define BCM_IO_BASE	0x3F000000 //라즈베리파이의 물리 주소
#define GPIO_BASE	(BCM_IO_BASE + 0x200000) //mapping하기 위해?
#define GPIO_SIZE	(256)

/* GPIO 설정 매크로 */
//디렉션
#define GPIO_IN(g)	(*(gpio+(g)/10)) &= ~(7<<((((g)%10)*3)))
#define GPIO_OUT(g) (*(gpio+((g)/10)) |= (1<<(((g)%10)*3)))
//설정클리어겟
#define GPIO_SET(g) (*(gpio+7) = 1<<g)
#define GPIO_CLR(g) (*(gpio+10) = 1<<g)
#define GPIO_GET(g) (*(gpio+13) & (1<<g))

#define GPIO_MAJOR 200
#define GPIO_MINOR 0
#define GPIO_DEVICE "gpiolcd"

volatile unsigned int *gpio; //gerneral purpose input output

//1. Character LCD 핀 정의
/*
	LCD 1~16번		pi
	pin_1(VSS)	:	 GND
	pin_2(VDD)	:	 5v
	pin_3(V0)	: 	가변저항
	pin_4(RS)	:	gpio_20 //여기서부터 3개는 제어
	pin_5(RW)	: 	gpio_16
	pin_6(Enable) :	gpio_21
	pin_7(db0)	:	gpio_17
	pin_8(db1)	: 	gpio_18
	pin_9(db2) 	: 	gpio_27
	pin_10(db3)	: 	gpio_22
	pin_11(db4)	: 	gpio_23
	pin_12(db5)	: 	gpio_24
	pin_13(db6)	:	gpio_25
	pin_14(db7)	: 	gpio_4
	pin_15(A)	: 	5v
	pin_16(K)	:	GND
   */

//2. 데이터와 제어 신호 분리
int data_pin[8] = {17, 18, 27, 22, 23, 24, 25, 4};
int command_pin[3] = {20, 16, 21}; //rs rw enable

//3. 명령어 핀 정의
#define RS	0 //command_pin의 index number 설정
#define RW 	1
#define EN 2

//4. 데이터 처리 함수 선언
void LCD_Data(char ch); 

//5. GPIO 초기화 함수 선언
void GPIO_Init(void);

//6. LCD_Clear() 함수 선언
void LCD_Clear(void);

//7. char 데이터 처리 함수 선언
void LCD_Char(char c);

//8. string 데이터 처리 함수 선언
void LCD_Str(char *str);

//9. 데이터 위치 설정 함수 선언
void LCD_Pos(unsigned char row, unsigned char col);

//10. LCD 초기화 함수 선언
void LCD_Init(void);

//11. LCD 명령어 처리 함수 선언
void LCD_Command(char ch);

void LCD_Command(char ch) //명령어가 ch로 옴
{
	int i;
	char data, bit;
	data = ch;

	//lcd가 command를 인식하는 거 rs가 low rw가 low enable high 데이터 올리고 enable을 low로 떨어뜨리면 쓸 수 있다
	// rs가 low **명령어** high 데이터
	// rw가 high면 명령어 low write
	// enable은 high에서 low로 떨어뜨리면 전송

	GPIO_CLR(command_pin[RS]);
	GPIO_CLR(command_pin[RW]);
	GPIO_SET(command_pin[EN]);
	mdelay(50);
	for(i = 0; i<8; i++) //데이터핀 설정
	{
		bit = (data>>i) & 0x01;
		if(bit)
			GPIO_SET(data_pin[i]);
		else
			GPIO_CLR(data_pin[i]);
	}
	mdelay(50);
	GPIO_CLR(command_pin[EN]); //전송
	
}
void LCD_Data(char ch) //데이터가 ch로 옴
{
	int i;
	char data, bit;
	data = ch;

	//lcd가 data를 인식하는 거 rs가 low rw가 low enable high 데이터 올리고 enable을 low로 떨어뜨리면 쓸 수 있다
	// rs가 low 명령어 high **데이터**
	// rw가 high면 명령어 low write
	// enable은 high에서 low로 떨어뜨리면 전송

	GPIO_SET(command_pin[RS]);
	GPIO_CLR(command_pin[RW]);
	GPIO_SET(command_pin[EN]);
	mdelay(50);
	for(i = 0; i<8; i++) //데이터핀 설정
	{
		bit = (data>>i) & 0x01;
		if(bit)
			GPIO_SET(data_pin[i]);
		else
			GPIO_CLR(data_pin[i]);
	}
	mdelay(50);
	GPIO_CLR(command_pin[EN]); //전송
	
}
void LCD_Clear(void)
{
	LCD_Command(0x01);
	mdelay(2);
}
void LCD_Init(void)
{
	LCD_Command(0x38); // 8비트 2행을 다 쓰겠다
	mdelay(2); //라파는 빠르고 lcd는 느리기 때문에
	LCD_Command(0x38);
	mdelay(2);
	
	LCD_Command(0x0e); //디스플레이, 커서, 커서 깜빡임 on
	//마지막이 0
	mdelay(2);

	LCD_Command(0x04); //데이터 읽쓰기 후 메모리의 증가 오른쪽으로
	//마지막이 0
	mdelay(2);
	LCD_Clear();
}

void LCD_Pos(unsigned char row, unsigned char col)
{
	LCD_Command(0x80 | (col + row*0x40));
}

void LCD_Char(char c)
{
	LCD_Data(c);
	mdelay(2);
}

void LCD_Str(char* str)
{
	while(*str !=0){
		LCD_Char(*str++);
	}
}

//--------------------------------------------------------

void GPIO_Init(void)
{
	int i;
	for(i = 0; i<8; i++)
		GPIO_OUT(data_pin[i]);

	for(i = 0; i<3; i++)
		GPIO_OUT(command_pin[i]);
}

static int gpio_open(struct inode * inode, struct file *filp)
{
	printk("gpio_open()\n");
	return 0;
}
static int gpio_close(struct inode *inode, struct file *filp)
{
	printk("gpio_close()\n");
	return 0;
}

typedef struct info
{
	char name[16];
	int row, col;
}info;
#define WR           _IOW('c', 1, char[16])//command, 4바이트,
#define BLK_ON       _IO('c', 2)
#define LCD_OFF      _IO('c', 3)
#define LCD_CLR      _IO('c', 4)
#define CUR_OFF      _IO('c', 5)
#define SHIFT_R      _IOW('c', 6, char[16])
#define SHIFT_R_U    _IOW('c', 7, char[16])
#define SHIFT_L_U    _IOW('c', 8, char[16])
#define LOCATION 	 _IOR('c', 9, info)
long my_ioctl(struct file *filp, unsigned int cmd, unsigned long opt)
{
	int size, ret, i, j,k, end;
	char str[16]; //찍고 싶은 string
	info val;
	GPIO_Init();
	LCD_Init();
	
	printk("my_ioctl()\n");
	switch(cmd)
	{
		case WR: //첫째 줄에 글자 쓰기
			size = _IOC_SIZE(cmd);
			ret = copy_from_user(&str, (void*)opt, size);

			printk("str : %s \n", str);
			LCD_Pos(0, 0);
			LCD_Str(str);
			mdelay(2);
			break;
		
		case BLK_ON : //blink on
			printk("blink on \n");
			LCD_Command(0x0d);
			mdelay(2);
			break;
		case LCD_OFF : //LCD OFF
			printk("LCD_OFF\n");
			LCD_Command(0x0b);
			mdelay(2);
			break;
 		case LCD_CLR: //LCD CLEAR
			printk("LCD_CLEAR\n");
			LCD_Clear();
			mdelay(2);
			break;	
		case CUR_OFF:
			printk("CUR_OFF\n");
			LCD_Command(0x0c);
			mdelay(2);
			break;
		case SHIFT_R://6 오른쪽으로 무한 반복
			LCD_Command(0x0c);
			printk("SHIFT_RIGHT\n");
			size = _IOC_SIZE(cmd);
			ret = copy_from_user(&str, (void*)opt, size);
			printk("str : %s \n", str);
	//		while(1){
				for(j = 0; j<2; j++){
					for(i = 0; i<16; i++){
						LCD_Clear();
						LCD_Pos(j, i);
						LCD_Str(str);
						mdelay(1000);
					}
				}
	//		}
			break;
		case SHIFT_R_U:
			LCD_Command(0x0c);
			printk("SHIFT_RIGHT_UNDER\n");
			size = _IOC_SIZE(cmd);
			ret = copy_from_user(&str, (void*)opt, size);
			end = strlen(str); //14
			printk("str : %s \n", str);
			printk("strlen : %d \n", end);
			
//			while(1){
				for(j = 0; j<2; j++){
					for(i = 0; i<16; i++){
						LCD_Clear();
						LCD_Pos(j, i);
						for(k = 0; k<end; k++){
							if(i + k <16){
								LCD_Char(str[k]);
								mdelay(2);
							}	
							else if((i + k) == 16) {
								if(j == 0)
									LCD_Pos(j+1, 0);
								else
									LCD_Pos(j-1, 0);
								LCD_Char(str[k]);
							}
							else {
								LCD_Char(str[k]);
								mdelay(2);
							}
						}
						
						mdelay(1000);
					}
				}
//			}
			break;
		case SHIFT_L_U:
			mdelay(2);
			LCD_Command(0x0c);
			printk("SHIFT_LEFT_UNDER\n");
			size = _IOC_SIZE(cmd);
			ret = copy_from_user(&str, (void*)opt, size);
			end = strlen(str); //14
			
//			while(1){
				for(j = 0; j<2; j++){
					for(i = 15; i>-1; i--){
						LCD_Clear();
						LCD_Pos(j, i);
						LCD_Str(str);
						mdelay(1000);
					}
				}
//			}
			break;
		case LOCATION:
			size = _IOC_SIZE(cmd);
			ret = copy_from_user(&val, (void*)opt, size);
			
			LCD_Clear();
			LCD_Pos(val.row, val.col);
			LCD_Str(val.name);
			mdelay(1000);
			break;
			
	}
	/*
	LCD_Pos(0, 0);
	LCD_Str(str); //출력
	LCD_Pos(1, 0); // 다음 열
	LCD_Str(str1);
	*/

	return 0;

}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = gpio_open,
	.release = gpio_close,
	.unlocked_ioctl = my_ioctl,
};

static struct miscdevice misc =
{
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mydev",
	.fops = &fops,
};

int my_init(void)
{
	static void *map;
	misc_register(&misc);
	map = ioremap(GPIO_BASE, GPIO_SIZE); // 가상주소를 MAPPING시켜서 시작주소를 달라는 거
	if(!map){
		printk("error : mapping GPIO memory\n");
		iounmap(map);
		return -EBUSY;
	}
	gpio = (volatile unsigned int *)map;
//	gpio_run();
	printk("my_init_lcd\n");
	return 0;
}

void my_exit(void)
{
//	unregister_chrdev(200, "mydev");
	misc_deregister(&misc);
	if(gpio){
		iounmap(gpio);
	}
	printk("my_exit_lcd\n");
}

module_init(my_init);
module_exit(my_exit);











