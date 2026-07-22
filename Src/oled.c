/*
 * oled.c
 *
 *  Created on: 2022?7?24?
 *      Author: Unicorn_Li
 */
#include "oled.h"

/**********************************************************
 * ?????,????????,?????????????
 ***********************************************************/
uint8_t CMD_Data[]={
0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40,0xA1, 0xC8, 0xDA,

0x12, 0x81, 0xCF, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6,0x8D, 0x14,

0xAF};

// 定义显存缓冲区 (128列 x 8页)
uint8_t OLED_GRAM[128][8];



/**
 * @function: void OLED_Init(void)
 * @description: OLED???
 * @return {*}
 */
void OLED_Init(void)
{
	HAL_Delay(200);

	uint8_t i = 0;
	for(i=0; i<23; i++)
	{
		OLED_WR_CMD(CMD_Data[i]);
	}
	
}

/**
 * @function: void OLED_WR_CMD(uint8_t cmd)
 * @description: ????????
 * @param {uint8_t} cmd ?????????
 * @return {*}
 */
void OLED_WR_CMD(uint8_t cmd)
{
	HAL_I2C_Mem_Write(&hi2c1 ,0x78,0x00,I2C_MEMADD_SIZE_8BIT,&cmd,1,0x0A);
}

/**
 * @function: void OLED_WR_DATA(uint8_t data)
 * @description: ????????
 * @param {uint8_t} data ??
 * @return {*}
 */
void OLED_WR_DATA(uint8_t data)
{
	HAL_I2C_Mem_Write(&hi2c1 ,0x78,0x40,I2C_MEMADD_SIZE_8BIT,&data,1,0x0A);
}

/**
 * @function: void OLED_On(void)
 * @description: ????

 * @return {*}
 */
void OLED_On(void)
{
	uint8_t i,n;
	for(i=0;i<8;i++)
	{
		OLED_WR_CMD(0xb0+i);    //?????(0~7)
		OLED_WR_CMD(0x00);      //??????????
		OLED_WR_CMD(0x10);      //??????????
		for(n=0;n<128;n++)
			OLED_WR_DATA(1);
	}
}


/**
 * @function: OLED_Clear(void)
 * @description: ??,????????!??????!!!
 * @return {*}
 */
void OLED_Clear(void)
{
    uint8_t i, n;
    // 立即清空屏幕硬件
    for(i = 0; i < 8; i++)
    {
        OLED_WR_CMD(0xb0 + i);
        OLED_WR_CMD(0x00);
        OLED_WR_CMD(0x10);
        for(n = 0; n < 128; n++)
            OLED_WR_DATA(0);
    }
}

/**
 * @function: void OLED_Display_On(void)
 * @description: ??OLED??
 * @return {*}
 */
void OLED_Display_On(void)
{
	OLED_WR_CMD(0X8D);  //SET DCDC??
	OLED_WR_CMD(0X14);  //DCDC ON
	OLED_WR_CMD(0XAF);  //DISPLAY ON,????
}


/**
 * @function: void OLED_Display_Off(void)
 * @description: ??OLED??
 * @return {*}
 */
void OLED_Display_Off(void)
{
	OLED_WR_CMD(0X8D);  //SET DCDC??
	OLED_WR_CMD(0X10);  //DCDC OFF
	OLED_WR_CMD(0XAE);  //DISPLAY OFF,????
}

/**
 * @function: void OLED_Set_Pos(uint8_t x, uint8_t y)
 * @description: ????
 * @param {uint8_t} x,y
 * @return {*}
 */
void OLED_Set_Pos(uint8_t x, uint8_t y)
{
	OLED_WR_CMD(0xb0+y);	//?????(0~7)
	OLED_WR_CMD(((x&0xf0)>>4)|0x10); //??????????
	OLED_WR_CMD(x&0x0f);	//??????????
}


/**
 * @function: unsigned int oled_pow(uint8_t m,uint8_t n)
 * @description: m^n??
 * @param {uint8_t} m,n
 * @return {unsigned int} result
 */
unsigned int oled_pow(uint8_t m,uint8_t n)
{
	unsigned int result=1;
	while(n--)result*=m;
	return result;
}

