#for keil5 project

create-stdp-proj:
	if [ ! -d "STM32F4xx_DSP_StdPeriph_Lib_V1.8.0" ]; \
	then \
		echo "$$PWD/STM32F4xx_DSP_StdPeriph_Lib_V1.8.0 SHOULD exist, please download and unzip the library package."; \
	elif [ -d "stdperiph" ]; then \
		echo "$$PWD/stdperiph SHOULD NOT exist."; \
	elif [ -d "project" ]; then \
		echo "$$PWD/project SHOULD NOT exist."; \
	else \
		chmod u+x Lib2Proj.sh \
		./Lib2Proj.sh; \
	fi

zip: clean
	zip -x "*/\.*" -x "\.*" -r Quadcopter.zip firmware project stdperiph ucosii

clean:
	rm -f project/*/Objects/* project/*/Listings/* project/*/DebugConfig/* project/*/*.uvguix.* project/*/*.scvd
	rm -f Quadcopter.zip

clean_root:
	rm -f *.bak *.ddk *.edk *.lst *.lnp *.mpf *.mpj *.obj *.omf *.plg *.rpt *.tmp *.__i *.crf *.o *.d *.axf *.tra *.dep JLinkLog.txt *.iex *.htm *.sct *.map

.PHONY: clean, clean_root, create-stdp-proj
