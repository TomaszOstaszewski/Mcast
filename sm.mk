SMC_JAR:=./smc_6_0_1/bin/Smc.jar

# Pattern rule to generate SM header and 
%_sm.c %_sm.h : %.sm
	@$(ECHO_JAVA) $@
	$(NOECHO)java -jar $(SMC_JAR) -c -g1  $<

