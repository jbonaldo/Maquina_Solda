################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
DSP2833x_headers/cmd/DSP2833x_Headers_nonBIOS.out: ../DSP2833x_headers/cmd/DSP2833x_Headers_nonBIOS.cmd $(GEN_CMDS)
	@echo 'Building file: $<'
	@echo 'Invoking: Linker'
	@echo 'Flags: -v28 -mt -ml -g --define="_DEBUG" --define="LARGE_MODEL" --quiet --diag_warning=225 --issue_remarks --float_support=fpu32 --output_all_syms --asm_directory="C:/Documents and Settings/San_Jose/Desktop/TI/Maquina_Solda/DSP_Maquina_Solda_Completa/Debug" --obj_directory="C:/Documents and Settings/San_Jose/Desktop/TI/Maquina_Solda/DSP_Maquina_Solda_Completa/Debug" -z -m"../DSP2833x_headers/cmd/Debug/Example_2833xSpi_FFDLB_int.map" --stack_size=0x300 --heap_size=0x1000 --warn_sections -i"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/c2000/lib" -i"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/c2000/include" -i"E:/TexasInstruments/C2000/workspace/DSP_Maquina_Solda" -i"C:/Documents and Settings/San_Jose/Desktop/TI/Maquina_Solda/DSP_Maquina_Solda_Completa/DSP2833x_headers/include" -i"C:/Documents and Settings/San_Jose/Desktop/TI/Maquina_Solda/DSP_Maquina_Solda_Completa/DSP2833x_common/include" --reread_libs --entry_point=code_start --rom_model'
	$(shell echo -v28 -mt -ml -g --define="_DEBUG" --define="LARGE_MODEL" --quiet --diag_warning=225 --issue_remarks --float_support=fpu32 --output_all_syms --asm_directory="C:/Documents and Settings/San_Jose/Desktop/TI/Maquina_Solda/DSP_Maquina_Solda_Completa/Debug" --obj_directory="C:/Documents and Settings/San_Jose/Desktop/TI/Maquina_Solda/DSP_Maquina_Solda_Completa/Debug" -z -m"../DSP2833x_headers/cmd/Debug/Example_2833xSpi_FFDLB_int.map" --stack_size=0x300 --heap_size=0x1000 --warn_sections -i"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/c2000/lib" -i"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/c2000/include" -i"E:/TexasInstruments/C2000/workspace/DSP_Maquina_Solda" -i"C:/Documents and Settings/San_Jose/Desktop/TI/Maquina_Solda/DSP_Maquina_Solda_Completa/DSP2833x_headers/include" -i"C:/Documents and Settings/San_Jose/Desktop/TI/Maquina_Solda/DSP_Maquina_Solda_Completa/DSP2833x_common/include" --reread_libs --entry_point=code_start --rom_model > "ccsLinker.opt")
	$(shell echo $< >> "ccsLinker.opt")
	"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/c2000/bin/cl2000" -@"ccsLinker.opt" -o "$@"
	@echo 'Finished building: $<'
	@echo ' '


