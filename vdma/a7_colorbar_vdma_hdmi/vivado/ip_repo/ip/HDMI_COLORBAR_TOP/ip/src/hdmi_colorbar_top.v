//****************************************Copyright (c)***********************************//
//ԭ�Ӹ����߽�ѧƽ̨��www.yuanzige.com
//����֧�֣�www.openedv.com
//�Ա����̣�http://openedv.taobao.com
//��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡZYNQ & FPGA & STM32 & LINUX���ϡ�
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2018-2028
//All rights reserved
//----------------------------------------------------------------------------------------
// File name:           hdmi_colorbar_top
// Last modified Date:  2020/05/28 20:28:08
// Last Version:        V1.0
// Descriptions:        HDMI������ʾʵ�鶥��ģ��
//                      
//----------------------------------------------------------------------------------------
// Created by:          ����ԭ��
// Created date:        2020/05/28 20:28:08
// Version:             V1.0
// Descriptions:        The original version
//
//----------------------------------------------------------------------------------------
//****************************************************************************************//

module  hdmi_colorbar_top #(

    //�ֱ���640*480�Ĳ���
    parameter H_SYNC   =  10'd96 ,  //��ͬ��         
    parameter H_BACK   =  10'd48 ,  //����ʾ����            
    parameter H_DISP   =  10'd640,  //����Ч����            
    parameter H_FRONT  =  10'd16 ,  //����ʾǰ��            
    parameter H_TOTAL  =  10'd800,  //��ɨ������
                          
    parameter V_SYNC   =  10'd2  ,  //��ͬ��
    parameter V_BACK   =  10'd33 ,  //����ʾ����
    parameter V_DISP   =  10'd480,  //����Ч����
    parameter V_FRONT  =  10'd10 ,  //����ʾǰ��
    parameter V_TOTAL  =  10'd525   //��ɨ������
)
(
    input         sys_rst_n    , 
    input         ddr_init_done, 
    input         pixel_clk    ,    //����ʱ��
    input         pixel_clk_5x ,    //5������ʱ��
    
    input  [15:0] data_in      ,    //��������
    output        data_req     ,    //�������� 
    
    output        tmds_clk_p   ,    //TMDSʱ��ͨ��
    output        tmds_clk_n   ,
    output [2:0]  tmds_data_p  ,    //TMDS����ͨ��
    output [2:0]  tmds_data_n  ,
    output        tmds_oen     ,    //TMDS���ʹ��
     
    output        tmds_clk_p1  ,    //TMDSʱ��ͨ��
    output        tmds_clk_n1  ,
    output [2:0]  tmds_data_p1 ,    //TMDS����ͨ��
    output [2:0]  tmds_data_n1 ,
    output        tmds_oen1         //TMDS���ʹ��

);

wire  [10:0]  pixel_xpos_w;
wire  [10:0]  pixel_ypos_w;

wire          video_hs;
wire          video_vs;
wire          video_de;
wire  [23:0]  video_rgb;

//*****************************************************
//**                    main code
//*****************************************************

//������Ƶ��ʾ����ģ��
video_driver # ( 
    .H_SYNC (H_SYNC),
    .H_BACK (H_BACK ),
    .H_DISP (H_DISP ),
    .H_FRONT(H_FRONT),
    .H_TOTAL(H_TOTAL),
    
    .V_SYNC (V_SYNC ),
    .V_BACK (V_BACK ),
    .V_DISP (V_DISP ),
    .V_FRONT(V_FRONT),
    .V_TOTAL(V_TOTAL)
    ) u_video_driver(
    .pixel_clk      (pixel_clk),
    .sys_rst_n      (sys_rst_n & ddr_init_done),

    .video_hs       (video_hs),
    .video_vs       (video_vs),
    .video_de       (video_de),
    .video_rgb      (video_rgb),

    .pixel_xpos     (pixel_xpos_w),
    .pixel_ypos     (pixel_ypos_w),
    .data_req       (data_req),
    .pixel_data     (data_in)
    );

//����HDMI����ģ��
dvi_transmitter_top u_rgb2dvi_0(
    .pclk           (pixel_clk),
    .pclk_x5        (pixel_clk_5x),
    .reset_n        (sys_rst_n & ddr_init_done),
                
    .video_din      (video_rgb),
    .video_hsync    (video_hs), 
    .video_vsync    (video_vs),
    .video_de       (video_de),
                
    .tmds_clk_p     (tmds_clk_p),
    .tmds_clk_n     (tmds_clk_n),
    .tmds_data_p    (tmds_data_p),
    .tmds_data_n    (tmds_data_n), 
    .tmds_oen       (tmds_oen),
    
    .tmds_clk_p1    (tmds_clk_p1),
    .tmds_clk_n1    (tmds_clk_n1),
    .tmds_data_p1   (tmds_data_p1),
    .tmds_data_n1   (tmds_data_n1), 
    .tmds_oen1      (tmds_oen1)
    );

endmodule 