/**
 * @function: void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t Char_Size,uint8_t Color_Turn)
 * @description: ?OLED12864????????????
 * @param {uint8_t} x??????????
 * @param {uint8_t} y??????????
 * @param {uint8_t} chr??????
 * @param {uint8_t} Char_Size??????????,???? 16/12
 * @param {uint8_t} Color_Turn??????(1???0???)
 * @return {*}
 */
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t Char_Size,uint8_t Color_Turn)
{
	unsigned char c=0,i=0;
		c=chr-' ';//???????
		if(x>128-1){x=0;y=y+2;}
		if(Char_Size ==16)
		{
			OLED_Set_Pos(x,y);
			for(i=0;i<8;i++)
				{
				  if(Color_Turn)
					  OLED_WR_DATA(~F8X16[c*16+i]);
				  else
					  OLED_WR_DATA(F8X16[c*16+i]);
				}
			OLED_Set_Pos(x,y+1);
			for(i=0;i<8;i++)
			    {
				  if(Color_Turn)
					  OLED_WR_DATA(~F8X16[c*16+i+8]);
				  else
					  OLED_WR_DATA(F8X16[c*16+i+8]);
			    }

			}
	     else
	     {
				OLED_Set_Pos(x,y);
				for(i=0;i<6;i++)
			    {
				  if(Color_Turn)
					  OLED_WR_DATA(~F6x8[c][i]);
				  else
					  OLED_WR_DATA(F6x8[c][i]);
			    }
		  }
}

/**
 * @function: void OLED_ShowString(uint8_t x, uint8_t y, uint8_t *chr, uint8_tChar_Size, uint8_t Color_Turn)
 * @description: ?OLED12864???????????
 * @param {uint8_t} x????????????x:0~127
 * @param {uint8_t} y???????????? y:0~7,????????16,???????????2,????????12,??1
 * @param {uint8_t} *chr???????
 * @param {uint8_t} Char_Size???????????,???? 16/12,16?8X16,12?6x8
 * @param {uint8_t} Color_Turn??????(1???0???)
 * @return {*}
 */
void OLED_ShowString(uint8_t x,uint8_t y,char*chr,uint8_t Char_Size, uint8_t Color_Turn)
{
	uint8_t  j=0;
	while (chr[j]!='\0')
	{		OLED_ShowChar(x,y,chr[j],Char_Size, Color_Turn);
			if (Char_Size == 12) //6X8?????6,???????
				x += 6;
			else  
				x += 8;

			if (x > 122 && Char_Size==12) //TextSize6x8?????????,????????
			{
				x = 0;
				y++;
			}
			if (x > 120 && Char_Size== 16) //TextSize8x16?????????,????????
			{
				x = 0;
				y+=2;
			}
			j++;
	}
}

/**
 * @function: void OLED_ShowNum(uint8_t x,uint8_t y,unsigned int num,uint8_t len,uint8_t size2, Color_Turn)
 * @description: ????
 * @param {uint8_t} x???????????,x:0~126
 * @param {uint8_t} y???????????, y:0~7,????????16,???????????2,????????12,??1
 * @param {unsigned int} num:?????
 * @param {uint8_t } len:???????
 * @param {uint8_t} size2:???????,?? 16/12,16?8X16,12?6x8
 * @param {uint8_t} Color_Turn??????(1???0???)
 * @return {*}
 */
void OLED_ShowNum(uint8_t x,uint8_t y,unsigned int num,uint8_t len,uint8_t size2, uint8_t Color_Turn)
{
	uint8_t t,temp;
	uint8_t enshow=0;
	for(t=0;t<len;t++)
	{
		temp=(num/oled_pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				OLED_ShowChar(x+(size2/2)*t,y,' ',size2, Color_Turn);
				continue;
			}else enshow=1;

		}
	 	OLED_ShowChar(x+(size2/2)*t,y,temp+'0',size2, Color_Turn);
	}
}


/**
 * @function: void OLED_Showdecimal(uint8_t x,uint8_t y,float num,uint8_t z_len,uint8_t f_len,uint8_t size2, uint8_t Color_Turn)
 * @description: ???????
 * @param {uint8_t} x???????????,x:0~126
 * @param {uint8_t} y???????????, y:0~7,????????16,???????????2,????????12,??1
 * @param {float} num:????????
 * @param {uint8_t } z_ len:???????
 * @param {uint8_t } f_len: ???????
 * @param {uint8_t} size2:???????,?? 16/12,16?8X16,12?6x8
 * @param {uint8_t} Color_Turn??????(1???0???)
 * @return {*}
 */
void OLED_Showdecimal(uint8_t x,uint8_t y,float num,uint8_t z_len,uint8_t f_len,uint8_t size2, uint8_t Color_Turn)
{
	uint8_t t,temp,i=0;//i??????
	uint8_t enshow;
	int z_temp,f_temp;
	if(num<0)
	{
		z_len+=1;
		i=1;
		num=-num;
	}
	z_temp=(int)num;
	//????
	for(t=0;t<z_len;t++)
	{
		temp=(z_temp/oled_pow(10,z_len-t-1))%10;
		if(enshow==0 && t<(z_len-1))
		{
			if(temp==0)
			{
				OLED_ShowChar(x+(size2/2)*t,y,' ',size2, Color_Turn);
				continue;
			}
			else
			enshow=1;
		}
		OLED_ShowChar(x+(size2/2)*t,y,temp+'0',size2, Color_Turn);
	}
	//???
	OLED_ShowChar(x+(size2/2)*(z_len),y,'.',size2, Color_Turn);

	f_temp=(int)((num-z_temp)*(oled_pow(10,f_len)));
  //????
	for(t=0;t<f_len;t++)
	{
		temp=(f_temp/oled_pow(10,f_len-t-1))%10;
		OLED_ShowChar(x+(size2/2)*(t+z_len)+5,y,temp+'0',size2, Color_Turn);
	}
	if(i==1)//????,???????????
	{
		OLED_ShowChar(x,y,'-',size2, Color_Turn);
		i=0;
	}
}






