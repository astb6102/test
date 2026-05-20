MEMORY
{

    RAMM0S                    : origin = 0x000128, length = 0x0002D8
    RAMM1D                    : origin = 0x000400, length = 0x0003F8
    CLATOCPU_MSGRAM           : origin = 0x001480, length = 0x000080
    CPUTOCLA_MSGRAM           : origin = 0x001500, length = 0x000080
    CLATODMA_MSGRAM           : origin = 0x001680, length = 0x000080
    DMATOCLA_MSGRAM           : origin = 0x001700, length = 0x000080
    RAMLSnP                   : origin = 0x008000, length = 0x004000
    RAMGSnP                   : origin = 0x00C000, length = 0x001000
    RAMGSC                    : origin = 0x00D000, length = 0x001000
    RAMGSF                    : origin = 0x00E000, length = 0x001000
    RAMGSD                    : origin = 0x00F000, length = 0x000FF8
    FLASHBANK0_BOOT           : origin = 0x080000, length = 0x001000
    FLASHBANK0_CODE           : origin = 0x081000, length = 0x00E000
    FLASHBANK0_DATA           : origin = 0x08F000, length = 0x001000
    FLASH_BANK1_SEC0          : origin = 0x090000, length = 0x001000
    FLASH_BANK1_SEC1          : origin = 0x091000, length = 0x001000
    FLASH_BANK1_SEC2          : origin = 0x092000, length = 0x001000
    FLASH_BANK1_SEC3          : origin = 0x093000, length = 0x001000
    FLASH_BANK1_SEC4          : origin = 0x094000, length = 0x001000
    FLASH_BANK1_SEC5          : origin = 0x095000, length = 0x001000
    FLASH_BANK1_SEC6          : origin = 0x096000, length = 0x001000
    FLASH_BANK1_SEC7          : origin = 0x097000, length = 0x001000
    FLASH_BANK1_SEC8          : origin = 0x098000, length = 0x001000
    FLASH_BANK1_SEC9          : origin = 0x099000, length = 0x001000
    FLASH_BANK1_SEC10         : origin = 0x09A000, length = 0x001000
    FLASH_BANK1_SEC11         : origin = 0x09B000, length = 0x001000
    FLASH_BANK1_SEC12         : origin = 0x09C000, length = 0x001000
    FLASH_BANK1_SEC13         : origin = 0x09D000, length = 0x001000
    FLASH_BANK1_SEC14         : origin = 0x09E000, length = 0x001000
    FLASH_BANK1_SEC15         : origin = 0x09F000, length = 0x001000
    FLASH_BANK2_SEC0          : origin = 0x0A0000, length = 0x001000
    FLASH_BANK2_SEC1          : origin = 0x0A1000, length = 0x001000
    FLASH_BANK2_SEC2          : origin = 0x0A2000, length = 0x001000
    FLASH_BANK2_SEC3          : origin = 0x0A3000, length = 0x001000
    FLASH_BANK2_SEC4          : origin = 0x0A4000, length = 0x001000
    FLASH_BANK2_SEC5          : origin = 0x0A5000, length = 0x001000
    FLASH_BANK2_SEC6          : origin = 0x0A6000, length = 0x001000
    FLASH_BANK2_SEC7          : origin = 0x0A7000, length = 0x001000
    FLASH_BANK2_SEC8          : origin = 0x0A8000, length = 0x001000
    FLASH_BANK2_SEC9          : origin = 0x0A9000, length = 0x001000
    FLASH_BANK2_SEC10         : origin = 0x0AA000, length = 0x001000
    FLASH_BANK2_SEC11         : origin = 0x0AB000, length = 0x001000
    FLASH_BANK2_SEC12         : origin = 0x0AC000, length = 0x001000
    FLASH_BANK2_SEC13         : origin = 0x0AD000, length = 0x001000
    FLASH_BANK2_SEC14         : origin = 0x0AE000, length = 0x001000
    FLASH_BANK2_SEC15         : origin = 0x0AF000, length = 0x000FF0
    RESET                     : origin = 0x3FFFC0, length = 0x000002
}


