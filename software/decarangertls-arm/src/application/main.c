/*! ----------------------------------------------------------------------------
 *  @file    main.c
 *  @brief   main loop for the DecaRanging application
 *
 * @attention
 *
 * Copyright 2013 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */
/* Includes */
#include "compiler.h"
#include "port.h"

#include "instance.h"

#include "deca_types.h"

#include "deca_spi.h"

extern void usb_run(void);
extern int usb_init(void);
extern void usb_printconfig(int, uint8*, int);
extern void send_usbmessage(uint8*, int);

#define SWS1_SHF_MODE 0x02	//short frame mode (6.81M)
#define SWS1_CH5_MODE 0x04	//channel 5 mode
#define SWS1_ANC_MODE 0x08  //anchor mode
#define SWS1_A1A_MODE 0x10  //anchor/tag address A1
#define SWS1_A2A_MODE 0x20  //anchor/tag address A2
#define SWS1_A3A_MODE 0x40  //anchor/tag address A3
#define SWS1_USB2SPI_MODE 0x78  //USB to SPI mode
#define SWS1_TXSPECT_MODE 0x38  //Continuous TX spectrum mode
                             //"1234567812345678"
#define SOFTWARE_VER_STRING    "Ver.  1.05  TREK" //16 bytes!

uint8 s1switch = 0;
int instance_anchaddr = 0;
int dr_mode = 0;
int chan, tagaddr, ancaddr;
int instance_mode = ANCHOR;
//int instance_mode = TAG;
//int instance_mode = LISTENER;

#define LCD_BUFF_LEN (80)
uint8 dataseq[LCD_BUFF_LEN];
uint8 dataseq1[LCD_BUFF_LEN];

typedef struct
{
    uint8 channel ;
    uint8 prf ;
    uint8 datarate ;
    uint8 preambleCode ;
    uint8 preambleLength ;
    uint8 pacSize ;
    uint8 nsSFD ;
    uint16 sfdTO ;
} chConfig_t ;


//Configuration for DecaRangeRTLS TREK Modes (4 default use cases selected by the switch S1 [2,3] on EVB1000, indexed 0 to 3 )
chConfig_t chConfig[4] ={
                    //mode 1 - S1: 2 off, 3 off
                    {
                        2,              // channel
                        DWT_PRF_16M,    // prf
                        DWT_BR_110K,    // datarate
                        4,              // preambleCode
                        DWT_PLEN_1024,  // preambleLength
                        DWT_PAC32,      // pacSize
                        1,       // non-standard SFD
                        (1025 + 64 - 32) //SFD timeout
                    },
                    //mode 2 - S1: 2 on, 3 off
                    {
                        2,              // channel
                        DWT_PRF_16M,    // prf
                        DWT_BR_6M8,    // datarate
                        4,             // preambleCode
                        DWT_PLEN_128,   // preambleLength
                        DWT_PAC8,       // pacSize
                        0,       // non-standard SFD
                        (129 + 8 - 8) //SFD timeout
                    },
                    //mode 3 - S1: 2 off, 3 on
                    {
                        5,              // channel
                        DWT_PRF_16M,    // prf
                        DWT_BR_110K,    // datarate
                        3,              // preambleCode
                        DWT_PLEN_1024,  // preambleLength
                        DWT_PAC32,      // pacSize
                        1,       // non-standard SFD
                        (1025 + 64 - 32) //SFD timeout
                    },
                    //mode 4 - S1: 2 on, 3 on
                    {
                        5,              // channel
                        DWT_PRF_16M,    // prf
                        DWT_BR_6M8,    // datarate
                        3,             // preambleCode
                        DWT_PLEN_128,   // preambleLength
                        DWT_PAC8,       // pacSize
                        0,       // non-standard SFD
                        (129 + 8 - 8) //SFD timeout
                    }
};


// ======================================================
//
//  Configure instance tag/anchor/etc... addresses
//
void addressconfigure(uint8 s1switch)
{
    instanceAddressConfig_t ipc ;

    instance_anchaddr = (((s1switch & SWS1_A1A_MODE) << 2) + (s1switch & SWS1_A2A_MODE) + ((s1switch & SWS1_A3A_MODE) >> 2)) >> 4;
    if(instance_anchaddr > 3)
    {
    	ipc.anchorAddress = GATEWAY_ANCHOR_ADDR | 0x4 ; //listener
    }
    else
    {
    	ipc.anchorAddress = GATEWAY_ANCHOR_ADDR | instance_anchaddr;
    }
    ipc.tagAddress = instance_anchaddr;

    instancesetaddresses(&ipc);
}

uint32 inittestapplication(uint8 s1switch);


