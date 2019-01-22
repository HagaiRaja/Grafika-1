#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <math.h>
#include <sys/mman.h>

#define FILENAME "image.txt"
#define SCREEN_WIDTH 1366
#define SCREEN_HEIGHT 762

typedef struct{
    int r,g,b,a;
} color;

struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
char *fbp = 0 ; //pointer framebuffer
int fbfd = 0; //pointer framebuffer driver

unsigned char buffer_r[SCREEN_WIDTH][SCREEN_HEIGHT];
unsigned char buffer_g[SCREEN_WIDTH][SCREEN_HEIGHT];
unsigned char buffer_b[SCREEN_WIDTH][SCREEN_HEIGHT];

void draw_dot(int x, int y, color* c);

int main()
{
//======================================================================================
// PREPARING SCREEN AND FRAME BUFFER

    // opening frame buffer file
    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        perror("Error: driver framebuffer tidak dapat dibuka");
        exit(1);
    }

    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        perror("Error reading fixed information");
        exit(2);
    }

    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading variable information");
        exit(3);
    }

    // Mapping framebuffer to memory
    int screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if((int)fbp == -1){
        perror("Error: failed to map framebuffer device to memory");
        exit(4);
    }
    // Turn off the cursor
    system("setterm -cursor off");

//======================================================================================
// LOAD IMAGE FROM FILENAME

    FILE* file = fopen (FILENAME, "r");
    int temp_r = 0, temp_g = 0, temp_b = 0;
    for (int i = 0; i < SCREEN_HEIGHT; ++i)
    {
        for (int j = 0; j < SCREEN_WIDTH; ++j)
        {
            fscanf (file, "%d %d %d", &temp_r, &temp_g, &temp_b);
            buffer_r[j][i] = temp_r;
            buffer_g[j][i] = temp_g;
            buffer_b[j][i] = temp_b;
        }
    }
    
    fclose (file); 

//=======================================================================================
// DISPLAYING THE IMAGE TO SCREEN
    int screen_speed = 2;
    int start = 0;

    while(1){
    // Translating the image
        int p, q;
        for(p = 0; p < SCREEN_WIDTH; p++) {
            for(q= 0; q < SCREEN_HEIGHT; q++){
                int yPos = (q + start) % SCREEN_HEIGHT;

                if (yPos < 0) {
                    yPos += SCREEN_HEIGHT;
                }

                color c =
                {
                    buffer_r[p][q],
                    buffer_g[p][q],
                    buffer_b[p][q],
                    255
                };
                draw_dot(p, yPos, &c);
            }
        }

	    start -= screen_speed;
        if (start < -SCREEN_HEIGHT) {
            start = 0;
        }
	    usleep(10000);
    }

    // Turn the cursor back on
    system("setterm -cursor on");
    return 0;

}

//######################################################################################
void draw_dot(int x, int y, color* c)
// Drawing a dot at (x,y) point in the screen with c as the color
{
	if((x<1) || (x>SCREEN_WIDTH) || (y<1) || (y>SCREEN_HEIGHT)){
		return ;
	}


    long int position = (x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) +
       (y + vinfo.yoffset) * finfo.line_length;
    if(vinfo.bits_per_pixel == 32){
        *(fbp + position) = c->b;
        *(fbp + position + 1) = c->g;
        *(fbp + position + 2) = c->r;
        *(fbp + position + 3) = c->a;
    }
    else
    {//assume 16 bit color
        int b = c->b;
        int g = c->g;
        int r = c->r;
        unsigned short int t = r<<11 | g << 5 | b;
        *((unsigned short int*)(fbp + position)) = t;
    }
}