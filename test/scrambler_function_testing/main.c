
typedef struct {
    int x;
    int y
} screen_t;

screen_t screen = {0};

int get_physical_int_num(int pixelNo){
    
    int pixelX, pixelY;
    
    int boxPixel = pixelNo%20;
    int boxNo = pixelNo/20;

    int boxX = boxPixel/5;
    int boxY;
    if ((boxPixel/5)%2 == 0){
        boxY = boxPixel%5;
    }
    else{
        boxY = 5 - boxPixel%5;
    }

    if ((boxNo == 2)||(boxNo == 3)){
        boxX = 4-boxX;
        boxY = 5-boxY;
    }

    if (boxNo == 0){
        pixelX = boxX;
        pixelY = boxY + 5;
    }

    if (boxNo == 1){
        pixelX = boxX+4;
        pixelY = boxY + 5;
    }

    if (boxNo == 2){
        pixelX = boxX+4;
        pixelY = boxY;
    }

    if (boxNo == 3){
        pixelX = boxX;
        pixelY = boxY;
    }

    //printf("X:%d, Y:%d\n",pixelX,pixelY);
    return pixelY*8+pixelX;
    }

int main(){
    screen.x=10;
    screen.y=8;
    for (int i=0;i<screen.x*screen.y;i++){
        printf("%d|%d\n",i,get_physical_int_num(i));
    }
    return 0;
}