//returns the use case / operational mode
int decarangingmode(uint8 s1switch)
{
    int mode = 0;

    if(s1switch & SWS1_SHF_MODE)
    {
        mode = 1;
    }

    if(s1switch & SWS1_CH5_MODE)
    {
        mode = mode + 2;
    }

    return mode;
}

uint32 inittestapplication(uint8 s1switch)
{
    uint32 devID ;
    instanceConfig_t instConfig;
    int result;

    SPI_ConfigFastRate(SPI_BaudRatePrescaler_32);  //max SPI before PLLs configured is ~4M

    //this is called here to wake up the device (i.e. if it was in sleep mode before the restart)
    devID = instancereaddeviceid() ;
    if(DWT_DEVICE_ID != devID) //if the read of device ID fails, the DW1000 could be asleep
    {
        port_SPIx_clear_chip_select();  //CS low
        Sleep(1);   //200 us to wake up then waits 5ms for DW1000 XTAL to stabilise
        port_SPIx_set_chip_select();  //CS high
        Sleep(7);
        devID = instancereaddeviceid() ;
        // SPI not working or Unsupported Device ID
        if(DWT_DEVICE_ID != devID)
            return(-1) ;
        //clear the sleep bit - so that after the hard reset below the DW does not go into sleep
        dwt_softreset();
    }

    //reset the DW1000 by driving the RSTn line low
    reset_DW1000();

    result = instance_init() ;
    if (0 > result) return(-1) ; // Some failure has occurred

    SPI_ConfigFastRate(SPI_BaudRatePrescaler_4); //increase SPI to max
    devID = instancereaddeviceid() ;

    if (DWT_DEVICE_ID != devID)   // Means it is NOT DW1000 device
    {
        // SPI not working or Unsupported Device ID
        return(-1) ;
    }

    addressconfigure(s1switch) ;                            // set up initial payload configuration

    if((s1switch & SWS1_ANC_MODE) == 0)
    {
        instance_mode = TAG;
    }
    else
    {
        instance_mode = ANCHOR;

        if(instance_anchaddr == 0x4)
        {
        	instance_mode = LISTENER;
        }
    }

    instancesetrole(instance_mode) ;     // Set this instance role

    // get mode selection (index) this has 4 values see chConfig struct initialiser for details.
    dr_mode = decarangingmode(s1switch);

    chan = instConfig.channelNumber = chConfig[dr_mode].channel ;
    instConfig.preambleCode = chConfig[dr_mode].preambleCode ;
    instConfig.pulseRepFreq = chConfig[dr_mode].prf ;
    instConfig.pacSize = chConfig[dr_mode].pacSize ;
    instConfig.nsSFD = chConfig[dr_mode].nsSFD ;
    instConfig.sfdTO = chConfig[dr_mode].sfdTO ;
    instConfig.dataRate = chConfig[dr_mode].datarate ;
    instConfig.preambleLen = chConfig[dr_mode].preambleLength ;

    instance_config(&instConfig) ;                  // Set operating channel etc

    instancesettagsleepdelay(POLL_SLEEP_DELAY); //set the Tag sleep time

    //set the default response delays
    instancesetreplydelay(FIXED_REPLY_DELAY_MULTI);

    return devID;
}
/**
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/
void process_dwRSTn_irq(void)
{
    instance_notify_DW1000_inIDLE(1);
}

void process_deca_irq(void)
{
    do{

        instance_process_irq(0);

    }while(port_CheckEXT_IRQ() == 1); //while IRS line active (ARM can only do edge sensitive interrupts)

}

void initLCD(void)
{
    uint8 initseq[9] = { 0x39, 0x14, 0x55, 0x6D, 0x78, 0x38 /*0x3C*/, 0x0C, 0x01, 0x06 };
    uint8 command = 0x0;
    int j = 100000;

    writetoLCD( 9, 0,  initseq); //init seq
    while(j--);

    command = 0x2 ;  //return cursor home
    writetoLCD( 1, 0,  &command);
    command = 0x1 ;  //clear screen
    writetoLCD( 1, 0,  &command);
}

void setLCDline1(uint8 s1switch)
{
	int role = instancegetrole();

	sprintf((char*)&dataseq[0], "DecaRangeRTLS %s%d", (s1switch & SWS1_SHF_MODE) ? "S" : "L", chan);
	writetoLCD( 40, 1, dataseq); //send some data

	tagaddr = instance_anchaddr;
	ancaddr = instance_anchaddr;

	if(role == TAG)
	{
		sprintf((char*)&dataseq1[0], "Tag:%d    ", tagaddr);
		writetoLCD( 16, 1, dataseq1); //send some data

	}
	else if(role == ANCHOR)
	{
		sprintf((char*)&dataseq1[0], "Anchor:%d ", ancaddr);
		writetoLCD( 16, 1, dataseq1); //send some data
	}
	else
	{
		ancaddr = 4;
		sprintf((char*)&dataseq1[0], "Listener ");
		writetoLCD( 16, 1, dataseq1); //send some data
	}
}

