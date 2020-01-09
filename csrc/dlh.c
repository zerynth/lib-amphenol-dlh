#define ZERYNTH_PRINTF
#include "zerynth.h"
#include "dlh.h"


uint8_t mcmds[5][3] = {
    {0xAA,0x00,0x00},
    {0xAC,0x00,0x00},
    {0xAD,0x00,0x00},
    {0xAE,0x00,0x00},
    {0xAF,0x00,0x00},
};

uint8_t rcmd[] = {0xF0,0x0,0x0,0x0,0x0,0x0,0x0};
uint8_t scmd[] = {0xF0};

#define DIG_OFFS_G ((1<<24)/10)
#define DIG_OFFS_D (1<<23)
#define FSS (0.8)


int _dlh_convert(uint32_t rawpress, uint32_t rawtemp, int unit, int d_or_g, int *press, int *temp) {

    // printf("%i %x\n",rawpress,rawpress);
    int dig =(d_or_g) ? (DIG_OFFS_G):(DIG_OFFS_D);
    int signedpress = ((int)rawpress) - dig;
    double p =  (((double)signedpress)/(1<<24))*1.25*FSS;
    double t = (rawtemp*125.0)/(1<<24) -40;

    switch(unit){
        case 1: //cmH20
            p = p*2.53746;
            break;
        case 2: //pascal
            p = p *248.84;
            break;
        default:
            break;
    }

    *press = (int)(p*1000000);
    *temp = (int)(t*1000);

    return 0;
}

int dlh_acquire(int spidrv, int mode, int unit, int d_or_g, int *press, int *temp){
    uint8_t buf [7];

    // printf("send cmd\n");
    do{
        //device is busy, sleep a bit
        vhalSpiSelect(spidrv);
        vhalSpiExchange(spidrv,&scmd,buf,1);
        vhalSpiUnselect(spidrv);
        // printf("status 1 %x\n",buf[0]);
        if(buf[0]&0x40!=0x40) vosThSleep(TIME_U(1,MILLIS));
    }while(buf[0]&0x40!=0x40);

    if (buf[0]&1 || buf[0]&4) {
        //alu error or eeprom error
        return buf[0];
    }

    //now send cmd
    vhalSpiSelect(spidrv);
    vhalSpiExchange(spidrv,mcmds[mode],buf,3);
    vhalSpiUnselect(spidrv);

    if (buf[0]&1 || buf[0]&4) {
        //alu error or eeprom error
        return buf[0];
    }

    do{
        //device is busy, sleep a bit
        vhalSpiSelect(spidrv);
        vhalSpiExchange(spidrv,scmd,buf,1);
        vhalSpiUnselect(spidrv);
        // printf("status 2 %x\n",buf[0]);
        if(buf[0]&0x40!=0x40) vosThSleep(TIME_U(1,MILLIS));
    }while(buf[0]&0x40!=0x40);

    if (buf[0]&1 || buf[0]&4) {
        //alu error or eeprom error
        return buf[0];
    }

    //now read value
    vhalSpiSelect(spidrv);
    vhalSpiExchange(spidrv,rcmd,buf,7);
    vhalSpiUnselect(spidrv);

    if (buf[0]&1 || buf[0]&4) {
        //alu error or eeprom error
        return buf[0];
    }

    uint32_t rawpress = (buf[1]<<16)|(buf[2]<<8)|buf[3];
    uint32_t rawtemp  = (buf[4]<<16)|(buf[5]<<8)|buf[6];

    _dlh_convert(rawpress,rawtemp,unit,d_or_g,press,temp);

    return 0;
}

C_NATIVE(_dlh_getfast){
    NATIVE_UNWARN();
    uint32_t mode;
    uint32_t spidrv;
    uint32_t unit;
    int d_or_g;
    int err = ERR_OK;
    int press;
    int temp;

    *res =MAKE_NONE();

    if (parse_py_args("iiii", nargs, args, &spidrv,&mode,&unit,&d_or_g) !=4) return ERR_TYPE_EXC;

    RELEASE_GIL();
    if(dlh_acquire(spidrv&0xff, mode, unit, d_or_g, &press, &temp)) err= ERR_PERIPHERAL_INVALID_HARDWARE_STATUS_EXC;
    ACQUIRE_GIL();

    if (err==ERR_OK) {
        PTuple *tpl = ptuple_new(2,NULL);
        PTUPLE_SET_ITEM(tpl,0,pinteger_new(press));
        PTUPLE_SET_ITEM(tpl,1,pinteger_new(temp));
        *res = tpl;
    }
    return err;
}