/**
 * @function: void OLED_HorizontalShift(uint8_t direction)
 * @description: ????????????
 * @param {uint8_t} direction			LEFT	   0x27     	RIGHT  0x26
 * @return {*}
 */
void OLED_HorizontalShift(uint8_t direction)

{
	OLED_WR_CMD(0x2e);//????
	OLED_WR_CMD(direction);//??????
	OLED_WR_CMD(0x00);//??????,???0x00
	OLED_WR_CMD(0x00);//???????
	OLED_WR_CMD(0x07);//??????????????????
    //  0x00-5?, 0x01-64?, 0x02-128?, 0x03-256?, 0x04-3?, 0x05-4?, 0x06-25?, 0x07-2?,
	OLED_WR_CMD(0x07);//???????
	OLED_WR_CMD(0x00);//??????,???0x00
	OLED_WR_CMD(0xff);//??????,???0xff
	OLED_WR_CMD(0x2f);//????-0x2f,????-0x2e,????????
}

/**
 * @function: void OLED_Some_HorizontalShift(uint8_t direction,uint8_t start,uint8_t end)
 * @description: ????????????
 * @param {uint8_t} direction			LEFT	   0x27     	RIGHT  0x26
 * @param {uint8_t} start ?????  0x00~0x07
 * @param {uint8_t} end  ?????  0x01~0x07
 * @return {*}
 */
void OLED_Some_HorizontalShift(uint8_t direction,uint8_t start,uint8_t end)
{
	OLED_WR_CMD(0x2e);//????
	OLED_WR_CMD(direction);//??????
	OLED_WR_CMD(0x00);//??????,???0x00
	OLED_WR_CMD(start);//???????
	OLED_WR_CMD(0x07);//??????????????????,0x07?????2?
	OLED_WR_CMD(end);//???????
	OLED_WR_CMD(0x00);//??????,???0x00
	OLED_WR_CMD(0xff);//??????,???0xff
	OLED_WR_CMD(0x2f);//????-0x2f,????-0x2e,????????

}

/**
 * @function: void OLED_VerticalAndHorizontalShift(uint8_t direction)
 * @description: ??????????????
 * @param {uint8_t} direction				????	 0x29
 *                                                            ????   0x2A
 * @return {*}
 */
void OLED_VerticalAndHorizontalShift(uint8_t direction)
{
	OLED_WR_CMD(0x2e);//????
	OLED_WR_CMD(direction);//??????
	OLED_WR_CMD(0x01);//??????
	OLED_WR_CMD(0x00);//???????
	OLED_WR_CMD(0x07);//??????????????????,?????
	OLED_WR_CMD(0x07);//???????
	OLED_WR_CMD(0x01);//???????
	OLED_WR_CMD(0x00);//??????,???0x00
	OLED_WR_CMD(0xff);//??????,???0xff
	OLED_WR_CMD(0x2f);//????-0x2f,????-0x2e,????????
}

/**
 * @function: void OLED_DisplayMode(uint8_t mode)
 * @description: ????????
 * @param {uint8_t} direction			ON	0xA7  ,
 *                                                          OFF	0xA6	?????,??????
 * @return {*}
 */
void OLED_DisplayMode(uint8_t mode)
{
	OLED_WR_CMD(mode);
}

/**
 * @function: void OLED_IntensityControl(uint8_t intensity)
 * @description: ??????
 * @param  {uint8_t} intensity	0x00~0xFF,RESET=0x7F
 * @return {*}
 */
void OLED_IntensityControl(uint8_t intensity)
{
	OLED_WR_CMD(0x81);
	OLED_WR_CMD(intensity);
}


/**
 * @function: void OLED_ShowChinese(uint8_t x, uint8_t y, uint8_t index, uint8_t Char_Size, uint8_t Color_Turn)
 * @description: 显示汉字（12x12或16x16，直接使用索引）
 * @param {uint8_t} x 起始列（12x12:0~116, 16x16:0~112）
 * @param {uint8_t} y 起始页（12x12:0~6, 16x16:0~6）
 * @param {uint8_t} index 汉字索引（0-7，对应字库数组中的位置）
 * @param {uint8_t} Char_Size 字体大小（12或16）
 * @param {uint8_t} Color_Turn 颜色翻转（1=反色显示，0=正常显示）
 */
