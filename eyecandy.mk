ifeq ($(V),1)
NOECHO=
else
NOECHO=@
endif
ECHO_AR:= echo 			"AR        " 
ECHO_DEP:= echo 		"DEP       " 
ECHO_CC := echo 		"CC        "  
ECHO_JAVA:= echo 		"JAVA      " 
ECHO_CXX:= echo 		"CXX       " 
ECHO_LD := echo 		"LD        "  
ECHO_MD := echo 		"MD        "  
ECHO_RM := echo 		"RM        "  
ECHO_GENERATE := echo 	"GEN       "
ECHO_MAKE     := echo 	"MAKE      "	
ECHO_CONFIGURE:= echo 	"CONFIGURE "
ECHO_UNPACK:= echo 		"UNPACK    "
ECHO_UT:= echo          "UT        "
