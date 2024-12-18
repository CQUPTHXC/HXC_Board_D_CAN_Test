#include <Arduino.h>
#include "ESP_CAN.hpp"
#include "HXCthread.hpp"
#include "tuple"

int led_pin = 4;

void LED_state(int CAN_Flag=0)      //启动后为默认状态
{
    pinMode(led_pin,OUTPUT);

    switch (CAN_Flag)
    {
    case 0:                 //Flag=0为默认状态，蓝色LED以1s为周期闪烁
        while(1)
        {
            digitalWrite(led_pin,HIGH);
            delay(1000);
            digitalWrite(led_pin,LOW);
            delay(1000);        
        }
        break;
    case 1:                //接收成功，Flag=1，LED常亮5s，后转为默认状态
        digitalWrite(led_pin,HIGH);
        delay(10000);
        CAN_Flag=0;
        break;
    case -1:                //发送或接收失败，LED熄灭
        digitalWrite(led_pin,LOW);
        break;
    case 2:                 //发送成功，Flag=2，LED快速闪两下，后转为默认状态
        for(int i=2;i--;i>0)
        {
            digitalWrite(led_pin,HIGH);
            delay(200);
            digitalWrite(led_pin,LOW);
            delay(200);
        }
        CAN_Flag=0;
        break;
    
    default:
        break;
    }
}

HXC::thread<int> LED(LED_state);

void CAN_Send(twai_message_t *data_send)
{
    constexpr const int send_address=0x123;     //要发送的硬件的16进制地址，参照参考文档
    int flag;
    while(true)
    {
        twai_message_t msg;
        data_send->identifier = send_address;
        data_send->data_length_code = 4;
        for(int i=0;i<4;i++)
        {
            msg.data[i]=0x111;          //自定义data数据内容
        }

        if(twai_transmit(&msg,pdMS_TO_TICKS(1000)==ESP_OK))     //发送成功
        {
            flag = 2;
            LED_state(flag);
        }
        else if(twai_transmit(&msg,pdMS_TO_TICKS(1000)==ESP_FAIL))      //发送失败
        {
            flag = -1;
            LED_state(flag);
        }
    }
}

void CAN_Receive(twai_message_t *message)
{
    while(true){
        /*接收数据*/
        if(twai_receive(message,pdMS_TO_TICKS(10000))==ESP_OK)
        {
            LED_state(1);
        }
        else
        {
            LED_state(-1);
        }
        /*接收数据*/
        
    }
}
/*HXC::thread<void> send([](){            //发送数据线程
});


HXC::thread<void> receive([](){             //接收数据线程  
});

*/
void setup() {
    can_setup(6,7); 
    twai_message_t send_msg;        //发送消息
    LED.start(0);
    add_user_can_func(0x123,CAN_Receive);           //接收数据          
    CAN_Send(&send_msg);
}

void loop() {}