SECTIONS
{

    .reset               : >  RESET, TYPE = DSECT /* not used, */
    codestart            : >  0x080000
    .text                : >  FLASHBANK0_CODE,
                              ALIGN(8)

    GROUP
    {
       .TI.ramfunc
       ramfuncs
       dclfuncs
       dcl32funcs
    }                         LOAD = FLASHBANK0_CODE,
                              RUN = RAMLSnP,
				              LOAD_START(RamfuncsLoadStart),
				              LOAD_SIZE(RamfuncsLoadSize),
				              LOAD_END(RamfuncsLoadEnd),
				              RUN_START(RamfuncsRunStart),
				              RUN_SIZE(RamfuncsRunSize),
				              RUN_END(RamfuncsRunEnd),
                              ALIGN(8)

    ctrlfuncs            :    LOAD >  FLASHBANK0_CODE,
                              RUN  >  RAMLSnP,
                              LOAD_START(loadStart_ctrlfuncs),
                              LOAD_END(loadEnd_ctrlfuncs),
                              LOAD_SIZE(loadSize_ctrlfuncs),
                              RUN_START(runStart_ctrlfuncs),
                              RUN_END(runEnd_ctrlfuncs),
                              RUN_SIZE(runSize_ctrlfuncs),
                              ALIGN(8)

    .binit               : >  FLASHBANK0_DATA,
                              ALIGN(8)
    .ovly                : >  FLASHBANK0_DATA,
                              ALIGN(8)
    .cinit               : >  FLASHBANK0_CODE,
                              ALIGN(8)
    .stack               : >  RAMM0S
    .init_array          : >  FLASHBANK0_CODE,
                              ALIGN(8)
    .bss                 : >  RAMM1D
    .const               : >  FLASHBANK0_CODE,
                              ALIGN(8)
    .data                : >  RAMM1D
    .switch              : >  FLASHBANK0_CODE,
                              ALIGN(8)
    .sysmem              : >  RAMM1D

    est_data             : >  RAMGSF,
                              LOAD_START(loadStart_est_data),
                              LOAD_END(loadEnd_est_data),
                              LOAD_SIZE(loadSize_est_data)
    hal_data		     :>   RAMM1D
                        	  LOAD_START(loadStart_hal_data),
                        	  LOAD_END(loadEnd_hal_data),
                        	  LOAD_SIZE(loadSize_hal_data)
    user_data            : >  RAMGSC,
                              LOAD_START(loadStart_user_data),
                              LOAD_END(loadEnd_user_data),
                              LOAD_SIZE(loadSize_user_data)
    foc_data             : >  RAMGSC,
                              LOAD_START(loadStart_foc_data),
                              LOAD_END(loadEnd_foc_data),
                              LOAD_SIZE(loadSize_foc_data)
    motor_data           : >  RAMGSC,
                              LOAD_START(loadStart_motor_data),
                              LOAD_END(loadEnd_motor_data),
                              LOAD_SIZE(loadSize_motor_data)
    sys_data             : >  RAMGSC,
                              LOAD_START(loadStart_sys_data),
                              LOAD_END(loadEnd_sys_data),
                              LOAD_SIZE(loadSize_sys_data)
    vibc_data            : >  RAMGSC,
                              LOAD_START(loadStart_vibc_data),
                              LOAD_END(loadEnd_vibc_data),
                              LOAD_SIZE(loadSize_vibc_data)
    dmaBuf_data          : >  RAMGSD,
                              LOAD_START(loadStart_dmaBuf_data),
                              LOAD_END(loadEnd_dmaBuf_data),
                              LOAD_SIZE(loadSize_dmaBuf_data)
    datalog_data         : >  RAMGSD,
                              LOAD_START(loadStart_datalog_data),
                              LOAD_END(loadEnd_datalog_data),
                              LOAD_SIZE(loadSize_datalog_data)
    SFRA_F32_Data   	 :>   RAMGSD,
                        	  LOAD_START(loadStart_SFRA_F32_Data),
                        	  LOAD_END(loadEnd_SFRA_F32_Data),
                        	  LOAD_SIZE(loadSize_SFRA_F32_Data)

}