void OLED_ShowChinese(uint8_t x, uint8_t y, uint8_t index, uint8_t Char_Size, uint8_t Color_Turn)
{
    uint8_t i;
    uint8_t data;
    
    // 边界检查
    if(index >= 8) return;  // 当前只有8个汉字
    
    if(Char_Size == 12)
    {
        // 12x12字体边界检查
        if(x > 116 || y > 6) return;
        
        OLED_Set_Pos(x, y);
        for(i = 0; i < 16; i++)
        {
            data = Chinese12x12[index*2][i];
            if(Color_Turn) data = ~data;
            OLED_WR_DATA(data);
        }
        
        // 下半部分（8行）
        OLED_Set_Pos(x, y + 1);
        for(i = 0; i < 16; i++)
        {
            data = Chinese12x12[index*2+1][i];
            if(Color_Turn) data = ~data;
            OLED_WR_DATA(data);
        }
    }
    else if(Char_Size == 16)
    {
        // 16x16字体边界检查
//        if(x > 112 || y > 6) return;
        
//        extern const unsigned char Chinese16x16[][32];
//        const unsigned char *pData = Chinese16x16[index];
        
        // 上半部分（8行）
        OLED_Set_Pos(x, y);
        for(i = 0; i < 16; i++)
        {
            data = Chinese16x16[index*2][i];
            if(Color_Turn) data = ~data;
            OLED_WR_DATA(data);
        }
        
        // 下半部分（8行）
        OLED_Set_Pos(x, y + 1);
        for(i = 0; i < 16; i++)
        {
            data = Chinese16x16[index*2+1][i];
            if(Color_Turn) data = ~data;
            OLED_WR_DATA(data);
        }
    }
}




/* USER CODE BEGIN 1 */



/**
 * @brief 画点
 * @param x: 0~127
 * @param y: 0~63
 * @param t: 1 填充, 0 清空
 */
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t)
{
    uint8_t pos, bx, temp;
    if(x > 127 || y > 63) return; // 超出范围
    
    pos = y / 8;      // 计算页地址
    bx = y % 8;       // 计算页内偏移
    temp = 1 << bx;   // 生成掩码
    
    if(t)
        OLED_GRAM[x][pos] |= temp;  // 置位
    else
        OLED_GRAM[x][pos] &= ~temp; // 清零
}

/**
 * @brief 刷新显存到 OLED
 */
void OLED_RefreshGram(void)
{
    uint8_t page, x;

	for(page = 0; page < 8; page++)
    {
        OLED_Set_Pos(0, page);

        for(x = 0; x < 128; x++)
        {
            OLED_WR_DATA(OLED_GRAM[x][page]);
        }
    }
}

void OLED_HalfRefreshGram(void)
{
    uint8_t page, x;

	for(page = 4; page < 8; page++)
    {
        OLED_Set_Pos(0, page);

        for(x = 0; x < 128; x++)
        {
            OLED_WR_DATA(OLED_GRAM[x][page]);
        }
    }
}

// void OLED_DrawPoint(uint8_t x,uint8_t y)
// {
//     uint8_t page, bit;

//     if(x >= 128 || y >= 64) return;

//     page = y / 8;
//     bit  = y % 8;

//     OLED_GRAM[x][page] |= (1 << bit);
// }
// 修改 OLED_Clear 以同时清除显存缓冲区
/**
/**
 * @brief  绘制位图（完全适配你的OLED驱动）
 * @param  bmp: 位图数组指针
 * @param  x,y: 起始坐标
 * @param  w,h: 位图宽高
 * @param  mode: 1=正常显示 0=反显
 */
void OLED_DrawBmp(const uint8_t *bmp, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t mode)
{
    uint8_t i, j, byteWidth = (w + 7) / 8;
    for(j = 0; j < h; j++)
    {
        for(i = 0; i < w; i++)
        {
            uint8_t pixel = (bmp[j * byteWidth + i / 8] & (0x80 >> (i % 8)));
            // 严格匹配你的OLED_DrawPoint三参数
            if((pixel && mode) || (!pixel && !mode))
                OLED_DrawPoint(x + i, y + j, 1);
            else
                OLED_DrawPoint(x + i, y + j, 0);
        }
    }
}


void OLED_ClearGRAM(void)
{
    uint8_t i, n;
    // 清除显存缓冲区
    for(i = 4; i < 8; i++)
        for(n = 0; n < 128; n++)
            OLED_GRAM[n][i] = 0X00;
}


void OLED_ClearALLGRAM(void)
{
    uint8_t i, n;
    // 清除显存缓冲区
    for(i = 0; i < 8; i++)
        for(n = 0; n < 128; n++)
            OLED_GRAM[n][i] = 0X00;
}

/* USER CODE END 1 */
