/*
脉搏波形显示器

材料：
  Arduino Uno*1
  脉搏传感器*1
  面包板*1
  OLED(SSD1309,4pin)*1
  杜邦线*若干


接线：
  脉搏传感器正极接主板5V,负极接GND,S接A2!
  OLED正负同理，SCL和SDA接主板同名接口！


使用：
  食指（效果好一些）按住脉搏传感器，等OLED的显示结果（建议多测一会，波形更加稳定！）
*/
#include <U8glib.h>  //OLED驱动库

U8GLIB_SSD1309_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);  //初始化OLED



void setup() {
  u8g.setFont(u8g_font_unifont);  //设置OLED字体
  Serial.begin(9600);
 
}
int px=0;    //横轴
float vl[128];   //数据
float xs[128];   //显示数据（和数据一样）
float pv=515.0;  //数据初始平均值
float xl=0;      //BPM

unsigned long lastt=millis();   //初始计时器（方便后面算心率）

void loop() {
  u8g.firstPage();  //图像循环
  
  
  do {

    //显示心率
     u8g.setPrintPos(10, 10);
     if(xl>=200 || xl<=20) u8g.print("--");
     else u8g.print(xl);
     u8g.setPrintPos(60, 10);
     u8g.print("bpm");

    
     float value = analogRead(A2); //获取传感器值
     vl[px%128]=value; //128项为一组数据
     xs[px%128]=vl[px%128];
     float py = xs[px%128];  //纵轴坐标
     px=px+1;//横轴+1
     for(int i=0;i<=px%128;i++){  //绘图
        if(i==0){ //i不能是0减1
          u8g.drawPixel(i,xs[i]/10-8); //-8为了让波显示在屏幕中间，/10让数据映射到（0，64）之间
        }else{
          u8g.drawLine(i-1,xs[i-1]/10-8,i,xs[i]/10-8); //和上一个数据点做线
        }
     }
     
     Serial.println(vl[px%128]);
     

  } while (u8g.nextPage());
  if(px%128==0){ //采集完一组数据，开始计算心率
    unsigned long nowt = millis();  //获取现在时间
    int ptm = nowt-lastt; //获取采集一组数据用的时间（ms)
    lastt=nowt;//更新上一次时间
    
    float sum=0;
    
    for(int i=0;i<=127;i++){
      sum=sum+vl[i];
    }
    pv=sum/128.0-10; //求和计算传感器平均值

    int flag=0;
    int xts=0;//一组数据中心跳次数
    for(int i=0;i<=127;i++){
      if(vl[i]>pv+35 and flag<=0){//找到一次心跳
        xts=xts+1;//心跳+1
        flag=1;//不要重复计算一次心跳
      }
      if(vl[i]<pv) flag=flag-1;
    }
    xl=(xts*1.0)*(60*1.0/(ptm*1.0/1000));//计算bpm(把运行时间毫秒转秒，再用一分钟（60s）除以采集一组数据所花的时间，再乘一组的心跳次数)
  }
}
