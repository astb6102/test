################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
src_board/%.obj: ../src_board/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'C2000 Compiler - building file: "$<"'
	"C:/ti/ccs2040/ccs/tools/compiler/ti-cgt-c2000_22.6.3.LTS/bin/cl2000" --cmd_file="ccsIncludes.opt"  -v28 -ml -mt --cla_support=cla2 --float_support=fpu32 --idiv_support=idiv0 --tmu_support=tmu1 --vcu_support=vcrc -O3 --opt_for_speed=3 --fp_mode=relaxed --define=_INLINE --define=_FLASH --define=_F28003x --define=_FULL_FAST_LIB --define=BSXL8316RT_REVA --define=MOTOR1_ESMO_N --define=MOTOR1_ENC_N --define=MOTOR1_OVM_N --define=MOTOR1_SSIPD_N --define=MOTOR1_VOLRECT_N --define=DATALOGF2_EN_N --define=TEST_ENABLE_N --define=STEP_RP_EN_N --define=CMD_POT_EN_N --define=CMD_CAP_EN_N --define=CMD_SWITCH_EN_N --define=CMD_CAN_EN --define=CPUTIME_ENABLE --define=syh_N --define=DRV_CS_GPIO --define=HALL_CAL_N --define=MOTOR1_HALL_N --define=MOTOR1_FAST --define=evm_N --define=MOTOR1_FWC_N --define=MOTOR1_LS_CAL_N --define=DAC128S_ENABLE --define=DAC128S_SPIB --define=SFRA_ENABLE_n --define=MOTOR1_MTPA --define=MOTOR1_FILTERVS_n --define=define=MOTOR1_FILTERIS_n --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="src_board/$(basename $(<F)).d_raw" --include_path="C:/Users/thekim/workspace_ccstheia/KIM_TEST/Flash_lib_DRV8316RT_3SC/syscfg" --obj_directory="src_board" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