void configureContinuousTxSpectrumMode(uint8 s1switch)
{
    uint8 command = 0x2 ;  //return cursor home
    writetoLCD( 1, 0,  &command);
	sprintf((char*)&dataseq[0], "Continuous TX %s%d", (s1switch & SWS1_SHF_MODE) ? "S" : "L", chan);
	writetoLCD( 40, 1, dataseq); //send some data
	memcpy(dataseq, (const uint8 *) "Spectrum Test   ", 16);
	writetoLCD( 16, 1, dataseq); //send some data

	//configure DW1000 into Continuous TX mode
	instance_starttxtest(0x1000);
	//measure the power
	//Spectrum Analyser set:
	//FREQ to be channel default e.g. 3.9936 GHz for channel 2
	//SPAN to 1GHz
	//SWEEP TIME 1s
	//RBW and VBW 1MHz
	//measure channel power

	//user has to reset the board to exit mode
	while(1)
	{
		Sleep(2);
	}

}


/*
 * @fn      main()
 * @brief   main entry point
**/
int main(void)
{
    int i = 0;
    //int toggle = 1;
    uint8 command = 0x0;

    uint8 usbVCOMout[LCD_BUFF_LEN];
    double range_result = 0, range_raw = 0;

    led_off(LED_ALL); //turn off all the LEDs

    peripherals_init();

    spi_peripheral_init();

    Sleep(1000); //wait for LCD to power on

    initLCD();

    memset(dataseq, 0, LCD_BUFF_LEN);
    memcpy(dataseq, (const uint8 *) "DECAWAVE        ", 16);
    writetoLCD( 40, 1, dataseq); //send some data
    memcpy(dataseq, (const uint8 *) SOFTWARE_VER_STRING, 16); // Also set at line #26 (Should make this from single value !!!)
    writetoLCD( 16, 1, dataseq); //send some data

    Sleep(1000);
#ifdef USB_SUPPORT
    // enable the USB functionality
    usb_init();
    Sleep(1000);
#endif

    s1switch = is_button_low(0) << 1 // is_switch_on(TA_SW1_2) << 2
    		| is_switch_on(TA_SW1_3) << 2
    		| is_switch_on(TA_SW1_4) << 3
    		| is_switch_on(TA_SW1_5) << 4
		    | is_switch_on(TA_SW1_6) << 5
    		| is_switch_on(TA_SW1_7) << 6
    		| is_switch_on(TA_SW1_8) << 7;

    port_DisableEXT_IRQ(); //disable ScenSor IRQ until we configure the device

    if((s1switch & SWS1_USB2SPI_MODE) == SWS1_USB2SPI_MODE)
    {
        int j = 1000000;

        memset(dataseq, 0, LCD_BUFF_LEN);

        while(j--);
        command = 0x2 ;  //return cursor home
        writetoLCD( 1, 0,  &command);

        memcpy(dataseq, (const uint8 *) "DECAWAVE   ", 12);
        writetoLCD( 40, 1, dataseq); //send some data
#ifdef USB_SUPPORT //this is set in the port.h file
        memcpy(dataseq, (const uint8 *) "USB-to-SPI ", 12);
#else
#endif
        writetoLCD( 16, 1, dataseq); //send some data

        j = 1000000;

        while(j--);

        command = 0x2 ;  //return cursor home
        writetoLCD( 1, 0,  &command);
#ifdef USB_SUPPORT //this is set in the port.h file
        // enable the USB functionality
        //usb_init();

        NVIC_DisableDECAIRQ();

        // Do nothing in foreground -- allow USB application to run, I guess on the basis of USB interrupts?
        while (1)       // loop forever
        {
            usb_run();
        }
#endif
        return 1;
    }
    else //run DecaRangeRTLS application for TREK
    {

        command = 0x2 ;  //return cursor home
        writetoLCD( 1, 0,  &command);
        memset(dataseq, ' ', LCD_BUFF_LEN);
        memcpy(dataseq, (const uint8 *) "DECAWAVE   TREK ", 16);
        writetoLCD( 16, 1, dataseq); //send some data

        led_off(LED_ALL);

        if(inittestapplication(s1switch) == (uint32)-1)
        {
            led_on(LED_ALL); //to display error....
            dataseq[0] = 0x2 ;  //return cursor home
            writetoLCD( 1, 0,  &dataseq[0]);
            memset(dataseq, ' ', LCD_BUFF_LEN);
            memcpy(dataseq, (const uint8 *) "ERROR   ", 12);
            writetoLCD( 40, 1, dataseq); //send some data
            memcpy(dataseq, (const uint8 *) "  INIT FAIL ", 12);
            writetoLCD( 40, 1, dataseq); //send some data
            return 0; //error
        }

#ifdef USB_SUPPORT //this is defined in the port.h file
        // Configure USB for output, (i.e. not USB to SPI)
        usb_printconfig(16, (uint8 *)SOFTWARE_VER_STRING, s1switch);
#endif
        // Is continuous spectrum test mode selected?
        if((s1switch & SWS1_TXSPECT_MODE) == SWS1_TXSPECT_MODE)
    	{
        	//this function does not return!
    		configureContinuousTxSpectrumMode(s1switch);
    	}

        //sleep for 5 seconds displaying last LCD message and flashing LEDs
        i=30;
        while(i--)
        {
            if (i & 1) led_off(LED_ALL);
            else    led_on(LED_ALL);

            Sleep(200);
        }
        i = 0;
        led_off(LED_ALL);
        command = 0x2 ;  //return cursor home
        writetoLCD( 1, 0,  &command);

        memset(dataseq, ' ', LCD_BUFF_LEN);
        memset(dataseq1, ' ', LCD_BUFF_LEN);

        setLCDline1(s1switch);

        command = 0x2 ;  //return cursor home
        writetoLCD( 1, 0,  &command);
    }

    port_EnableEXT_IRQ(); //enable ScenSor IRQ before starting

    //memset(dataseq, ' ', LCD_BUFF_LEN);
    memset(dataseq1, ' ', LCD_BUFF_LEN);

    // main loop
    while(1)
    {
        instance_run();

        if(instancenewrange())
        {
        	int n = 0, l = 0, r= 0, aaddr, taddr;
        	int rangeTime;
        	int rres, rres_raw;
            uint16 txa, rxa;

            //send the new range information to LCD and/or USB
            range_result = instancegetidist();
            range_raw = instancegetidistraw();
            aaddr = instancenewrangeancadd() & 0xf;
            taddr = instancenewrangetagadd() & 0xf;
            rangeTime = instancenewrangetim() & 0xffffffff;
#if 1 //LCD update?
            command = 0x2 ;  //return cursor home
            writetoLCD( 1, 0,  &command);

            memset(dataseq1, ' ', LCD_BUFF_LEN);
            writetoLCD( 40, 1, dataseq); //send some data

            if(instance_mode == ANCHOR)
            {
            	sprintf((char*)&dataseq1[0], "A%d  A%dT%d: %3.2f m", ancaddr, aaddr, taddr, range_result);
            }
            else if(instance_mode == TAG)
            {
                sprintf((char*)&dataseq1[0], "T%d  A%dT%d: %3.2f m", tagaddr, aaddr, taddr, range_result);
            }
            else
            {
            	sprintf((char*)&dataseq1[0], "LS  A%dT%d: %3.2f m", aaddr, taddr, range_result);
            }
            writetoLCD( 16, 1, dataseq1); //send some data
#endif
#ifdef USB_SUPPORT //this is set in the port.h file
            l = instancegetlcount() & 0xFFFF;
            r = instancegetrnum();
            txa =  instancetxantdly();
            rxa =  instancerxantdly();
            rres = ((int)(range_result*1000));
            rres_raw = ((int)(range_raw*1000));

            //				anchorID tagID range countofranges rangenum
            if(instance_mode == TAG)
            {
            	n = sprintf((char*)&usbVCOMout[0], "ma%02x t%02x %08x %08x %04x %02x %08x %04x %04x t%d", aaddr, taddr, rres, rres_raw, l, r, rangeTime, txa, rxa, tagaddr);
            }
            else if (instance_mode == ANCHOR)
            {
            	n = sprintf((char*)&usbVCOMout[0], "ma%02x t%02x %08x %08x %04x %02x %08x %04x %04x a%d", aaddr, taddr, rres, rres_raw, l, r, rangeTime, txa, rxa, ancaddr);
            }
            else
            {
            	n = sprintf((char*)&usbVCOMout[0], "ma%02x t%02x %08x %08x %04x %02x %08x %04x %04x l%d", aaddr, taddr, rres, rres_raw, l, r, rangeTime, txa, rxa, ancaddr);
            }
            send_usbmessage(&usbVCOMout[0], n);
#endif
        }



#ifdef USB_SUPPORT //this is set in the port.h file
        usb_run();
#endif
    }


    return 0;
}



